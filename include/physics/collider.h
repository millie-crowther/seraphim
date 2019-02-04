#ifndef COLLIDER_H
#define COLLIDER_H

#include "sdf.h"
#include "logic/emitter.h" 
#include "scene/transform.h"

struct collision_t {
    vec3_t position;
    vec3_t normal;
};

class collider_t : public emitter_t<collision_t> { 
private:
    sdf_t sdf;
    transform_t transform;

public:
    collider_t(const sdf_t & sdf);

    vec3_t get_centre() const;

    void collide(const collider_t & c) const; 
};

#endif