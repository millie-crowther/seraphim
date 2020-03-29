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

    std::weak_ptr<sdf3_t> get_sdf() const;
    data_t get_data() const;

    substance_t(std::shared_ptr<sdf3_t> sdf);

private:
    data_t data;
    std::shared_ptr<sdf3_t> sdf;

};

#endif