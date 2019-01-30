#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "maths/vec.h"

class rigidbody_t {
private:
    vec3_t centre_of_mass;
    double mass;

public:
    void add_force(const vec3_t& f);
    void add_force_at(const vec3_t& f, const vec3_t& s);
};

#endif 