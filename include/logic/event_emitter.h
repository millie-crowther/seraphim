#ifndef EVENT_EMITTER_H
#define EVENT_EMITTER_H

#include <string>
#include <functional>
#include <map>
#include <vector>

class event_emitter_t {
public:
    typedef std::function<void(void)> callback_t;

    void on(std::string event, const callback_t& cb);

    void emit(std::string event);

private:
    std::map<std::string, std::vector<callback_t>> callbacks;
};

#endif