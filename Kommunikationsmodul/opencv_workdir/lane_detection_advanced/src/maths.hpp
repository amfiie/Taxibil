#pragma once

#include <opencv2/opencv.hpp>

cv::Point arg_max(const cv::Mat& src);

void linear_space(
    const int start,
    const int stop,
    const size_t num,
    cv::Mat& dst
);

cv::Mat merge(const cv::Mat& xs, const cv::Mat& ys);

void solve_polyfit(
    const std::vector<double>& coeffs, 
    const cv::Mat& ys, 
    cv::Mat& xs
);

void set_pixel_colors(
    cv::Mat& src,
    const cv::Mat& pts,
    const cv::Scalar color
);

void line_from_points(
    const cv::Point& P, 
    const cv::Point& Q,
    double& a,
    double& b,
    double& c
);

double solve_line(
    double& a,
    double& b,
    double& c,
    const int x=NULL, 
    const int y=NULL
);

cv::Mat vconcat(
    const cv::Mat& xs,
    const cv::Mat& ys
);

cv::Mat hconcat(
    const cv::Mat& xs,
    const cv::Mat& ys
);

cv::Mat transpose(
    const cv::Mat& matrix
);

cv::Mat flip_rows(
    const cv::Mat& matrix
);

std::vector<cv::Point> zip(
    const std::vector<int>& xs,
    const std::vector<int>& ys
);