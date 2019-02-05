#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <functional>

namespace scheduler {    
    void halt();

    typedef std::function<void(void)> effector_t;

    void submit(const effector_t & effector);
    void submit_after(const effector_t & effector, double delay);
}

#endif
