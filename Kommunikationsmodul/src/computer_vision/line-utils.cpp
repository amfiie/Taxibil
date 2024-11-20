#include "line-utils.hpp"
#include "maths.hpp"
#include "polynomial-regression.hpp"
#include "globals.hpp"
#include <algorithm>

template<typename T>
Queue<T>::Queue(const unsigned buffer_len) {
    this->buffer_len = buffer_len;
    this->queue.reserve(buffer_len);
}

template<typename T>
void Queue<T>::push(T elem) {
    if (this->queue.size() >= static_cast<size_t>(this->buffer_len)) {
        this->queue.erase(this->queue.begin());
    }
    this->queue.insert(this->queue.end(), elem);
}

template<typename T>
void Queue<T>::pop() {
    this->queue.erase(this->queue.begin());
}

template<typename T>
T Queue<T>::front() const {
    return *this->queue.begin();
}

template<typename T>
void Queue<T>::clear() {
    std::vector<T> empty;
    std::swap(this->queue, empty);
}

template<typename T>
cv::Scalar Queue<T>::mean() const {
    return cv::mean(this->queue);
}

inline bool validate_fit_pixel(const std::vector<double>& fit_pixel) {
    if (fit_pixel.size() != 3) {
        std::cerr << "Warning: fit_pixel.size() was not 3!\n";
        return true;
    }

    return false;
}

Line::Line(const int buffer_len) {
    this->buffer_len = buffer_len;
    this->recent_fits_pixel = Queue<cv::Mat>(buffer_len);
    this->recent_fits_meter = Queue<cv::Mat>(buffer_len * 2);
}

void Line::update(
    const cv::Mat& new_fit_pixel, 
    const cv::Mat& new_fit_meter,
    const cv::Mat& lane_x,
    const cv::Mat& lane_y, 
    const bool detected, 
    const bool clear_buffer
) {
    this->detected = detected;
    if (clear_buffer) {
        this->recent_fits_pixel.clear();
        this->recent_fits_meter.clear();
    }

    this->last_fit_pixel = new_fit_pixel;
    this->last_fit_meter = new_fit_meter;

    this->radius_of_curvature = curvature_meter();

    this->lane_x = lane_x;
    this->lane_y = lane_y;

    this->recent_fits_pixel.push(this->last_fit_pixel);
    this->recent_fits_meter.push(this->last_fit_meter);
}

void Line::draw (
    const cv::Mat& mask, 
    const cv::Scalar color, 
    const int line_width, 
    const bool average
) const {
    cv::Mat line_left_side = lane_x - line_width / 2;
    cv::Mat line_right_side = lane_x + line_width / 2;
    std::vector<cv::Point> pts = zip(line_left_side, lane_y);
    std::vector<cv::Point> pts_right = zip(line_right_side, lane_y);
    std::reverse(pts_right.begin(), pts_right.end());
    pts.insert(pts.end(), pts_right.begin(), pts_right.end());
    cv::fillPoly(mask, pts, color);
}

cv::Scalar Line::avarage_fit() const {
    return this->recent_fits_pixel.mean();
}

double Line::curvature_meter() {
    // return pow(pow(1 + (2 * coeffs[0] * y_eval + coeffs[1]), 2), 1.5) / abs(2 * coeffs[0]);
    return pow(1 + last_fit_meter.at<double>(1), 3) / abs(2 * last_fit_meter.at<double>(0)); 
}


