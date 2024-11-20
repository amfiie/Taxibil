#pragma once
#include <opencv2/opencv.hpp>
#include "types.hpp"
#include "line-utils.hpp"

void prepare_out_blend_frame(
	const cv::Mat& blend_on_road,
	const cv::Mat& img_binary,
	const cv::Mat& img_birdeye,
	const cv::Mat& img_fit,
	const double offset_pixel,
	const CameraData& camera_data,
	const PathAction& direction,
	const double duration,
	cv::Mat& dst
);

void compute_distance_to_stop_line(
    cv::Mat& stop_lines,
	const Line& line_lt,
	const Line& line_rt,
	CameraData& camera_data
);

CameraData process_pipeline(
	const cv::Mat& src, 
	cv::Mat& dst,
	cv::Mat& road_warped,
	int& processed_frames,
	Line& line_lt,
	Line& line_rt,
	const PathAction& direction,
	const bool& keep_state=true
);