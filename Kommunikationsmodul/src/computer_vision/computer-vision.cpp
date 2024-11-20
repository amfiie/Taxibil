#include "computer-vision.hpp"
#include "line-utils.hpp"
#include "pipeline.hpp"
#include "globals.hpp"
#include "types.hpp"

CameraReader::CameraReader() {}

CameraReader::CameraReader(cv::VideoCapture&& cap) : cap(std::move(cap)) {}

CameraReader::~CameraReader() {
	running.clear();
	thread.join();
	cap.release();
}

void CameraReader::get_last_frame(cv::Mat& frame) {
	mutex.lock();
	cv::swap(frame_a, frame_b);
	mutex.unlock();
	frame = frame_b;
}

void CameraReader::start() {
	running.test_and_set();
	
	cap >> frame_a;
	
	thread = std::thread([this]() { 
		while (running.test_and_set()) {
			mutex.lock();
			// if cap is past last frame, reset it to beginning.
			if (!cap.read(frame_a)) {
				cap.set(cv::CAP_PROP_POS_FRAMES, 0);
				cap >> frame_a;
			}
			mutex.unlock();

			// Sleep the thread so that other thread can aquire mutex.
			// Since this thread still has to wait for another frame from the camera
			//   this is fine.
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	});
}

void update_camera_data(
	CameraReader& camera_reader,
	cv::VideoWriter& stream_writer,
	CameraData& camera_data,
	const PathAction& direction,
	const bool stream,
	const bool verbose
) {
	static Line line_lt(10), line_rt(10);
	static cv::Mat frame, output, road_warped;
	static int processed_frames = 0;

	// Read a new frame from capture
	camera_reader.get_last_frame(frame);

	camera_data = process_pipeline(frame, output, road_warped, processed_frames, line_lt, line_rt, direction);

	if (stream) {
		stream_writer << output;
	}

	if (verbose) {
		cv::imshow("output", output);
		cv::waitKey(1);
	}

}

void create_camera_reader(CameraReader& reader) {
	const std::string CAM_SET = 
		"libcamerasrc ! capsfilter caps=video/x-raw,width=640,height=480,framerate=20/1 ! appsink";

	// Open the video file for reading
	cv::VideoCapture capture = cv::VideoCapture(CAM_SET, cv::CAP_GSTREAMER);
	
	// if not success, exit program
	if (!capture.isOpened()) {
		std::cout << "ERROR: can't create capture!" << std::endl;
		exit(1);
	}

	new (&reader) CameraReader(std::move(capture));
}

void create_camera_reader(CameraReader& reader, const std::string& filename) {
	const std::string VID_PATH = 
		"opencv_workdir/lane_detection_advanced/res/" + filename;

	// Open the video file for reading
	cv::VideoCapture capture = cv::VideoCapture(VID_PATH);
	
	// if not success, exit program
	if (!capture.isOpened()) {
		std::cout << "ERROR: can't create capture!" << std::endl;
		exit(1);
	}

	new (&reader) CameraReader(std::move(capture));
}

cv::VideoWriter create_camera_writer(
	const std::string& ip_adress
) {
	const std::string OUT_SET = "appsrc ! video/x-raw ! videoconvert ! x264enc tune=zerolatency bitrate=2048 speed-preset=superfast"
	" ! rtph264pay ! udpsink host=" + ip_adress;

	cv::VideoWriter writer;
	writer.open(OUT_SET, 0, static_cast<double>(20.0), cv::Size(WIDTH, HEIGHT), true);
	if (!writer.isOpened()){
		std::cout << "ERROR: can't create writer" << std::endl;
	}

	return writer;
}



// int main(int argc, char *argv[]) {
// 	std::string ip_address = argv[1];

//     cv::VideoCapture capture = create_camera_reader(ip_address);
// 	CameraData camera_data;
// 	Direction direction = Direction::DEFAULT;
	
// 	Direction next_direction = Direction::STRAIGHT;

// 	bool stop_line_handled = false;

//     while(1) {
// 		update_camera_data(capture, camera_data, direction);

// 		if (camera_data.line_type == LineType::NONE) {
// 			stop_line_handled = false;
// 		} else if (camera_data.line_type == LineType::STOP && camera_data.distance_to_line < 50 && !stop_line_handled) {
// 			stop_line_handled = true;
// 			if (direction == Direction::DEFAULT) {
// 				direction = next_direction;
// 			} else {
// 				direction =	Direction::DEFAULT;
// 			}
// 		}
// 	}

// 	capture.release();
// 	cv::destroyAllWindows();

// 	return 0;


// }
