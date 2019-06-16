#ifndef COMPOSITION_H
#define COMPOSITION_H

#include "sdf/lambda_sdf.h"
#include <memory>
#include <set>

// TODO: make this more succinct
namespace compose {
    template<uint8_t D>
    class union_t : public sdf_t<D> {
    private:
        std::vector<std::shared_ptr<sdf_t<D>>> sdfs;

    public:
        union_t(const std::vector<std::shared_ptr<sdf_t<D>>> & sdfs){
            this->sdfs = sdfs;
        }

        double phi(const vec_t<double, D> & x) const override {
            double p = constant::rho;

            for (auto sdf : sdfs){
                p = std::min(p, sdf->phi(x));
                if (p <= -constant::rho){
                    return p;
                }
            }

            return p;
        }

        vec_t<double, D> normal(const vec_t<double, D> & x) const override {
            std::shared_ptr<sdf_t<D>> min_sdf = nullptr;
            double min_phi;

            for (auto sdf : sdfs){
                double phi = sdf->phi(x);
                if (min_sdf == nullptr || phi < min_phi){
                    min_sdf = sdf;
                    min_phi = phi;
                }
            }

            if (min_sdf == nullptr){
                return vec_t<double, D>();
            } else { 
                return min_sdf->normal(x);
            }
        }
    };
}

// union
template<uint8_t D>
std::shared_ptr<sdf_t<D>>
operator|(std::shared_ptr<sdf_t<D>> a, std::shared_ptr<sdf_t<D>> b){
    return std::make_shared<compose::union_t>({ a, b });
}

// TODO: the below operators can have more efficient normal 
//       function overrides

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