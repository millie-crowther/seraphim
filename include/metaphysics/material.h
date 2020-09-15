#ifndef MATERIAL_H
#define MATERIAL_H

#include "core/debug.h"
#include "maths/matrix.h"

struct material_t {
    vec3_t colour;
    double density;         // kg / m^3
    double restitution;
    double static_friction;
    double dynamic_friction;

    material_t(){}

    material_t(
        const vec3_t & colour, double density, double restitution, double static_friction, 
        double dynamic_friction
    ){
        this->colour = colour;
        this->density = density;
        this->restitution = restitution;
        this->static_friction = static_friction;    
        this->dynamic_friction = dynamic_friction;

#if SERAPHIUM_DEBUG
        if (restitution <= 0.0 || restitution >= 1.0){
            throw std::runtime_error("Error: restitution not in range (0...1) exclusive.");
        } 
#endif  
    }
};

#endif
