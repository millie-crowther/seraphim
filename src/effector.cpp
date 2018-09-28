#include "effector.h"

effector_t::effector_t(){
    last_tick = std::chrono::high_resolution_clock::now();
}

void 
effector_t::tick(float delta, const input_t& input){
    // by default, no behaviour
}

void 
effector_t::tick_template(const input_t& input){
    auto now = std::chrono::high_resolution_clock::now();

    float delta = std::chrono::duration<float, std::chrono::seconds::period>(
        now - last_tick
    ).count();

    last_tick = now;

    tick(delta, input);
}