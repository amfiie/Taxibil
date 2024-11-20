#pragma once

#include <opencv2/opencv.hpp>

const cv::Scalar YELLOW_HSV_TH_MIN(0, 70, 70);
const cv::Scalar YELLOW_HSV_TH_MAX(50, 255, 255);

void binarize(const cv::Mat& src, cv::Mat& dst, const bool verbose=false);

void thresh_frame_in_HSV(
    const cv::Mat& src, 
    cv::Mat& dst, 
    const cv::Scalar& hsv_th_min, 
    const cv::Scalar& hsv_th_max, 
    const bool verbose=false
);

void get_binary_from_equalized_grayscale(
    const cv::Mat& src, 
    cv::Mat& dst, 
    const bool verbose=false
);

// not working, but canny makes almost the same
void thresh_frame_sobel(
    const cv::Mat& src, 
    cv::Mat& dst, 
    const int kernel_size, 
    const bool verbose=false
);

void thresh_frame_canny(
    const cv::Mat &src, 
    cv::Mat &dst, 
    const int kernel_size, 
    const bool verbose=false
);