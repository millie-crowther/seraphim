#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <vector>

#include "maths/vec.h"
#include "collider.h"

class rigidbody_t {
private:
    vec3_t centre_of_mass;
    double mass; 
    std::vector<collider_t> colliders;

public:
    void add_force(const vec3_t& f);
    void add_force_at(const vec3_t& f, const vec3_t& s);
};

#endif 