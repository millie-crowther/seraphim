#ifndef SOLVER_H
#define SOLVER_H

#include "vec.h"

#include <functional>

namespace solver {
    constexpr int max_iterations = 50;

    template<uint8_t N>
    struct result_t {
        double fx;
        vec_t<double, N> x;
    };

    template<uint8_t N, class F, class GradF>
    result_t<N> minimise(
        const F & f, const GradF & grad_f, 
        const vec_t<double, N> & initial_x,
        const vec_t<double, N> & xmin, const vec_t<double, N> & xmax
    ){
        auto x = initial_x;
        auto fx = f(x);

        for (int i = 0; i < max_iterations; i++){
            auto E = grad_f(x) * std::abs(fx);
            x = vec::clamp(x - E, xmin, xmax);
            fx = f(x);
            
            if (fx < hyper::epsilon){
                return { fx, x };
            } 
        }

        return { fx, x };
    }
    
    template<uint8_t N, class F>
    result_t<N> minimise(
        const F & f, const vec_t<double, N> initial_x,
        const vec_t<double, N> & xmin, const vec_t<double, N> & xmax
    ){
        auto grad_f = [f](const vec_t<double, N> & x){
            return vec_t<double, N>::grad(f, x);
        };

        return minimise(f, grad_f, initial_x, xmin, xmax);
    }
}

#endif
