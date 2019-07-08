#include "logic/interval_revelator.h"

#include <iostream>

interval_revelator_t::interval_revelator_t(){
    previous     = std::chrono::high_resolution_clock::now();
}

void
interval_revelator_t::tick(){
    auto   now   = std::chrono::high_resolution_clock::now();
    double delta = std::chrono::duration_cast<std::chrono::microseconds>(now - previous).count();
    previous     = now;
    
    announce(delta / 1000000);
}