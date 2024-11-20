#pragma once

#include <opencv2/opencv.hpp>

template<typename T>
class Queue {
public:

    Queue() = default;

    Queue(const unsigned buffer_len);

    void push(T elem);

    void pop();

    T front() const;

    void clear();

    cv::Scalar mean() const;

private:
    unsigned buffer_len;
    std::vector<T> queue;
};

class Line {
public:

    Line(const int buffer_len=10);

    void update(
        const cv::Mat& new_fit_pixel, 
        const cv::Mat& new_fit_meter,
        const cv::Mat& lane_x,
        const cv::Mat& lane_y, 
        const bool detected, 
        const bool clear_buffer=false
    );

    void draw(
        const cv::Mat& mask, 
        const cv::Scalar color = cv::Scalar(255, 0, 0), 
        const int line_width = 50, 
        const bool average = false
    ) const;

    double radius_of_curvature;
    cv::Mat all_points;
    cv::Mat last_fit_pixel, last_fit_meter;
    cv::Mat lane_x, lane_y;
    bool detected;

private:

    cv::Scalar avarage_fit() const;

    void curvature();

    double curvature_meter();

    int buffer_len;
    Queue<cv::Mat> recent_fits_pixel, recent_fits_meter;
};

cv::Point arg_max(const cv::Mat& src);

void get_fits_by_sliding_window(
    const cv::Mat& src,
    cv::Mat& dst,
    Line& line,
    const bool is_left,
    const int n_windows=9,
    const bool verbose=false
);

void get_fits_by_previous_fits(
    const cv::Mat& src,
    cv::Mat& dst,
    Line& line,
    const bool is_left,
    const bool verbose=false
);

void draw_back_onto_the_road(
    const cv::Mat& img_cropped,
    cv::Mat& dst,
    cv::Mat& road_warped,
    const cv::Mat &minv,
    const Line& line_lt,
    const Line& line_rt,
    int& offset,
    const bool verbose=false
);

/*
 * Borrowed code from URL: 
 * https://answers.opencv.org/question/59139/line-segment-detection-near-horizontal/ 
 */
cv::Mat find_horizontal_lines(
    const cv::Mat& src
);

cv::Mat find_road_mask(
    const cv::Mat& src,
    const Line& line_lt,
    const Line& line_rt,
    const cv::Scalar& color,
    const int type,
    const int margin=0
);

Line find_line_from_another_line(
    const Line& line,
    const bool is_left
);
