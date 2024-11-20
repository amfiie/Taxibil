#pragma once

const int TIME_WINDOW = 10;        // results are averaged over this number of frames

const int X_LOW = 15;
const int Y_LOW = 45;
const int WIDTH = 620;
const int HEIGHT = 390;

const float XM_PER_PIX = 0.12;  // centimeter per pixel in x dimension
const float YM_PER_PIX = 0.4; // centimeter per pixel in y dimension

const double OFFSET_Y_COEFF = 0.75;
const double OFFSET_WARP_COEFF = 0.5;

const int WARP_X_MARGIN = 0;

const int WARP_X_LOW = WARP_X_MARGIN;
const int WARP_X_HIGH = WIDTH - WARP_X_MARGIN;
const int WARP_Y_LOW = 200;
const int WARP_Y_HIGH = HEIGHT;

const int STOP_LINE_MARGIN = 25;

const int LANE_WIDTH_LOW = 204;
const int LANE_WIDTH_HIGH = 452;

const int OFFSET_Y = HEIGHT - 50;
