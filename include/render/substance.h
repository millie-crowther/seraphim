#ifndef SUBSTANCE_H
#define SUBSTANCE_H

#include <memory>

#include "sdf/sdf.h"

class substance_t {
public:
    struct data_t {
        f32vec3_t c;
        int32_t root;

        f32vec3_t r;
        uint32_t id;
    };

    substance_t(uint32_t id, int32_t root, std::shared_ptr<sdf3_t> sdf);

    std::weak_ptr<sdf3_t> get_sdf() const;
    std::weak_ptr<aabb3_t> get_aabb();
    data_t get_data();
    uint32_t get_id() const;

    vec3_t get_position() const;
    void set_position(const vec3_t & x);

private:
    int32_t root;
    uint32_t id;
    std::shared_ptr<sdf3_t> sdf;
    std::shared_ptr<aabb3_t> aabb;
    vec3_t position;

    void create_aabb();
};

#endif