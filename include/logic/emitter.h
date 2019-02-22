#ifndef EVENT_EMITTER_H
#define EVENT_EMITTER_H

#include <functional>
#include <vector>

#include "logic/scheduler.h"

// TODO: read / write synchronisation on listeners vector

template<class output_t>
class emitter_t {
public:
    typedef std::function<void(const output_t)> listener_t;

    void listen(const listener_t & listener){
        listeners.push_back(listener);
    }

    void emit(const output_t & output){
        for (auto & listener : listeners){
            scheduler::submit(std::bind(listener, output));
        }
    }

protected:
    bool has_listeners() const {
        return !listeners.empty();
    }

private:
    std::vector<listener_t> listeners;
};

#endif