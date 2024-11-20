#include <opencv2/opencv.hpp>
#include "trackbars.hpp"
#include "binarization-utils.hpp"
#include "perspective_utils.hpp"
#include "line-utils.hpp"
#include <chrono>
#include "globals.hpp"
#include <cmath>
#include <fmt/core.h>
#include "maths.hpp"

const std::string VIDEO_PATH = "res/camera-test3.mp4";

void prepare_out_blend_frame(
	const cv::Mat& blend_on_road,
	const cv::Mat& img_binary,
	const cv::Mat& img_birdeye,
	const cv::Mat& img_fit,
	const double offset_pixel,
	const CameraData& camera_data,
	const double duration,
	cv::Mat& dst
) {
	const int width = blend_on_road.size().width;
	const int height = blend_on_road.size().height;
	
	const double thumb_ratio = 0.2;
	const int thumb_width = int(thumb_ratio * width);
	const int thumb_height = int(thumb_ratio * height);

	int padding_x = 20, padding_y = 15;

	// add a gray rectangle to highlight the upper area
	cv::Mat mask;
	blend_on_road.copyTo(mask);
	cv::rectangle(
		mask, 
		cv::Point(0, 0), 
		cv::Point(width, thumb_height + 2 * padding_y),
		cv::Scalar(0, 0, 0),
		cv::FILLED
	);
	cv::addWeighted(mask, 0.2, blend_on_road, 0.8, 0, dst);

	// add thumbnail of binary image
	cv::Mat thumb_binary;
	cv::resize(img_binary, thumb_binary, cv::Size(thumb_width, thumb_height));
	cv::cvtColor(thumb_binary, thumb_binary, cv::COLOR_GRAY2BGR);
	thumb_binary.copyTo(dst(cv::Rect(padding_x, padding_y, thumb_width, thumb_height)));

	// add thumbnail of bird's eye view
	cv::Mat thumb_birdeye;
	cv::resize(img_birdeye, thumb_birdeye, cv::Size(thumb_width, thumb_height));
	cv::cvtColor(thumb_birdeye, thumb_birdeye, cv::COLOR_GRAY2BGR);
	thumb_birdeye.copyTo(dst(cv::Rect(2 * padding_x + thumb_width, padding_y, thumb_width, thumb_height)));

	// add thumbnail of bird's eye view (lane-line highlighted)
	cv::Mat thumb_img_fit;
	cv::resize(img_fit, thumb_img_fit, cv::Size(thumb_width, thumb_height));
	thumb_img_fit.copyTo(dst(cv::Rect(3 * padding_x + 2 * thumb_width, padding_y, thumb_width, thumb_height)));

	// add text (curvature and offset info) on the upper right of the blend
	cv::putText(
		dst, 
		fmt::format("Offset from center: {:.{}f}m", offset_pixel * XM_PER_PIX, 2), 
		cv::Point(450, 30),
		cv::FONT_HERSHEY_SIMPLEX,
		0.4,
		cv::Scalar(255, 255, 255),
		1,
		cv::LINE_AA
	);

	cv::putText(
		dst,
		fmt::format("Distance to vertical line: {:.{}f}px", camera_data.distance_to_line, 4),
		cv::Point(450, 60),
		cv::FONT_HERSHEY_SIMPLEX,
		0.3,
		cv::Scalar(255, 255, 255),
		1,
		cv::LINE_AA
	);

	cv::putText(
		dst,
		fmt::format("Pipeline Duration: {}ms", duration, 4),
		cv::Point(450, 90),
		cv::FONT_HERSHEY_SIMPLEX,
		0.4,
		cv::Scalar(255, 255, 255),
		1,
		cv::LINE_AA
	);
}

