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
    };

    substance_t(uint32_t root, std::shared_ptr<sdf3_t> sdf);

    std::weak_ptr<sdf3_t> get_sdf() const;
    data_t get_data();

private:
    uint32_t root;
    std::shared_ptr<sdf3_t> sdf;
    std::unique_ptr<aabb3_t> aabb;

    void create_aabb();
};

#endif