#ifndef EVENT_EMITTER_H
#define EVENT_EMITTER_H

#include <functional>
#include <map>

#include "core/uuid.h"

// TODO: read / write synchronisation on followers map

template<class T>
class revelator_t {
public:
    uuid_t follow(const std::function<void(const T &)> & follower){
        uuid_t uuid;
        followers[uuid] = follower;
        return uuid;
    }

    void renounce(const uuid_t & apostate){
        auto a = followers.find(apostate);
        if (a != followers.end()){
            followers.erase(a);
        }
    }

protected:
    revelator_t(){}

    void announce(const T & t) const {
        // TODO: perhaps adopt more efficient strategy if map becomes very large
        for (auto & follower : followers){
            follower.second(t);
        }
    }

    bool has_followers() const {
        return !followers.empty();
    }

private:
    std::map<uuid_t, std::function<void(const T &)>> followers;
};

#endif