#pragma once

const double YM_PER_PIX = 30 / 720;   // meters per pixel in y dimension
const double XM_PER_PIX = 3.7 / 700;  // meters per pixel in x dimension

const int TIME_WINDOW = 10;        // results are averaged over this number of frames

const int WIDTH = 640;
const int HEIGHT = 480;

const double OFFSET_Y_COEFF = 0.75;
const double OFFSET_WARP_COEFF = 0.5;

const int CROP_X_MARGIN = 120;
const int CROP_WIDTH = WIDTH - CROP_X_MARGIN * 2;

const int CROP_Y_MARGIN = 0;
const int CROP_HEIGHT = HEIGHT - CROP_Y_MARGIN * 2;

const int WARP_X_MARGIN = 50;

const int WARP_X_LOW = WARP_X_MARGIN;
const int WARP_X_HIGH = WIDTH - WARP_X_MARGIN;
const int WARP_Y_LOW = 200;
const int WARP_Y_HIGH = HEIGHT;

const int STOP_LINE_MARGIN = 25;

const int LANE_WIDTH_LOW = 280;
const int LANE_WIDTH_HIGH = 512;

const int OFFSET_Y = HEIGHT - 50;

enum class LineType {
    STOP,
    LEFT,
    RIGHT,
    NONE
};

struct TargetLine {
    LineType line_type;
    float distance = -1.0;
};

enum class Direction {
    STRAIGHT,
    RIGHT,
    LEFT,
    DEFAULT
};

//Temporary
struct CameraData {
	float center_offset_meter;
	LineType line_type = LineType::NONE;
    float distance_to_line = -1;
};
