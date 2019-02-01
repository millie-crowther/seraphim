#ifndef EVENT_EMITTER_H
#define EVENT_EMITTER_H

#include <functional>
#include <vector>

#include "logic/scheduler.h"

template<class output_t>
class emitter_t {
public:
    typedef std::function<void(const output_t)> effector_t;

    void on_emit(const effector_t & effector){
        effectors.push_back(effector);
    }

    void emit(const output_t & output){
        for (auto & effector : effectors){
            scheduler::submit([&](){ effector(output); });
        }
    }

private:
    std::vector<effector_t> effectors;
};

#endif