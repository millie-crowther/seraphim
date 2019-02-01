#ifndef COLLIDER_H
#define COLLIDER_H

#include "sdf.h"
#include "logic/event_emitter.h"

class collider_t : public event_emitter_t { 
private:
    sdf_t sdf;

public:
    struct intersection_t {
        bool   is_hit;
        vec3_t position;
        double distance;
    };

    collider_t(const sdf_t& sdf);

    intersection_t collide(const collider_t& c) const; 
};

#endif