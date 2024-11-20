#include <opencv2/opencv.hpp>
#include "utils.hpp"

const std::string VIDEO_PATH = "res/test1.mp4";

int main() {
    // Open the video file for reading
	cv::VideoCapture capture(VIDEO_PATH);

	// if not success, exit program
	if (!capture.isOpened()) {
		std::cin.get(); // Wait for any key press
		return -1;
	}

    while(1) {
        cv::Mat frame;

        // Read a new frame from video
		if (!capture.read(frame)) // Video replayes for internety
		{
			capture = cv::VideoCapture(VIDEO_PATH);
			continue;
		}

        cv::Mat canny_output;
        canny(frame, canny_output);

        cv::Mat masked_output;
        region_of_interest(canny_output, masked_output);

        std::vector<cv::Vec4d> lines;
        hough_lines(masked_output, lines);

        std::vector<cv::Vec4d> average_lines;
        average_slope_intercept(frame, average_lines, lines);

        cv::Mat line_image;
        fill_lane(frame, line_image, average_lines, cv::Scalar(0,0,255), cv::Scalar(0,255,255));
        // display_lines(line_image, line_image, lines, cv::Scalar(255,0,0));
        display_lines(line_image, line_image, average_lines, cv::Scalar(0,0,255));

        cv::imshow("Original", frame); 
        cv::imshow("Lines", line_image);
        // cv::imshow("Masked", masked_output);

        // Wait for for 30 ms until any key is pressed.  
        // If the 'Esc' key is pressed, break the while loop.
        // If the any other key is pressed, continue the loop 
        // If any key is not pressed withing 30 ms, continue the loop
		if (cv::waitKey(30) == 27) // if esc key is pressed by user, exit program
			break;
	}

	capture.release();
	cv::destroyAllWindows();

	return 0;


}
