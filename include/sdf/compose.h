#ifndef COMPOSITION_H
#define COMPOSITION_H

#include "sdf/lambda_sdf.h"
#include <memory>
#include <set>

namespace compose {
    template<uint8_t D>
    std::shared_ptr<sdf_t<D>>
    union_(const std::set<std::shared_ptr<sdf_t<D>>> & sdfs) {
        return std::make_shared<lambda_sdf_t<D>>([=](const vec_t<double, D> & x){
            double p = constant::rho;

            for (auto sdf : sdfs){
                p = std::min(p, sdf->phi(x));
            }

            return p;
        });
    }
}

// union
template<uint8_t D>
std::shared_ptr<sdf_t<D>> 
operator|(std::shared_ptr<sdf_t<D>> a, std::shared_ptr<sdf_t<D>> b){
    return std::make_shared<lambda_sdf_t<D>>([=](const vec_t<double, D> & x){
        return std::min(a->phi(x), b->phi(x));
    });
}

// intersection
template<uint8_t D>
std::shared_ptr<sdf_t<D>> 
operator&(std::shared_ptr<sdf_t<D>> a, std::shared_ptr<sdf_t<D>> b){
    return std::make_shared<lambda_sdf_t<D>>([=](const vec_t<double, D> & x){
        return std::max(a->phi(x), b->phi(x));
    });
}

// difference
template<uint8_t D>
std::shared_ptr<sdf_t<D>> 
operator-(std::shared_ptr<sdf_t<D>> a, std::shared_ptr<sdf_t<D>> b){
    return std::make_shared<lambda_sdf_t<D>>([=](const vec_t<double, D> & x){
        return std::max(a->phi(x), -b->phi(x));
    });
}


#endif  