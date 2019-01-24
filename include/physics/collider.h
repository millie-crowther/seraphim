#ifndef COLLIDER_H
#define COLLIDER_H

#include "sdf.h"

class collider_t { 
private:
    sdf_t sdf;

public:
    struct intersection_t {
        
    };

    collider_t(const sdf_t& sdf);

    intersection_t collide(const collider_t& c) const; 
};

#endif