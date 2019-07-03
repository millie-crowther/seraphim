#ifndef EVENT_EMITTER_H
#define EVENT_EMITTER_H

#include <functional>
#include <map>
#include <memory>

#include "core/uuid.h"

// TODO: read / write synchronisation on followers map

template<class T>
class revelator_t {
public:
    class follower_t : public uuid_t {
    private:
        revelator_t * revelator;

    public:
        follower_t(revelator_t<T> * revelator){
            this->revelator = revelator;
        }

        ~follower_t(){
            revelator->renounce(*this);
        }
    };

    typedef std::shared_ptr<follower_t> follower_ptr_t;

    follower_ptr_t follow(const std::function<void(const T &)> & follower){
        std::shared_ptr<follower_t> f = std::make_shared<follower_t>(this);
        followers[*f] = follower;
        return f;
    }

    void announce(const T & t) const {
        // TODO: perhaps adopt more efficient strategy if map becomes very large
        for (auto & follower : followers){
            follower.second(t);
        }
    }

    bool has_followers() const {
        return !followers.empty();
    }

    void renounce(const follower_t & apostate){
        auto a = followers.find(apostate);
        if (a != followers.end()){
            followers.erase(a);
        }
    }

private:
    std::map<follower_t, std::function<void(const T &)>> followers;
};

#endif