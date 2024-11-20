#pragma once
#include <opencv2/opencv.hpp>
#include "types.hpp"
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

/**
 * Class for asynchronous reading of camera frames.
 * Frames are read on a separete thread to allow always getting the latest availible camera data. 
 */ 
class CameraReader {
public:
	CameraReader();

	CameraReader(cv::VideoCapture&& cap);

	~CameraReader();

	void get_last_frame(cv::Mat& frame);

	void start();

private:
	cv::VideoCapture cap;

	cv::Mat frame_a, frame_b;

	std::thread thread;

	std::mutex mutex;

	// Flag used to stop the reading thread on destructor call.
	std::atomic_flag running;
};

void update_camera_data(
	CameraReader& camera_reader,
	cv::VideoWriter& stream_writer,
	CameraData& camera_data,
	const PathAction& direction,
	const bool stream,
	const bool verbose
);

void create_camera_reader(CameraReader& reader);

void create_camera_reader(CameraReader& reader, const std::string& filename);

cv::VideoWriter create_camera_writer(
    const std::string& ip_adress
);
