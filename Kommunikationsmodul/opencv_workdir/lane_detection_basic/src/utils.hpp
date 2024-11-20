#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>

void canny(cv::Mat &src, cv::Mat &dst);

void region_of_interest(cv::Mat &src, cv::Mat &dst);

void hough_lines(cv::Mat &src, std::vector<cv::Vec4d> &lines);

void display_lines(cv::Mat &src, cv::Mat &dst, std::vector<cv::Vec4d> &lines, const cv::Scalar &color);

void average_slope_intercept(cv::Mat &src, std::vector<cv::Vec4d> &dst, std::vector<cv::Vec4d> &lines);

void make_points(cv::Mat &src, cv::Scalar &line_si, cv::Vec4d &dst);

// Mathematical algorithm from: https://www.varsitytutors.com/hotmath/hotmath_help/topics/line-of-best-fit
void polyfit(std::vector<cv::Point2d> &pts, cv::Vec2d &dst);

void fill_lane(
    cv::Mat &src, cv::Mat &dst, std::vector<cv::Vec4d> &lines, 
    const cv::Scalar &color_lines, const cv::Scalar &color_lane
);

# endif // UTILS_H