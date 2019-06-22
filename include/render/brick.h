#ifndef BRICK_H
#define BRICK_H

#include "render/texture_manager.h"
#include "sdf/sdf.h"

class brick_t {
public:
    // types
    struct data_t {
        f32vec2_t n;
        float d;
        u16vec2_t uv;

        data_t(){}

        data_t(f32vec2_t n, float d, uint32_t uv){
            this->n  = n;
            this->d  = d;
            this->uv = uv;
        }
    };

    // constructors and destructors
    brick_t(); // TODO: remove
    brick_t(
        const vec3_t & x, 
        std::weak_ptr<texture_manager_t> texture_manager, 
        std::weak_ptr<sdf3_t> sdf, 
        data_t * data
    );
    ~brick_t();

private:
    // private fields
    std::weak_ptr<texture_manager_t> texture_manager_ptr;
    u16vec2_t uv;
};

#endif