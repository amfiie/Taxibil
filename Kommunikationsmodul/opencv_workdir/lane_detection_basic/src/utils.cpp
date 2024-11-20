#include "utils.hpp"

void canny(cv::Mat &src, cv::Mat &dst) {
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_RGB2GRAY);

    int kernel = 5;
    cv::Mat blur;
    cv::GaussianBlur(gray, blur, cv::Size(kernel, kernel), 0);

    Canny(blur, dst, 50, 150);
}

void region_of_interest(cv::Mat &src, cv::Mat &dst) {
    int width = src.size().width;
    int height = src.size().height;
    cv::Mat mask = cv::Mat::zeros(cv::Size(width,height), CV_8U);
    std::vector<cv::Point> triangle = {
        cv::Point(200, height),
        cv::Point(800, 350),
        cv::Point(1200, height)
    };
    cv::fillPoly(mask, triangle, cv::Scalar(255, 255, 255));
    bitwise_and(src, src, dst, mask); 
}

void hough_lines(cv::Mat &src, std::vector<cv::Vec4d> &lines) {
    cv::HoughLinesP(src, lines, 2, CV_PI/180, 300, 40, 5); 
}

void display_lines(cv::Mat &src, cv::Mat &dst, std::vector<cv::Vec4d> &lines, const cv::Scalar &color) {
    dst = src.clone();
    for (size_t i=0; i<lines.size(); i++) {
        cv::Vec4d l = lines[i];
        cv::line(dst, cv::Point2d(l[0], l[1]), cv::Point2d(l[2], l[3]), color, 10, cv::LINE_AA);
    }
}

void average_slope_intercept(cv::Mat &src, std::vector<cv::Vec4d> &dst, std::vector<cv::Vec4d> &lines) {
    std::vector<cv::Vec2d> left_fit;
    std::vector<cv::Vec2d> right_fit;
    for (size_t i=0; i<lines.size(); i++) {
        cv::Vec4i l = lines[i];
        std::vector<cv::Point2d> points = {cv::Point2d(l[0], l[1]), cv::Point2d(l[2], l[3])};
        cv::Vec2d fit;
        polyfit(points, fit);
        double slope = fit[0];
        double intercept = fit[1];
        if (slope < 0) {
            left_fit.push_back(cv::Vec2d(slope, intercept));
        } else {
            right_fit.push_back(cv::Vec2d(slope, intercept));
        }
    } 
    cv::Scalar left_fit_avarage = cv::mean(left_fit);
    cv::Scalar right_fit_avarage = cv::mean(right_fit);
    cv::Vec4d left_line, right_line;
    make_points(src, left_fit_avarage, left_line);
    make_points(src, right_fit_avarage, right_line);
    dst = {left_line, right_line};
}

void make_points(cv::Mat &src, cv::Scalar &line_si, cv::Vec4d &dst) {
    double slope = line_si[0];
    double intercept = line_si[1];
    double y1 = src.size().height;
    double y2 = y1*3.0/5;
    double x1 = (y1-intercept)/slope;
    double x2 = (y2-intercept)/slope;
    dst = cv::Vec4d(x1, y1, x2, y2);
}

void polyfit(std::vector<cv::Point2d> &pts, cv::Vec2d &dst) {
    double n = pts.size();
    double sum_x = 0, sum_y = 0;
    for (int i = 0; i < n; i++) {
        sum_x += pts[i].x;
        sum_y += pts[i].y;
    }
    double mean_x = sum_x / n;
    double mean_y = sum_y / n;
    double a = 0, b = 0;
    for (int i = 0; i < n; i++) {
        a += (pts[i].x - mean_x) * (pts[i].y - mean_y);
        b += pow((pts[i].x - mean_x), 2);
    }
    double slope = a / b;
    double intercept = mean_y - slope * mean_x;
    dst = cv::Vec2d(slope, intercept);
}

void fill_lane(
    cv::Mat &src, cv::Mat &dst, std::vector<cv::Vec4d> &lines, 
    const cv::Scalar &color_lines, const cv::Scalar &color_lane
){
    int width = src.size().width;
    int height = src.size().height;
    cv::Mat image_lines = cv::Mat::zeros(cv::Size(width,height), CV_8UC3);
    for (size_t i=0; i<lines.size(); i++) {
        cv::Vec4d l = lines[i];
        cv::line(image_lines, cv::Point2d(l[0], l[1]), cv::Point2d(l[2], l[3]), color_lines, 10, cv::LINE_AA);
    }
    dst = src.clone();
    for(int i=0; i<height; i++){
        bool started = 0;
        int startX = 0, stopX = 0;
        for(int j=0; j<width; j++) {
            cv::Vec3b colour = image_lines.at<cv::Vec3b>(cv::Point(j, i));
            if(colour.val[0]==color_lines[0] && colour.val[1]==color_lines[1] && colour.val[2]==color_lines[2]) {
                started = 1;
                if(startX && !stopX)
                    stopX = j;
            } else if(started) {
                if(!startX)
                    startX = j;
                started = 0;
            }
        }
        if (startX && stopX) {
            for(int k=startX; k<stopX; k++) {
                dst.at<cv::Vec3b>(cv::Point(k,i))[0] = color_lane[0];
                dst.at<cv::Vec3b>(cv::Point(k,i))[1] = color_lane[1];
                dst.at<cv::Vec3b>(cv::Point(k,i))[2] = color_lane[2];
            }
        }
    }
}