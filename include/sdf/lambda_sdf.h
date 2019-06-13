#ifndef LAMBDA_SDF
#define LAMBDA_SDF

#include "sdf/sdf.h"

#include <functional>

template<uint8_t D>
class lambda_sdf_t : public sdf_t<D> {
private:
    std::function<double(const vec_t<double, D>)> f;
    
public:
    lambda_sdf_t(const std::function<double(const vec_t<double, D>)> & f){
        this->f = f;
    }

    double phi(const vec_t<double, D> & x) const override {
        return f(x);
    }
};

#endif