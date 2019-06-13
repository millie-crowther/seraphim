#ifndef LAMBDA_SDF
#define LAMBDA_SDF

#include "sdf/sdf.h"

template<uint8_t D, class F>
class lambda_sdf_t : public sdf_t<D> {
private:
    F f;
public:
    lambda_sdf_t(const F & f){
        this->f = f;
    }

    double phi(const vec_t<double, D> & x) const override {
        return f(x);
    }
};

#endif