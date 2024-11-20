#include "perspective_utils.hpp"
#include "maths.hpp"
#include "globals.hpp"

void birds_eye(
    const cv::Mat &src, 
    cv::Mat &dst,
    cv::Mat &m,
    cv::Mat &minv,
    const std::vector<cv::Point2f> &src_pts,
    const bool verbose
) {
    int width = src.size().width;
    int height = src.size().height;
    std::vector<cv::Point2f> dst_pts = {
        cv::Point2f(0, 0), 
        cv::Point2f(width, 0), 
        cv::Point2f(0, height), 
        cv::Point2f(width, height)
    };
    m = cv::getPerspectiveTransform(src_pts, dst_pts);
    minv = cv::getPerspectiveTransform(dst_pts, src_pts);
    cv::warpPerspective(src, dst, m, cv::Size(width, height));

    if (verbose) {
        cv::imshow("birds-eye view", dst);
    }
}

double bird_eye_x_coeff(
    const int y,
    const std::vector<cv::Point2f>& warping_points
) {
    int width_base = warping_points[3].x - warping_points[2].x;
    int width_horizon = warping_points[1].x - warping_points[0].x;
    int height = warping_points[2].y - warping_points[0].y;
    cv::Point p(width_base, 0);
    cv::Point q(width_horizon, height);
    double a, b, c;
    line_from_points(p, q, a, b, c);
    std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
    double x = solve_line(a, b, c, NULL, y);
    std::cout << "x: " << x << std::endl;
    std::cout << "width_base: " << height << std::endl;
    return x / width_base;
}