void get_fits_by_sliding_window(
    const cv::Mat& src,
    cv::Mat& dst,
    Line& line,
    const bool is_left,
    const int n_windows,
    const bool verbose
) {
    const int width = src.size().width;
    const int height = src.size().height;

    // Assuming you have created a warped binary image called "binary_warped"
    // Take a histogram of the bottom half of the image
    cv::Mat histogram;
    int offset = 0;
    if (is_left) {
        histogram = src(cv::Rect(0, height / 2, width / 2, height / 2));
        // cv::imshow("left hist", histogram);
    } else {
        histogram = src(cv::Rect(width / 2, height / 2, width / 2, height / 2));
        offset = width / 2;
        // cv::imshow("right hist", histogram);
    }

    // Identify the x and y positions of all nonzero pixels in the image
    std::vector<cv::Point> nonzero;
    cv::findNonZero(src, nonzero);

    // Find the peak of the left and right halves of the histogram
    // These will be the starting point for the left and right lines
    std::vector<cv::Point> hist_nonzero;
    cv::findNonZero(histogram, hist_nonzero);

    // Create an output image to draw on and visualize the result
    if (dst.empty()) {
        cv::cvtColor(src, dst, cv::COLOR_GRAY2BGR);
    }

    int base_x;
    if (!hist_nonzero.empty()) {
        base_x = cv::mean(hist_nonzero)[0] + offset;
    } else {
        line.detected = false;
        return;
    }

    // Set height of windows
    int window_height = (height / n_windows);

    // Current positions to be updated for each window
    int current_x = base_x;

    int margin = 100;  // width of the windows +/- margin
    int min_pix = 50;  // minimum number of pixels found to recenter window

    // Create empty lists to receive left and right lane pixel indices
    cv::Mat lane_inds;
    for (int window = 0; window < n_windows; window++) {
         // Identify window boundaries in x and y (and right and left)
        int win_y_low = height - (window + 1) * window_height;
        int win_y_high = height - window * window_height;
        
        int win_x_low = current_x - margin;
        int win_x_high = current_x + margin;

        win_x_low = std::clamp(win_x_low, 0, width);
        win_x_high = std::clamp(win_x_high, 0, width);

        // Draw the windows on the visualization image
        cv::rectangle(
            dst, 
            cv::Point2i(win_x_low, win_y_low), 
            cv::Point2i(win_x_high, win_y_high), 
            cv::Scalar(0, 255, 0),
            2
        );

        // Identify the nonzero pixels in x and y within the window
        cv::Mat good_inds;
        for (cv::Point& p : nonzero) {
            if (p.y >= win_y_low && p.y < win_y_high) {
                if (p.x >= win_x_low && p.x < win_x_high) {
                    good_inds.push_back(p);
                }
            }
        }

        // Concatinate these indices to the lists
        lane_inds.push_back(good_inds);

        // If you found > minpix pixels, recenter next window on their mean position
        if (good_inds.rows > min_pix) {
            current_x = cv::mean(good_inds)[0];
        }
    }

    // Extract left and right line pixel positions
    line.all_points = lane_inds;

    bool detected = true;
    std::vector<double> fit_pixel, fit_meter;
    if (lane_inds.empty()) {
        fit_pixel = line.last_fit_pixel;
        fit_meter = line.last_fit_meter;
        detected = false;
    } else {
        inverse_polyfit(lane_inds, 2, fit_pixel);
        inverse_polyfit(lane_inds.mul(cv::Scalar(XM_PER_PIX, YM_PER_PIX)), 2, fit_meter);
    }

    if (validate_fit_pixel(fit_pixel)) {
        return;
    }

    // Generate x and y values for plotting
    cv::Mat ploty;
    linear_space(0, height - 1, height, ploty);
    cv::Mat fit_x;
    solve_polyfit(fit_pixel, ploty, fit_x);

    cv::Scalar lane_inds_color;
    if (is_left) {
        lane_inds_color = cv::Scalar(255, 0, 0);
    } else {
        lane_inds_color = cv::Scalar(0, 0, 255);
    }

    // Color in left and right line pixels
    set_pixel_colors(dst, lane_inds, lane_inds_color);

    // Plot lane lines
    set_pixel_colors(dst, merge(fit_x, ploty), cv::Scalar(0, 255, 255));

    line.update(cv::Mat(fit_pixel, true), cv::Mat(fit_meter, true), fit_x, ploty, detected);

    if (verbose) {
        if (is_left) {
            cv::imshow("bird's-eye view left lane detection", dst);
        } else {
            cv::imshow("bird's-eye view right lane detection", dst);
        }
    }
}

void get_fits_by_previous_fits(
    const cv::Mat& src,
    cv::Mat& dst,
    Line& line,
    const bool is_left,
    const bool verbose
) {
    const int width = src.size().width;
    const int height = src.size().height;

    std::vector<double> fit_pixel = line.last_fit_pixel;

    int margin = 120;

    // Create an output image to draw on and  visualize the result
    if (dst.empty()) {
        cv::cvtColor(src, dst, cv::COLOR_GRAY2BGR);
    }

    cv::Mat lane_inds;
    for (int h = 0; h < height; h++) {
        int line_x = line.lane_x.at<int>(h);
        for (int w = -margin; w <= margin; w++) {
            int x = line_x + w;
            if (x >= 0 && x < width) {
                cv::Vec3b clr = dst.at<cv::Vec3b>(h, x);
                if (clr[0] != 0 || clr[1] != 0 || clr[2] != 0) {
                    lane_inds.push_back(cv::Point(x, h));
                }
            }      
        }
    }

    // Extract left and right line pixel positions
    line.all_points = lane_inds;

    bool detected = true;
    std::vector<double> fit_meter;
    if (lane_inds.empty()) {
        fit_pixel = line.last_fit_pixel;
        fit_meter = line.last_fit_meter;
        detected = false;
    } else {
        inverse_polyfit(lane_inds, 2, fit_pixel);
        inverse_polyfit(lane_inds.mul(cv::Scalar(XM_PER_PIX, YM_PER_PIX)), 2, fit_meter);
    }

    if (validate_fit_pixel(fit_pixel)) {
        return;
    }

    // Generate x and y values for plotting
    cv::Mat fit_x;
    solve_polyfit(fit_pixel, line.lane_y, fit_x);

    line.update(cv::Mat(fit_pixel, true), cv::Mat(fit_meter, true), fit_x, line.lane_y, detected);

    cv::Scalar lane_inds_color;
    if (is_left) {
        lane_inds_color = cv::Scalar(255, 0, 0);
    } else {
        lane_inds_color = cv::Scalar(0, 0, 255);
    }

    // Color in left and right line pixels
    set_pixel_colors(dst, lane_inds, lane_inds_color);

    // Plot lane lines
    // set_pixel_colors(dst, merge(fit_x, line.lane_y), cv::Scalar(0, 255, 255));

    if (verbose) {
        if (is_left) {
            cv::imshow("bird's-eye view left lane detection", dst);
        } else {
            cv::imshow("bird's-eye view right lane detection", dst);
        }
    }
}

