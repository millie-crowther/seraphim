#ifndef COMPOSE_H
#define COMPOSE_H

#include "sdf.h"

#include <memory>

namespace compose {
    template<uint8_t D>
    class intersection_t : public sdf_t<D> {
    private:
        std::shared_ptr<sdf_t<D>> a;
        std::shared_ptr<sdf_t<D>> b;
    
    public:
        intersection_t(std::shared_ptr<sdf_t<D>> a, std::shared_ptr<sdf_t<D>> b){
            this->a = a;
            this->b = b;
        }

        vec_t<double, D> normal(const vec_t<double, D> & x){
            if (a->phi(x) > b->phi(x)){
                return a->normal(x);
            } else {
                return b->normal(x);
            }
        }

        bool contains(const vec_t<double, D> & x){
            return a->contains(x) && b->contains(x);
        }

        aabb_t<double, D> get_aabb(){
            auto aabb_a = a->get_aabb();
            auto aabb_b = b->get_aabb();
            return aabb_t<double, D>(
                aabb_a.get_min().max(aabb_b.get_min()),
                aabb_a.get_max().min(aabb_b.get_max())
            );
        }
    };

    namespace operators {
        template<uint8_t D>
        std::shared_ptr<intersection_t<D>>
        operator&&(std::shared_ptr<sdf_t<D>> a, std::shared_ptr<sdf_t<D>> b){
            return std::make_shared<intersection_t<D>>(a, b);
        }
    } 
}

#endif
