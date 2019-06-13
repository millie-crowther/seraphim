#ifndef MUTATE_H
#define MUTATE_H

#include "sdf/lambda_sdf.h"

#include <memory>

// inverse
template<uint8_t D>
std::shared_ptr<sdf_t<D>>
operator-(std::shared_ptr<sdf_t<D>> sdf){
    return std::make_shared<lambda_sdf_t<D>>([=](const vec_t<double, D> & x){
        return -sdf->phi(x);
    });
}

#endif
