#ifndef PHYSICS_H
#define PHYSICS_H

#include "core/hyper.h"
#include "substance/matter/matter.h"

#include <memory>
#include <set>
#include <thread>

class physics_t {
public:
    physics_t();
    ~physics_t();

    void register_matter(std::shared_ptr<matter_t> matter);
    void unregister_matter(std::shared_ptr<matter_t> matter);

private:
    bool quit;
    std::thread thread;
    std::set<std::shared_ptr<matter_t>> matters;

    void run();
};


#endif