#include "logic/event_emitter.h"

void 
event_emitter_t::on(std::string event, const callback_t& cb){
    if (callbacks.find(event) == callbacks.end()){
        callbacks[event] = std::vector<callback_t>();
    }

    callbacks[event].push_back(cb);
}

void 
event_emitter_t::emit(std::string event){
    if (callbacks.find(event) != callbacks.end()){
        for (auto& callback : callbacks[event]){
            callback();
        } 
    }
}