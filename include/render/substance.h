#ifndef SUBSTANCE_H
#define SUBSTANCE_H

#include <memory>

#include "sdf/sdf.h"

class substance_t {
public:
    struct data_t {
        f32vec3_t x;
        int32_t root;

        f32vec3_t size;
        float _1;

        data_t();
    };

    substance_t(uint32_t root, std::shared_ptr<sdf3_t> sdf);

    std::weak_ptr<aabb3_t> get_aabb();
    std::weak_ptr<sdf3_t> get_sdf() const;
    data_t get_data() const;

private:
    data_t data;
    std::shared_ptr<sdf3_t> sdf;
    std::shared_ptr<aabb3_t> aabb;

    void create_aabb();
};

#endif