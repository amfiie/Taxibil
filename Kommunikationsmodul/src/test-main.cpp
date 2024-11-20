#include "computer_vision/computer-vision.hpp"
#include <queue>

/*
 *  Borrowed code from URL: https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c 
 */
class InputParser{
public:
    InputParser (int &argc, char **argv){
        for (int i=1; i < argc; ++i)
            this->tokens.push_back(std::string(argv[i]));
    }

    const std::string& getCmdOption(const std::string &option) const{
        std::vector<std::string>::const_iterator itr;
        itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end()){
            return *itr;
        }
        static const std::string empty_string("");
        return empty_string;
    }
    
    bool cmdOptionExists(const std::string &option) const{
        return std::find(this->tokens.begin(), this->tokens.end(), option)
                != this->tokens.end();
    }

private:
    std::vector <std::string> tokens;
};


int main(int argc, char** argv) {
    InputParser input(argc, argv);

    std::string video_path;
 	CameraReader camera_reader;
	bool stream = false;
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
    Direction direction = Direction::DEFAULT;
    bool stop_line_handled = false;

    Direction next_direction = Direction::LEFT;

    while(1) {
        update_camera_data(camera_reader, stream_writer, camera_data, direction, stream, verbose);

        if (camera_data.line_type == LineType::NONE) {
            stop_line_handled = false;
        } else if (
            camera_data.line_type == LineType::STOP && 
            camera_data.distance_to_line < 50 && 
            camera_data.distance_to_line > 20 &&
            !stop_line_handled
        ) {
            stop_line_handled = true;
            if (direction == Direction::DEFAULT) {
                direction = next_direction;
            } else {
                direction =	Direction::DEFAULT;
            }
        }   
    }

    if (verbose) {
        cv::destroyAllWindows();
    }

    return 0;
}
