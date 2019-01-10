#include "effector.h"

effector_t::effector_t(const effector_func_t& f){
    last_tick = std::chrono::high_resolution_clock::now();
    this->f = f;
}

void 
effector_t::run(const std::shared_ptr<scheduler_t> scheduler){
    auto now = std::chrono::high_resolution_clock::now();

    double delta = std::chrono::duration<double, std::chrono::seconds::period>(
        now - last_tick
    ).count();

    last_tick = now;

    f(delta, scheduler);
}