#pragma once
#include "sockpp/tcp_acceptor.h"
#include "types.hpp"
#include "pathing.hpp"
#include <mutex>
#include <atomic>


void start_server(
    const in_port_t, const in_port_t, 
    ButtonStatus& button_status, 
    SensorData& sensor_data, 
    SteeringData& steering_data, 
    std::mutex& sensor_mutex, 
    std::mutex& steering_mutex, 
    std::vector<Node>& nodes,
    std::vector<std::pair<Node*, PathAction>> &path,
    std::mutex& path_mutex,
    bool& manual_steering,
    char& pos
);
