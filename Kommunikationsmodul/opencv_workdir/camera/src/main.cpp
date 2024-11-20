#include <opencv2/opencv.hpp>
#include <future>
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
	CameraReader(cv::VideoCapture&& cap) : cap(std::move(cap)) {}

	~CameraReader() {
		running.clear();
		thread.join();
		cap.release();
	}

	void get_last_frame(cv::Mat& frame) {
		mutex.lock();
		cv::swap(frame_a, frame_b);
		mutex.unlock();
		frame = frame_b;
	}

	void start() {
		running.test_and_set();
		cap >> frame_a;
		thread = std::thread([this]() { 
			while (running.test_and_set()) {
				mutex.lock();	
				cap >> frame_a;
				mutex.unlock();
				// Sleep the thread so that other thread can aquire mutex.
				// Since this thread still has to wait for another frame from the camera,
				//   this is fine.
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		});
	}

private:
	cv::VideoCapture cap;

	cv::Mat frame_a, frame_b;

	std::thread thread;

	std::mutex mutex;

	// Flag used to stop the reading thread on destructor call.
	std::atomic_flag running;
};


int main(int argc, char *argv[]) {
   
	int width = 640;
	int height = 480;
	int fps = std::stoi(argv[1]);
	const std::string addr(argv[2]);
	
	const std::string CAM_SET = "libcamerasrc ! capsfilter caps=video/x-raw,width=640,height=480,framerate="
	+ std::to_string(fps) +
	"/1 ! appsink";
	const std::string OUT_SET = "appsrc ! video/x-raw ! videoconvert ! x264enc tune=zerolatency bitrate=2048 speed-preset=superfast"
	" ! rtph264pay ! udpsink host=" + addr + " port=5000";

    	// Open the video file for reading
	cv::VideoCapture cap(CAM_SET, cv::CAP_GSTREAMER);
	// if not success, exit program	
	if (!cap.isOpened()) {
		std::cout << "failed to read from camera, can't create capture" << std::endl;
		return -1;
	}	
	CameraReader camera_reader(std::move(cap));

	cv::VideoWriter writer;
	writer.open(OUT_SET, 0, static_cast<double>(fps), cv::Size(width, height), true);
	if (!writer.isOpened()){
		std::cout << "=ERR= can't create writer" << std::endl;
		return -1;
	}

	cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat dist = cv::Mat::zeros(4, 1, CV_64F);

	camera_matrix.at<double>(0, 0) = 3.8547620031793207e+02;
	camera_matrix.at<double>(0, 1) = 0.0;
	camera_matrix.at<double>(0, 2) = 3.0731065784030818e+02;
	camera_matrix.at<double>(1, 0) = 0.0;
	camera_matrix.at<double>(1, 1) = 3.8584917668003942e+02;
	camera_matrix.at<double>(1, 2) = 2.6424581870480455e+02;
	camera_matrix.at<double>(2, 0) = 0.0;
	camera_matrix.at<double>(2, 1) = 0.0;
	camera_matrix.at<double>(2, 2) = 1.0;

	dist.at<double>(0, 0) = -3.1972772186288120e-02;
	dist.at<double>(0, 1) = -6.2476921103664559e-02;
	dist.at<double>(0, 2) = 8.1133134131846127e-02;
	dist.at<double>(0 ,3) = -4.2835434417375869e-02;
	cv::Mat new_camera_matrix;
	cv::fisheye::estimateNewCameraMatrixForUndistortRectify(camera_matrix, dist, cv::Size(640, 480), cv::Matx33d::eye(), new_camera_matrix, 1);


	cv::Mat temp, frame;
	int key = 0;
	camera_reader.start();
    	while(key != 27) {
        	camera_reader.get_last_frame(temp);
  

		cv::fisheye::undistortImage(temp, frame, camera_matrix, dist, new_camera_matrix);

		if (frame.empty()) {
			std::cout << "=ERR= frame is empty" << std::endl;
			break;
		}



    		cv::imshow("live stream", frame);
                	
		writer << frame;
		
		key = cv::waitKey(50);
    	} 

    	cv::destroyAllWindows();
	
    	return 0;
}
