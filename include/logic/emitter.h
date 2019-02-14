#ifndef EVENT_EMITTER_H
#define EVENT_EMITTER_H

#include <functional>
#include <vector>

#include "logic/scheduler.h"

// TODO: read / write synchronisation on effectors vector

template<class output_t>
class emitter_t {
public:
    typedef std::function<void(const output_t &)> effector_t;

    void consume(const effector_t & effector){
        effectors.push_back(effector);
    }

    void emit(const output_t & output){
        for (auto & effector : effectors){
            scheduler::submit([&](){ effector(output); });
        }
    }

    bool has_listeners() const {
        return !effectors.empty();
    }

private:
    std::vector<effector_t> effectors;
};

#endif