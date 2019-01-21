#ifndef COLLIDER_H
#define COLLIDER_H

#include "maths/sdf.h"

class collider_t {
private:

public:
    struct intersection_t {
        
    };

    intersection_t intersect(const collider_t& c) const;
};

#endif