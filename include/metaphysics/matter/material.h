#ifndef MATERIAL_H
#define MATERIAL_H

#include "maths/matrix.h"

struct material_t {
    vec3_t colour;
    double density;         // kg / m^3
    double restitution;

    material_t(){}

    material_t(const vec3_t & colour, double density, double restitution){
        this->colour = colour;
        this->density = density;
        this->restitution = restitution;
    }
};

#endif
