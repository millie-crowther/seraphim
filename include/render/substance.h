#ifndef SUBSTANCE_H
#define SUBSTANCE_H

#include "sdf/sdf.h"

class substance_t {
public:
    struct data_t {
        f32vec3_t x;
        uint32_t root;

        f32vec3_t size;
        float _1;
    };

    std::weak_ptr<sdf3_t> get_sdf() const { return sdf; }

    substance_t(std::shared_ptr<sdf3_t> sdf){ this->sdf = sdf;}

private:
    std::shared_ptr<sdf3_t> sdf;

};

#endif