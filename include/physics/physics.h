#ifndef PHYSICS_H
#define PHYSICS_H

#include "core/constant.h"
#include "metaphysics/matter.h"

#include <memory>
#include <set>
#include <thread>

namespace srph {
    class physics_t {
    public:
        physics_t();
        ~physics_t();

        void register_matter(std::shared_ptr<matter_t> matter);
        void unregister_matter(std::shared_ptr<matter_t> matter);

        int get_frame_count();

    private:
        bool quit;
        std::thread thread;
        std::set<std::shared_ptr<matter_t>> matters;
        int frames;

        void run();
    };
}

#endif
