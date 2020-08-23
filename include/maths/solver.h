#ifndef SOLVER_H
#define SOLVER_H

#include "vec.h"

#include <functional>

namespace solver {
    constexpr int max_iterations = 5;

    template<uint8_t N>
    struct result_t {
        double fx;
        vec_t<double, N> x;
    };

    template<uint8_t N, class F, class GradF>
    result_t<N> find_root(
        const F & f, const GradF & grad_f, 
        const vec_t<double, N> & initial_x,
        const vec_t<double, N> & xmin, const vec_t<double, N> & xmax
    ){
        result_t<N> result = {
            f(initial_x),
            initial_x
        };

        for (int i = 0; i < max_iterations; i++){
            
        }

        return result; 
    }
    
    template<uint8_t N, class F>
    result_t<N> find_root(
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
