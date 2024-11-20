#include "timing.hpp"
#include <iostream>

std::chrono::time_point<std::chrono::system_clock> time_point;
int index = 0;


void timing_init() {
    time_point = std::chrono::system_clock::now();
}

void print_time_diff() {
   auto now = std::chrono::system_clock::now();
   const std::chrono::duration<double> diff = now - time_point;
   time_point = now;	
   std::cout << index << ": " << diff.count() << std::endl;
   index++;
}
