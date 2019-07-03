#ifndef BRICK_H
#define BRICK_H

#include "render/texture_manager.h"
#include "sdf/sdf.h"

class brick_t {
public:
    // constructors and destructors
    brick_t(
        const vec4_t & aabb, 
        std::weak_ptr<texture_manager_t> texture_manager, 
        const sdf3_t & sdf
    );
    ~brick_t();

    // overloaded operators
    bool operator<(const brick_t & brick) const;

    uint32_t get_id() const;

private:
    // private fields
    std::weak_ptr<texture_manager_t> texture_manager_ptr;
    uint32_t id;
};

#endif