#include "maths.hpp"

cv::Point arg_max(const cv::Mat& src) {
    double min_val = 0, max_val = 0;
    cv::Point min_loc, max_loc;
    cv::minMaxLoc(src, &min_val, &max_val, &min_loc, &max_loc);
    return max_loc;
}

void linear_space(
    const int start,
    const int stop,
    const size_t num,
    cv::Mat& dst
) {
    float delta = (stop - start) / static_cast<float>(num - 1);
    dst = cv::Mat(num, 1, CV_32S);
    for (size_t i = 0; i < num; i++) {
        dst.at<int>(i) = start + i * delta;
    }
}

cv::Mat merge(const cv::Mat& xs, const cv::Mat& ys) {
    cv::Mat pts;
    std::vector<cv::Mat> merge;
    merge.push_back(xs);
    merge.push_back(ys);
    cv::merge(merge, pts);
    return pts;
}

void solve_polyfit(
    const std::vector<double>& coeffs, 
    const cv::Mat& ys, 
    cv::Mat& xs
) {
    cv::Mat power_2;
    cv::pow(ys, 2.0, power_2);
    xs = power_2.mul(coeffs[2]) + ys.mul(coeffs[1]) + coeffs[0];
}

void set_pixel_colors(
    cv::Mat& src,
    const cv::Mat& points,
    const cv::Scalar color
) {
    for (int i = 0; i < points.rows; i++) {
        cv::Point p = points.at<cv::Point>(i,0);
        cv::Vec3b& clr = src.at<cv::Vec3b>(p.y, p.x);
        clr[0] = color[0];
        clr[1] = color[1];
        clr[2] = color[2];
    }
}

void line_from_points(
    const cv::Point& P, 
    const cv::Point& Q,
    double& a,
    double& b,
    double& c
) {
    a = Q.y - P.y;
    b = P.x - Q.x;
    c = a * (P.x) + b * (P.y);
}

double solve_line(
    double& a,
    double& b,
    double& c,
    const int x, 
    const int y
) {
    assert((x != NULL && y == NULL) || (x == NULL && y != NULL));
    if (x == NULL) {
        // Solve for x
        return (c - b * y) / a;
    } else {
        // Solve for y
        return (c - a * x) / b;
    }
}

cv::Mat vconcat(
    const cv::Mat& xs,
    const cv::Mat& ys
) {
    cv::Mat pts[] = {xs, ys};
    cv::Mat ret;
    cv::vconcat(pts, 2, ret);
    return ret;
}

cv::Mat hconcat(
    const cv::Mat& xs,
    const cv::Mat& ys
) {
    cv::Mat pts[] = {xs, ys};
    cv::Mat ret;
    cv::hconcat(pts, 2, ret);
    return ret;
}

cv::Mat transpose(
    const cv::Mat& matrix
) {
    cv::Mat ret;
    cv::transpose(matrix, ret);
    return ret;
}

cv::Mat flip_rows(
    const cv::Mat& matrix
) {
    cv::Mat ret;
    cv::flip(matrix, ret, 0);
    return ret;
}

std::vector<cv::Point> zip(
    const std::vector<int>& xs,
    const std::vector<int>& ys
) {
    std::vector<cv::Point> pts;
    pts.reserve(xs.size());
    for (int i = 0; i < xs.size(); i++) {
        pts.push_back(cv::Point(xs[i], ys[i]));
    }
    return pts;
}
