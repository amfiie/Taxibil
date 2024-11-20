#include "binarization-utils.hpp"
#include "globals.hpp"

void binarize(const cv::Mat& src, cv::Mat& dst, const bool verbose) {
    dst = cv::Mat::zeros(src.size(), CV_8U);

	// cv::Mat hsv_yellow_mask;
	// thresh_frame_in_HSV(src, hsv_yellow_mask, YELLOW_HSV_TH_MIN, YELLOW_HSV_TH_MAX);
	// cv::bitwise_or(dst, hsv_yellow_mask, dst);

	cv::Mat eq_black_mask;
	get_binary_from_equalized_grayscale(src, eq_black_mask);
	cv::bitwise_or(dst, eq_black_mask, dst);

	cv::Mat canny_mask;
	thresh_frame_canny(src, canny_mask, 9);
	cv::bitwise_or(dst, canny_mask, dst);

	cv::Mat kernel = cv::Mat::ones(cv::Size(5, 5), CV_8U);
	cv::morphologyEx(dst, dst, cv::MORPH_CLOSE, kernel);

	if (verbose) {
        // cv::imshow("src", src);
		// cv::imshow("after closure", kernel);
		cv::imshow("dst", dst);
	}
}

void thresh_frame_in_HSV(
    const cv::Mat& src, 
    cv::Mat& dst, 
    const cv::Scalar& hsv_th_min, 
    const cv::Scalar& hsv_th_max, 
    const bool verbose
) {
    cv::cvtColor(src, dst, cv::COLOR_BGR2HSV);
    cv::inRange(dst, hsv_th_min, hsv_th_max, dst);

    if (verbose) {
        cv::imshow("yellow mask", dst);
    }
}

void get_binary_from_equalized_grayscale(
    const cv::Mat& src, 
    cv::Mat& dst, 
    const bool verbose
) {
    cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(dst, dst);
    cv::threshold(dst, dst, 30, 255, cv::THRESH_BINARY_INV);

    if (verbose) {
        cv::imshow("black mask", dst);
    }
}

void thresh_frame_sobel(
    const cv::Mat& src, 
    cv::Mat& dst, 
    const int kernel_size, 
    const bool verbose
) {
    cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
    cv::Mat sobel_x, sobel_y;
    cv::Sobel(dst, sobel_x, CV_64F, 1, 0, kernel_size);
    cv::Sobel(dst, sobel_y, CV_64F, 0, 1, kernel_size);

    cv::Mat sobel_mag;
    cv::pow(sobel_x, 2, sobel_x);
    cv::pow(sobel_y, 2, sobel_y);
    cv::sqrt(sobel_x + sobel_y, sobel_mag);

    double max;
    cv::minMaxLoc(sobel_mag, NULL, &max);
    sobel_mag = sobel_mag / max * 255;

    cv::threshold(sobel_mag, dst, 50, 1, cv::THRESH_BINARY);

    if (verbose) {
        cv::imshow("sobel mask", dst);
    }
}

void thresh_frame_canny(
    const cv::Mat &src, 
    cv::Mat &dst, 
    const int kernel_size, 
    const bool verbose
) {
    cv::cvtColor(src, dst, cv::COLOR_RGB2GRAY);
    cv::GaussianBlur(dst, dst, cv::Size(kernel_size, kernel_size), 0);
    Canny(dst, dst, 50, 150);
    
    if (verbose) {
        cv::imshow("canny mask", dst);
    }
}