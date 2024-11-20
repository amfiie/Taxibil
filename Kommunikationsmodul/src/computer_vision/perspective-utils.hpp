#pragma once

#include <opencv2/opencv.hpp>

void birdeye(
    const cv::Mat &src, 
    cv::Mat &dst,
    cv::Mat &m,
    cv::Mat &minv,
    const std::vector<cv::Point2f> &src_pts,
    const bool verbose=false
);

void undistort(
    const cv::Mat& src,
    cv::Mat& dst
);