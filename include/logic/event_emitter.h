#ifndef EVENT_EMITTER_H
#define EVENT_EMITTER_H

#include <string>
#include <functional>

class event_emitter_t {
private:

public:
    typedef std::function<void(void)> callback_t;

    void on(std::string event, const callback_t& cb){

    }

    void emit(std::string event){
        
    }
};

#endif