void compute_distance_to_stop_line(
    cv::Mat& stop_lines,
	const Line& line_lt,
	const Line& line_rt,
	CameraData& camera_data
) {
    std::vector<cv::Point> nonzero;
    cv::findNonZero(stop_lines, nonzero);

    if (nonzero.empty()) {
		camera_data.distance_to_line = -1.0;
		camera_data.line_type = LineType::NONE;
        return;
    }

    cv::Scalar midpoint = cv::mean(nonzero);
    float x = abs(midpoint[0] - WIDTH / 2);
    float y = abs(midpoint[1] - HEIGHT);

	int line_mid_x = 
		(line_rt.lane_x.at<int>(midpoint[1]) - line_lt.lane_x.at<int>(midpoint[1])) / 2
		+ line_lt.lane_x.at<int>(midpoint[1]);

	int offset = midpoint[0] - line_mid_x;

	if (offset < -25) {
		camera_data.line_type = LineType::LEFT;
	} else if (offset > 25) {
		camera_data.line_type = LineType::RIGHT;
	} else {
		camera_data.line_type = LineType::STOP;
	}

    camera_data.distance_to_line = sqrt(pow(x, 2) + pow(y, 2));
}

CameraData process_pipeline(
	const cv::Mat& src, 
	cv::Mat& dst,
	cv::Mat& road_warped,
	int& processed_frames,
	Line& line_lt,
	Line& line_rt,
	const Direction& direction,
	TargetLine& target_line,
	const bool& keep_state=true
) {
	CameraData camera_data;

	auto time = std::chrono::system_clock::now();

	// TEMPORARY CROP IMAGE
	// cv::Mat img_cropped = src(cv::Rect(CROP_X_MARGIN, CROP_Y_MARGIN, CROP_WIDTH, CROP_HEIGHT));

	// binarize the frame s.t. lane lines are highlighted as much as possible
	cv::Mat img_binary;
	binarize(src, img_binary);

	// compute perspective transform to obtain bird's eye view
	cv::Mat img_birds_eye, m, minv;
	std::vector<cv::Point2f> warping_pts = {
		cv::Point2f(WARP_X_LOW, WARP_Y_LOW),
		cv::Point2f(WARP_X_HIGH, WARP_Y_LOW),
		cv::Point2f(0, WARP_Y_HIGH),
		cv::Point2f(WIDTH, WARP_Y_HIGH)
	};
	birds_eye(img_binary, img_birds_eye, m, minv, warping_pts);

	// Find horizontal lines and subtract them from img_birds_eye
    cv::Mat lanes;
	cv::Mat stop_lines;
	if (processed_frames > 0) {
		cv::Mat road_mask = find_road_mask(img_birds_eye, line_lt, line_rt, cv::Scalar(255, 255, 255), CV_8U, STOP_LINE_MARGIN);
		cv::subtract(img_birds_eye, road_mask, lanes);
		cv::bitwise_and(img_birds_eye, img_birds_eye, stop_lines, road_mask); 
	} else {
    	stop_lines = find_horizontal_lines(img_birds_eye);
		img_birds_eye.copyTo(lanes);
	}

	// fit 2-degree polynomial curve onto lane lines found
	cv::Mat img_fit;
	if (processed_frames > 0 && keep_state && (line_rt.detected || line_lt.detected)) {
		switch (direction)
		{
		case Direction::STRAIGHT:
			if (line_lt.detected) {
				get_fits_by_previous_fits(lanes, img_fit, line_lt, true);
			} else {
				if (line_lt.radius_of_curvature > 0.5) {
					line_lt = find_line_from_another_line(line_rt, true);
				} else {
					get_fits_by_sliding_window(lanes, img_fit, line_lt, true, 9);
				}
			}
			
			if (line_rt.detected) {
				get_fits_by_previous_fits(lanes, img_fit, line_rt, false);
			} else {
				if (line_rt.radius_of_curvature > 0.5) {
					line_rt = find_line_from_another_line(line_lt, false);
				} else {
					get_fits_by_sliding_window(lanes, img_fit, line_rt, false, 9);
				}
			}
			
			break;
		
		case Direction::RIGHT:
			if (line_rt.detected) {
				get_fits_by_previous_fits(lanes, img_fit, line_rt, false, true);
			} else {
				get_fits_by_sliding_window(lanes, img_fit, line_rt, false, 9, true);
			}
			line_lt = find_line_from_another_line(line_rt, true);
			break;

		case Direction::LEFT:
			if (line_lt.detected) {
				get_fits_by_previous_fits(lanes, img_fit, line_lt, true, true);
			} else {
				get_fits_by_sliding_window(lanes, img_fit, line_lt, true, 9, true);
			}
			line_rt = find_line_from_another_line(line_lt, false);
			break;

		case Direction::DEFAULT:
			if (line_lt.detected) {
				get_fits_by_previous_fits(lanes, img_fit, line_lt, true, true);
			} else {
				get_fits_by_sliding_window(lanes, img_fit, line_lt, true, 9, true);
			}
			
			if (line_rt.detected) {
				get_fits_by_previous_fits(lanes, img_fit, line_rt, false, true);
			} else {
				get_fits_by_sliding_window(lanes, img_fit, line_rt, false, 9, true);
			}
			break;

		default:
			break;
		}
	} else {
		get_fits_by_sliding_window(lanes, img_fit, line_lt, true, 9, true);
	 	get_fits_by_sliding_window(lanes, img_fit, line_rt, false, 9, true);
	}

	compute_distance_to_stop_line(stop_lines, line_lt, line_rt, camera_data);

	// draw the surface enclosed by lane lines back onto the original frame
	int offset_pixel;
	cv::Mat blend_on_road;
	draw_back_onto_the_road(src, blend_on_road, road_warped, minv, line_lt, line_rt, offset_pixel);

	camera_data.center_offset_meter = offset_pixel * XM_PER_PIX;

	// process pipeline duration
	auto time2 = std::chrono::system_clock::now();
	double duration = std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time).count();

	// stitch on the top of final output images from different steps of the pipeline
	prepare_out_blend_frame(blend_on_road, img_binary, stop_lines, img_fit, offset_pixel, camera_data, duration, dst);

	processed_frames++;

	return camera_data;
}

