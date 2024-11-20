#pragma once

#include <opencv2/opencv.hpp>

void birds_eye(
    const cv::Mat &src, 
    cv::Mat &dst,
    cv::Mat &m,
    cv::Mat &minv,
    const std::vector<cv::Point2f> &src_pts,
    const bool verbose=false
);

double bird_eye_x_coeff(
    const int y,
    const std::vector<cv::Point2f>& warping_points
);