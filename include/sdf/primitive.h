#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <memory>

#include "sdf/sdf.h"

// TODO: remove 'centre' param from all SDFs
//       translation operation should be performed by transform_t class

namespace primitive {
    class cube_t : public sdf_t {
    private:
        vec3_t c;
        double s;
    public:
        cube_t(const vec3_t & c, double s);
        double phi(const vec3_t & x) const override;
    };

    class sphere_t : public sdf_t {
    private:
        vec3_t c;
        double r;
    public:
        sphere_t(const vec3_t & c, double r);
        double phi(const vec3_t & x) const override;
        vec3_t normal(const vec3_t & x) const override;
    };    
    
    class floor_t : public sdf_t {
    public:
        double phi(const vec3_t & x) const override;
        vec3_t normal(const vec3_t & x) const override;
    };
}

#endif