void update_camera_data(
	cv::VideoCapture& capture,
	CameraData& camera_data,
	const Direction& direction
) {
	static Line line_lt(10), line_rt(10);
	static cv::Mat frame, output, road_warped;
	static int processed_frames = 0;
	static TargetLine target_line;

	// Read a new frame from capture
	if (!capture.read(frame))
	{
		std::cout << "ERROR: can't read frame from capture!\n";
		exit(2);
	}

	camera_data = process_pipeline(frame, output, road_warped, processed_frames, line_lt, line_rt, direction, target_line);


	// DEBUG
	cv::imshow("output", output);

	// Wait for for 30 ms until any key is pressed.  
	// If the 'Esc' key is pressed, break the while loop.
	// If the any other key is pressed, continue the loop 
	// If any key is not pressed withing 30 ms, continue the loop
	if (cv::waitKey(1) == 27) // if esc key is pressed by user, exit program
		return;
	
	// DEBUG
}

cv::VideoCapture create_camera_reader(
	const std::string& ip_address
) {
	// Open the video file for reading
	cv::VideoCapture capture(VIDEO_PATH);

	// if not success, exit program
	if (!capture.isOpened()) {
		std::cout << "ERROR: can't create capture!\n";
		exit(1);
	}

	return capture;
}



int main(int argc, char *argv[]) {
	std::string ip_address = argv[1];

    cv::VideoCapture capture = create_camera_reader(ip_address);
	CameraData camera_data;
	Direction direction = Direction::DEFAULT;
	
	Direction next_direction = Direction::STRAIGHT;

	bool stop_line_handled = false;

    while(1) {
		update_camera_data(capture, camera_data, direction);

		if (camera_data.line_type == LineType::NONE) {
			stop_line_handled = false;
		} else if (camera_data.line_type == LineType::STOP && camera_data.distance_to_line < 50 && !stop_line_handled) {
			stop_line_handled = true;
			if (direction == Direction::DEFAULT) {
				direction = next_direction;
			} else {
				direction =	Direction::DEFAULT;
			}
		}
	}

	capture.release();
	cv::destroyAllWindows();

	return 0;


}