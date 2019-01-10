#ifndef EFFECTOR_H
#define EFFECTOR_H

#include <chrono>
#include <memory>
#include <functional>

#include "scheduler.h"

class scheduler_t;

typedef std::function<void(double, const std::shared_ptr<scheduler_t>&)> effector_func_t;

class effector_t {
private:
    // private fields
    std::chrono::high_resolution_clock::time_point last_tick;
    effector_func_t f;

public:
    effector_t(const effector_func_t& f);

    void tick(const std::shared_ptr<scheduler_t>& scheduler);
};

#endif
