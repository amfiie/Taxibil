#include <iostream>
#include <algorithm>
#include "types.hpp"
#include "steering.hpp"
#include "pathing.hpp"
#include <chrono>

/**
 * Registers a button press or release.
 */
void steering_action(char button_action, ButtonStatus &button_status) {
    switch (button_action) {
        case 0 : // press drive (up arrow)
            button_status.drive_pressed = true;
            break;
        case 1 : // press left arrow
            button_status.left_pressed = true;
            break;
        case 2 : // press right arrow
            button_status.right_pressed = true;
            break;
        case 3 : // press brake (down arrow)
            button_status.brake_pressed = true;
            break;
        case 4 : // release drive
            button_status.drive_pressed = false;
            break;
        case 5 : // release left
            button_status.left_pressed = false;
            break;
        case 6 : // release right
            button_status.right_pressed = false;
            break;
        case 7 : // release brake
            button_status.brake_pressed = false;
            break;
    }
}

void steering_task_manual(SteeringData &steering_data, std::mutex& steering_mutex, const ButtonStatus &button_status) {
	steering_mutex.lock();
	if (button_status.drive_pressed) {
		steering_data.intensity = 3500;
	}
	else if (!button_status.drive_pressed) {
		steering_data.intensity = 0;
	}
	if (button_status.right_pressed == button_status.left_pressed){
		steering_data.turn = 1500;
	}
	else if (button_status.left_pressed) {
		steering_data.turn = 1100;
	}
	else if(button_status.right_pressed) {
		steering_data.turn = 1900;
	}
	if (button_status.brake_pressed) {
		steering_data.is_breaking = true;
	}
	else {
		steering_data.is_breaking = false;
	}
	steering_mutex.unlock();
}


constexpr int DIST_THRESHOLD = 60;
constexpr double KP = 30.0;
constexpr double KD = 20.0;

int to_turn(double angle) {
	return 1500 + angle;
}

void steering_task_auto(
	const SensorData& sensor_data, 
	SteeringData& steering_data, 
	std::mutex& sensor_mutex, 
	std::mutex& steering_mutex, 
	const std::vector<std::pair<Node*, PathAction>> &path, 
	const CameraData& camera_data,
	const CameraData& prev_camera_data,
	double time_step,
	bool slow_down,
	bool stop
){
	sensor_mutex.lock();
	uint16_t dist = sensor_data.dist;
	sensor_mutex.unlock();
	
	
	if (dist < DIST_THRESHOLD || stop) {
		steering_mutex.lock();
		steering_data.is_breaking = true;
		steering_data.intensity = 0;
		steering_mutex.unlock();
	} else {
		double p_component = KP * camera_data.center_offset;
		double d_component = KD / time_step * (camera_data.center_offset - prev_camera_data.center_offset);
		
		double res = p_component + d_component;
			
		uint16_t turn =  to_turn(res);
		turn = std::clamp(turn, MIN_TURN, MAX_TURN);
		
		std::cout << p_component  << ", " << d_component << std::endl;
		std::cout << res << std::endl;
		steering_mutex.lock();
		steering_data.turn = turn;
		if (slow_down) {
			steering_data.intensity = 1500;
		} else {
			steering_data.intensity = 1700;  // MAX_INTENSITY;
		}
		steering_data.is_breaking = false;
		steering_mutex.unlock();
	
	}
}
