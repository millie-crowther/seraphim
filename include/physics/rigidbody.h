#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <vector>

#include "maths/quat.h"
#include "maths/vec.h"
#include "collider.h"
#include "scene/transform.h"

class rigidbody_t {
private:
    vec3_t centre_of_mass;
    double mass; 

    vec3_t velocity;
    quat_t omega;

    std::vector<collider_t> colliders;

    transform_t transform;
    
public:
    void add_force(const vec3_t & f);
    void add_force_at(const vec3_t & f, const vec3_t & s);

    void add_child(transform_t & child);
};

#endif 