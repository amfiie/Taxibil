#ifndef TYPES_H
#define TYPES_H

constexpr uint16_t MAX_TURN = 2000, MIN_TURN = 1000, NO_TURN = 1500, MAX_INTENSITY = 1500;

struct SensorData {
    uint16_t dist;
    uint16_t vel_r;
    uint16_t vel_l;
    int16_t angle;
} __attribute__((packed));

struct SteeringData {
    uint16_t intensity;
    uint16_t turn;
    bool is_breaking;
} __attribute__((packed));


#ifdef COMMUNICATION_MODULE
struct ButtonStatus {
    bool drive_pressed = false;
    bool reverse_pressed = false;
    bool left_pressed = false;
    bool right_pressed = false;
    bool brake_pressed = true;
};

enum class LineType {
    STOP,
    LEFT,
    RIGHT,
    NONE
};

enum class PathAction {
    RIGHT,
    LEFT,
    STOP,
	NODE
};

/*
 * Borrowed code from URL: https://stackoverflow.com/questions/5093460/how-to-convert-an-enum-type-variable-to-a-string 
 */
inline const std::string ToString(LineType v)
{
    switch (v)
    {
        case LineType::STOP:   return "stop";
        case LineType::LEFT:   return "left";
        case LineType::RIGHT:  return "right";
        case LineType::NONE:   return "none";
        default:               return "[Unknown LineType]";
    }
}

/*
 * Borrowed code from URL: https://stackoverflow.com/questions/5093460/how-to-convert-an-enum-type-variable-to-a-string 
 */
inline const std::string ToString(PathAction v)
{
    switch (v)
    {
        case PathAction::LEFT:       return "left";
        case PathAction::RIGHT:      return "right";
        case PathAction::STOP:    	 return "stop";
		case PathAction::NODE:		 return "node";
        default:                     return "[Unknown Direction]";
    }
}

struct CameraData {
	float center_offset = 0.0;
	LineType line_type = LineType::NONE;
    float distance_to_line = -1.0;
};

#endif
#endif
