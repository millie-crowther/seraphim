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
    class physics_t {
    public:
        physics_t();
        ~physics_t();

        void register_matter(std::shared_ptr<matter_t> matter);
        void unregister_matter(std::shared_ptr<matter_t> matter);

        int get_frame_count();

    private:
        static constexpr int max_contact_points = 16;
        void correct(collision_t & c);

        bool quit;
        std::thread thread;

        std::mutex matters_mutex;

        std::vector<std::shared_ptr<matter_t>> matters;
        std::vector<std::shared_ptr<matter_t>> asleep_matters;

        std::map<std::pair<matter_t *, matter_t *>, std::vector<vec3_t>> contact_points; 

        int frames;

        void run();
    };
}

#endif
