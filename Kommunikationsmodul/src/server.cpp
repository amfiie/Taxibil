#include <chrono>
#include <iostream>
#include <thread>
#include <fmt/core.h>
#include "server.hpp"
#include "steering.hpp"
#include "types.hpp"

void receive_data(
    sockpp::tcp_socket sock, 
    ButtonStatus& button_status, 
    bool& manual_steering, 
    std::vector<Node>& nodes, 
    std::vector<std::pair<Node*, PathAction>> &path,
    std::mutex& path_mutex,
    char& pos
){
    uint8_t buf[512];

    while(sock.read(buf, sizeof(buf)) > 0) {
        uint8_t identifier = buf[0];
        switch (identifier) {
            case 101: // Button press
                steering_action(buf[1], button_status);
                break;
	        case 102: // switch to manual/automatic steering
		        manual_steering = !manual_steering;
			path_mutex.lock();
			path.clear();
			path_mutex.unlock();
		        break;
            case 103: { // Recive new automatic task
                manual_steering = false;
                if (buf[1] > 5 || buf[2] > 5) break;
                path_mutex.lock();
		path.clear();
                parse_path(find_path(&nodes[buf[1]], &nodes[buf[2]]), path);
                reset_nodes(nodes);
                parse_path(find_path(&nodes[to_index(pos)], &nodes[buf[1]]), path);
                reset_nodes(nodes);
                path_mutex.unlock();
                break;
            }
            case 104:
                break;
        }


    }    
    fmt::print("Connection closed from {}\n", sock.peer_address().to_string());
}

void fill_buffer(uint8_t* buf, SensorData sensor_data, SteeringData steering_data, std::mutex& sensor_mutex, std::mutex& steering_mutex, uint8_t identifier){
    buf[0] = identifier;
    sensor_mutex.lock();
    buf[1] = sensor_data.dist & 0xFF;
    buf[2] = (sensor_data.dist >> 8) & 0xFF;
    buf[3] = sensor_data.vel_r & 0xFF;
    buf[4] = (sensor_data.vel_r >> 8) & 0xFF;
    buf[5] = sensor_data.vel_l & 0xFF;
    buf[6] = (sensor_data.vel_l >> 8) & 0xFF;
    buf[7] = sensor_data.angle & 0xFF;
    buf[8] = (sensor_data.angle >> 8) & 0xFF;
    sensor_mutex.unlock();
    steering_mutex.lock();
    buf[9] = steering_data.intensity & 0xFF;
    buf[10] = (steering_data.intensity >> 8) & 0xFF;
    buf[11] = steering_data.turn & 0xFF;
    buf[12] = (steering_data.turn >> 8) & 0xFF;
    buf[13] = steering_data.is_breaking ? 1 : 0;
    steering_mutex.unlock();
}

void send_data(
    sockpp::tcp_socket sock, 
    SensorData& sensor_data,
    SteeringData& steering_data, 
    std::mutex& sensor_mutex, 
    std::mutex& steering_mutex,
    bool& manual_steering, 
    std::vector<std::pair<Node*, PathAction>> &path, 
    std::mutex& path_mutex,
    char& pos
) {
	uint8_t buf[1+sizeof(SensorData)+sizeof(SteeringData)];
	uint8_t identifier = 200;
	while(true) {
		fill_buffer(buf, sensor_data, steering_data, sensor_mutex, steering_mutex, identifier);
    	sock.write_n(buf, sizeof(buf));
	// send path
	uint8_t path_buf[3 + path.size()];
        path_buf[0] = 201;
	path_buf[1] = path.size() + 1;
        path_buf[2] = pos;
        path_mutex.lock();
	for(int i = 0; i < (int) path.size(); i++) {
        	path_buf[3 + i] = to_track_id(path[path.size() - i - 1].first->id);
	}
        path_mutex.unlock();
        sock.write_n(path_buf, sizeof(path_buf));
	uint8_t manual_steering_buf[2];
	manual_steering_buf[0] = 202;
	manual_steering_buf[1] = manual_steering ? 1 : 0;
	sock.write_n(manual_steering_buf, sizeof(manual_steering_buf));
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }	
}

void start_server(
    const in_port_t port_send, 
    const in_port_t port_rec, 
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
) {
    sockpp::socket_initializer sock_init;
    sockpp::tcp_acceptor acc_send(port_send);
    sockpp::tcp_acceptor acc_rec(port_rec);

    if(!acc_send | !acc_rec){
        std::cerr << "Error creating the acceptor: " << '\n';
	
        return;
    }
    fmt::print("Awaiting connection on port {}\n", port_rec);

    while (true){
        sockpp::inet_address peer;
        sockpp::tcp_socket sock_send = acc_send.accept(&peer);
        sockpp::tcp_socket sock_rec = acc_rec.accept(&peer);
        fmt::print("Received a connection request from {}\n", peer.to_string());

        if (!sock_send | !sock_rec){
            std::cerr << fmt::format("Error accepting incoming connection {}\n", acc_rec.last_error_str());
        } else{
	        std::thread send(send_data, std::move(sock_send), std::ref(sensor_data), std::ref(steering_data), std::ref(sensor_mutex), std::ref(steering_mutex), std::ref(manual_steering), std::ref(path), std::ref(path_mutex), std::ref(pos));
            std::thread receive(receive_data, std::move(sock_rec), std::ref(button_status), std::ref(manual_steering), std::ref(nodes), std::ref(path), std::ref(path_mutex), std::ref(pos));
            send.detach();
	    receive.detach();
        }
    }
}
