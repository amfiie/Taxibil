#include "perspective-utils.hpp"
#include "maths.hpp"
#include "globals.hpp"

double undistortion_constants[] = {
    3.8547620031793207e+02,
    0.0,
    3.0731065784030818e+02,
    0.0,
    3.8584917668003942e+02,
    2.6424581870480455e+02,
    0.0,
    0.0,
    1.0
};

double dist[] = {
    -3.1972772186288120e-02,
    -6.2476921103664559e-02,
    8.1133134131846127e-02,
    -4.2835434417375869e-02
};

const cv::Mat undistortion_matrix(3, 3, CV_64F, &undistortion_constants);
const cv::Mat dist_data(1, 4, CV_64F, &dist);

void birdeye(
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
    cv::warpPerspective(src, dst, m, src.size());

    if (verbose) {
        cv::imshow("birds-eye view", dst);
    }
}

void undistort(
    const cv::Mat& src,
    cv::Mat& dst
) {
    static cv::Mat camera_matrix = []() {
        cv::Mat temp;
        cv::fisheye::estimateNewCameraMatrixForUndistortRectify(
            undistortion_matrix,
            dist_data,
            cv::Size(640, 480),
            cv::Matx33d::eye(),
            temp,
            1
        );
        return temp;
    }();

    cv::Mat undistort;
    cv::fisheye::undistortImage(src, undistort, undistortion_matrix, dist_data, camera_matrix);

    dst = undistort(cv::Rect(X_LOW, Y_LOW, WIDTH, HEIGHT));
}