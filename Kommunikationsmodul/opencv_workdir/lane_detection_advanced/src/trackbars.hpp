#pragma once

#include <opencv2/opencv.hpp>

class WarpingTrackbars {
public:
    WarpingTrackbars(const std::string window_name, const cv::Size &src_size);

    void update();

    void get_points(std::vector<cv::Point2f> &dst);

    void draw_points(cv::Mat &src, cv::Mat &dst);

private:
    std::string window_name;
    cv::Size src_size;
    int p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y;
};

