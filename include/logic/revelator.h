#ifndef EVENT_EMITTER_H
#define EVENT_EMITTER_H

#include <functional>
#include <vector>

#include "logic/scheduler.h"
#include "core/uuid.h"

// TODO: read / write synchronisation on listeners vector

template<class output_t>
class revelator_t {
public:
    typedef std::function<void(const output_t &)> follower_t;

    uuid_t follow(const follower_t & follower){
        followers.push_back(follower);
    }

    void renounce(const uuid_t & apostate){
        
    }

protected:
    // protected constructor enforces abstract class
    revelator_t(){}

    void reveal(const output_t & output) const {
        for (auto & follower : followers){
            scheduler::submit(std::bind(follower, output));
        }
    }

    bool has_followers() const {
        return !followers.empty();
    }

private:
    std::vector<follower_t> followers;
};

#endif