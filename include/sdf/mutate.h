#ifndef MUTATE_H
#define MUTATE_H

#include <memory>

// inverse
template<uint8_t D>
std::shared_ptr<sdf_t<D>>
operator-(std::shared_ptr<sdf_t<D>> sdf){
    struct inverse_t : public sdf_t<D> {
        std::shared_ptr<sdf_t<D>> sdf;
        
        inverse_t(std::shared_ptr<sdf_t<D>> sdf){
            this->sdf = sdf;
        }

        double phi(const vec_t<double, D> & x) const override {
            return -sdf->phi(x);
        }

        vec_t<double, D> normal(const vec_t<double, D> & x) const override {
            return -sdf->normal(x);
        }
    };

    return std::make_shared<inverse_t>(sdf);
}

#endif
