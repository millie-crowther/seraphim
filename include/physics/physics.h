#ifndef PHYSICS_H
#define PHYSICS_H

#include "collision.h"

#include "core/constant.h"
#include "metaphysics/matter.h"

#include <map>
#include <memory>
#include <set>
#include <thread>

namespace srph {
    struct physics_t {
        physics_t();
        ~physics_t();

        void start();

        void register_matter(srph_matter * matter);
        void unregister_matter(srph_matter * matter);

        int get_frame_count();

        bool quit;
        std::thread thread;

        std::mutex matters_mutex;

        std::vector<srph_matter *> matters;
        std::vector<srph_matter *> asleep_matters;

        int frames;

        void run();
    };
}

#endif
