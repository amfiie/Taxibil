#include "trackbars.hpp"

WarpingTrackbars::WarpingTrackbars(const std::string window_name, const cv::Size &src_size) {
    this->src_size = src_size;
    this->window_name = window_name;
    p1x = 697, p1y = 450; 
    p2x = 850, p2y = 448;
    p3x = 390, p3y = 720;
    p4x = 1280, p4y = 720;
}

void WarpingTrackbars::update() {
    cv::createTrackbar("p1x", window_name, &p1x, src_size.width);
    cv::createTrackbar("p1y", window_name, &p1y, src_size.height);
    cv::createTrackbar("p2x", window_name, &p2x, src_size.width);
    cv::createTrackbar("p2y", window_name, &p2y, src_size.height);
    cv::createTrackbar("p3x", window_name, &p3x, src_size.width);
    cv::createTrackbar("p3y", window_name, &p3y, src_size.height);
    cv::createTrackbar("p4x", window_name, &p4x, src_size.width);
    cv::createTrackbar("p4y", window_name, &p4y, src_size.height);
}

void WarpingTrackbars::get_points(std::vector<cv::Point2f> &dst) {
    dst = {
        cv::Point2f(p1x, p1y),
        cv::Point2f(p2x, p2y),
        cv::Point2f(p3x, p3y),
        cv::Point2f(p4x, p4y)
    };
}

void WarpingTrackbars::draw_points(cv::Mat &src, cv::Mat &dst) {
    dst = src.clone();
    cv::circle(dst, cv::Point2f(p1x, p1y), 10, cv::Scalar(0,0,255), cv::FILLED);
    cv::circle(dst, cv::Point2f(p2x, p2y), 10, cv::Scalar(0,0,255), cv::FILLED);
    cv::circle(dst, cv::Point2f(p3x, p3y), 10, cv::Scalar(0,0,255), cv::FILLED);
    cv::circle(dst, cv::Point2f(p4x, p4y), 10, cv::Scalar(0,0,255), cv::FILLED);
}
