#ifndef STEERING_H
#define STEERING_H
#include <mutex>
#include "types.hpp"
#include "pathing.hpp"

// Manual steering
void steering_action(char button_action, ButtonStatus &button_status);
void steering_task_manual(SteeringData &steering_data, std::mutex& steering_mutex, const ButtonStatus &button_status);

// Non-manual steering
void steering_task_auto(
	const SensorData& sensor_data, 
	SteeringData& steering_data,
    std::mutex& sensor_mutex,
    std::mutex& steering_mutex,
    const std::vector<std::pair<Node*, PathAction>> &path,
	const CameraData& camera_data,
	const CameraData& prev_camera_data,
	double time_stamp,
	bool slow_down,
	bool stop
);


#endif
