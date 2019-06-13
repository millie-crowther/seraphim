#ifndef COMPOSITION_H
#define COMPOSITION_H

#include "sdf/lambda_sdf.h"
#include <memory>

template<uint8_t D>
std::shared_ptr<sdf_t<D>> operator&(std::shared_ptr<sdf_t<D>> a, std::shared_ptr<sdf_t<D>> b){
    return std::make_shared<lambda_sdf_t<D>>([=](const vec_t<double, D> & x){
        return std::min(a->phi(x), b->phi(x));
    });
}

#endif  