void draw_back_onto_the_road(
    const cv::Mat& src,
    cv::Mat& dst,
    cv::Mat& road_warped,
    const cv::Mat &minv,
    const Line& line_lt,
    const Line& line_rt,
    int& offset,
    const bool verbose
) {
    if (line_lt.lane_x.empty() || line_rt.lane_x.empty()) {
        src.copyTo(dst);
        return;
    }

    // draw road as green polygon on original frame
    road_warped = find_road_mask(src, line_lt, line_rt, cv::Scalar(0, 255, 0), CV_8UC3);

    // cv::imshow("road", road_warped);

    // warp back to img_cropped
    cv::Mat road_dewarped;
    cv::warpPerspective(road_warped, road_dewarped, minv, src.size());

    // blend back to img_cropped
    cv::addWeighted(src, 1.0, road_dewarped, 0.3, 0, dst);

    // now separately draw solid lines to highlight them
    cv::Mat line_warped = cv::Mat::zeros(src.size(), CV_8UC3);
    line_lt.draw(line_warped, cv::Scalar(255, 0, 0), 20);
    line_rt.draw(line_warped, cv::Scalar(0, 0, 255), 20);

    // draw offset point and midpoint
    int offset_x = (line_rt.lane_x.at<int>(OFFSET_Y) - line_lt.lane_x.at<int>(OFFSET_Y)) / 2 + line_lt.lane_x.at<int>(OFFSET_Y);
	cv::circle(line_warped, cv::Point(WIDTH / 2, OFFSET_Y), 3, cv::Scalar(0, 0, 255), 8);
	cv::circle(line_warped, cv::Point(offset_x, OFFSET_Y), 3, cv::Scalar(0, 255, 0), 8);
    offset = offset_x - WIDTH / 2;

    // warp back to img_cropped
    cv::Mat line_dewarped;
    cv::warpPerspective(line_warped, line_dewarped, minv, src.size());

    // blend back to img_cropped
    cv::addWeighted(dst, 0.8, line_dewarped, 0.5, 0, dst);

    if (verbose) {
        cv::imshow("dst", dst);
    }
}

cv::Mat find_horizontal_lines(
    const cv::Mat& src
) {
    // Create the images that will use to extract the horizonta and vertical lines
    cv::Mat dst = src.clone();

    // Specify size on horizontal axis
    int scale = 15; // play with this variable in order to increase/decrease the amount of lines to be detected
    int horizontalsize = dst.cols / scale;

    // Create structure element for extracting horizontal lines through morphology operations
    cv::Mat horizontalStructure = getStructuringElement(cv::MORPH_RECT, cv::Size(horizontalsize,1));

    // Apply morphology operations
    erode(dst, dst, horizontalStructure, cv::Point(-1, -1));
    dilate(dst, dst, horizontalStructure, cv::Point(-1, -1));

    return dst;
}

cv::Mat find_road_mask(
    const cv::Mat& src,
    const Line& line_lt,
    const Line& line_rt,
    const cv::Scalar& color,
    const int type,
    const int margin
) {
    // draw road as green polygon on original frame
    if (line_lt.lane_x.empty() || line_rt.lane_x.empty()) {
        return cv::Mat::zeros(src.size(), type);
    }

    cv::Mat road_mask = cv::Mat::zeros(src.size(), type);
    cv::Mat lane_left_x = line_lt.lane_x + margin;
    cv::Mat lane_right_x = line_rt.lane_x - margin;
    std::vector<cv::Point> pts = zip(lane_left_x, line_lt.lane_y);
    std::vector<cv::Point> right_pts = zip(lane_right_x, line_rt.lane_y);
    std::reverse(right_pts.begin(), right_pts.end());
    pts.insert(pts.end(), right_pts.begin(), right_pts.end());
    cv::fillPoly(road_mask, pts, color);
    return road_mask;
}

Line find_line_from_another_line(
    const Line& line,
    const bool is_left
) {
    Line ret_line(10);
    ret_line.detected = false;
    cv::Mat margin;
    linear_space(LANE_WIDTH_LOW, LANE_WIDTH_HIGH, HEIGHT, margin);
    if (is_left) {
        ret_line.lane_x = line.lane_x - margin;
    } else {
        ret_line.lane_x = line.lane_x + margin;
    }
    ret_line.lane_y = line.lane_y;
    return ret_line;
}
