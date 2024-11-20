#include <atomic>
#include <chrono>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <functional>
#include <fmt/core.h>
#include "i2c-master.hpp"
#include "server.hpp"
#include "pathing.hpp"
#include "types.hpp"
#include "steering.hpp"
#include "computer_vision/computer-vision.hpp"


class InputParser{
public:
	InputParser() {}

    InputParser (int& argc, char* argv[]){
        for (int i=1; i < argc; ++i)
            this->tokens.push_back(std::string(argv[i]));
    }

    const std::string& getCmdOption(const std::string& option) const{
        std::vector<std::string>::const_iterator itr;
        itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end()){
            return *itr;
        }
        static const std::string empty_string("");
        return empty_string;
    }
    
    bool cmdOptionExists(const std::string& option) const{
        return std::find(this->tokens.begin(), this->tokens.end(), option)
                != this->tokens.end();
    }

private:
    std::vector <std::string> tokens;
};

InputParser input;

void thread_steering(
	const SensorData& sensor_data, 
	SteeringData &steering_data, 
	std::mutex& sensor_mutex, 
	std::mutex& steering_mutex, 
	const ButtonStatus &button_status, 
	bool& manual_steering, 
	std::vector<std::pair<Node*, PathAction>>& path, 
	std::mutex& path_mutex, char& pos
) {
	std::string video_path;
 	CameraReader camera_reader;
	bool stream = input.cmdOptionExists("-s");
	bool verbose = input.cmdOptionExists("-v");
   
    const std::string& filename = input.getCmdOption("-f");
    if (!filename.empty()) {
		create_camera_reader(camera_reader, filename);
    } else {
        create_camera_reader(camera_reader);
	}
	camera_reader.start();

    cv::VideoWriter stream_writer;
    const std::string& ip_adress = input.getCmdOption("-s");
    if (!ip_adress.empty()) {
        stream_writer = create_camera_writer(ip_adress);
		stream = true;
    }

	CameraData camera_data, prev_camera_data;
    PathAction direction = PathAction::NODE;
    bool line_visible = false;
	bool slow_down = false, stop = false;

    PathAction next_direction = PathAction::RIGHT;
	auto time = std::chrono::system_clock::now();
	
    while (true) {
		update_camera_data(camera_reader, stream_writer, camera_data, direction, stream, verbose);
		
		
		
		
		if (manual_steering) {
            steering_task_manual(steering_data, steering_mutex, button_status);
	        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        } else {
			path_mutex.lock();
			if (path.size() == 0) {
				manual_steering = true;
				path_mutex.unlock();
				return;
			}
			PathAction next = path.back().second;
			if (line_visible && (camera_data.line_type == LineType::NONE || camera_data.distance_to_line < 20)) {
				line_visible = false;
				switch (next) {
					case PathAction::NODE: // We have just arrived at a RIGHT line, so keep using PathAction::NODE for direction
						direction = PathAction::NODE;
						pos = to_track_id(path.back().first->id);
						path.pop_back();
						break;
					case PathAction::LEFT:
					case PathAction::RIGHT:
						if (direction == PathAction::NODE) {
							direction = next;
						} else {
							direction = PathAction::NODE;
							pos = to_track_id(path.back().first->id);
							path.pop_back();
						}
						break;
					case PathAction::STOP:
						slow_down = false;
						stop = true;
						break;
				}
			}
			path_mutex.unlock();
			else if (!line_visible && camera_data.line_type != LineType::NONE && camera_data.line_type != LineType::LEFT && 
				camera_data.distance_to_line < 100 && camera_data.distance_to_line > 20) 
			{
				line_visible = true;
				switch (next) {
					case PathAction::STOP:
						slow_down = true;
						break;
				}
			}
			
			/*if (camera_data.line_type == LineType::NONE) {
				stop_line_handled = false;
			} else if (
				camera_data.line_type == LineType::STOP && 
				camera_data.distance_to_line < 100 && 
				camera_data.distance_to_line > 20 &&
				!stop_line_handled
			) {
				stop_line_handled = true;
				if (direction == PathAction::NODE) {
					direction = next_direction;
				} else {
					direction =	PathAction::NODE;
				}
			}*/
			const auto cur_time = std::chrono::system_clock::now();
            std::chrono::duration<double> time_step = cur_time - time;
            time = cur_time;
		
            steering_task_auto(sensor_data, steering_data, sensor_mutex, steering_mutex, path, camera_data, prev_camera_data, time_step.count(), slow_down, stop);
			prev_camera_data = camera_data;
			
        }
    }
}

void thread_i2c(SensorData& sensor_data, const SteeringData& steering_data, std::mutex& sensor_mutex, std::mutex& steering_mutex, I2cMaster& i2c_master) {
    uint8_t steering_buffer[sizeof(SteeringData)]; // Copy the data to avoid locking while sending over i2c.
    uint8_t sensor_buffer[sizeof(SensorData)];
    while(true) {
        steering_mutex.lock();
        std::memcpy(steering_buffer, reinterpret_cast<const uint8_t*>(&steering_data), sizeof(SteeringData));
        steering_mutex.unlock();
        if (i2c_master.write_slave(steering_buffer, sizeof(SteeringData), 2) == -1) {
            fmt::print("Something went wrong while writing. Error {}\n", errno);
        }
	
	if (i2c_master.read_slave(sensor_buffer, sizeof(SensorData), 1) == -1) {
	    fmt::print("Something went wrong while reading. Error{}\n", errno);
        }
	sensor_mutex.lock();
	std::memcpy(reinterpret_cast<uint8_t*>(&sensor_data), sensor_buffer, sizeof(SensorData));
	sensor_mutex.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main(int argc, char* argv[]) {
    std::vector<Node> nodes;
    create_nodes(nodes);

    input = InputParser(argc, argv);
    I2cMaster i2c_master(1);
    auto status = i2c_master.initialise();

    if (status != 0) {
        fmt::print("Something went wrong with I2C initialization. Error {}\n", status);
        return 1;
    }

    ButtonStatus button_status;
    SteeringData steering_data = {1000, 1500, true};
    std::mutex steering_mutex, sensor_mutex, path_mutex;
    bool manual_steering = input.cmdOptionExists("-m");

    SensorData sensor_data;

    std::vector<std::pair<Node*, PathAction>> path;

    in_port_t port_send = 54321, port_rec = 12345;
    std::string port_s = input.getCmdOption("--port-send");
    std::string port_r = input.getCmdOption("--port-rec");
    if (!port_s.empty()) {
        port_send = std::stoi(port_s);
    }
    if (!port_r.empty()) {
    	port_rec = std::stoi(port_r);
    }

    char pos = 'F';

    std::thread server(start_server, std::move(port_send), std::move(port_rec), std::ref(button_status), std::ref(sensor_data), std::ref(steering_data), std::ref(sensor_mutex), std::ref(steering_mutex), std::ref(nodes), std::ref(path), std::ref(path_mutex), std::ref(manual_steering), std::ref(pos));

    std::thread steering(thread_steering, std::ref(sensor_data), std::ref(steering_data), std::ref(sensor_mutex), std::ref(steering_mutex), std::cref(button_status), std::ref(manual_steering), std::ref(path), std::ref(path_mutex), std::ref(pos));
   
    thread_i2c(sensor_data, steering_data, sensor_mutex, steering_mutex, i2c_master); // swap first steering_mutex for sensor_mutex (also in the two rows above, this probably does not work though)

    server.join();
    steering.join();
    return 0;
}
