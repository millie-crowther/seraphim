#ifndef MATTER_H
#define MATTER_H

#include "maths/vec.h"

class matter_t {
public:
    matter_t(const vec3_t & colour);

    vec3_t get_colour(const vec3_t & x) const;

private:
    vec3_t colour;
};

#endif