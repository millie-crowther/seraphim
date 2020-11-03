#ifndef NELDER_MEAD_H
#define NELDER_MEAD_H

#include "maths/matrix.h"

namespace seraph { namespace maths { namespace nelder_mead {
    constexpr double alpha = 1.0;
    constexpr double gamma = 2.0;
    constexpr double rho   = 0.5;
    constexpr double sigma = 0.5;

    template<int N>
    struct result_t {
        vec_t<double, N> x;
        double fx; 

        result_t(const vec_t<double, N> & _x, double _fx) : x(_x), fx(_fx){}
        
        struct comparator_t {
            bool operator()(const result_t<N> & a, const result_t<N> & b){
                return a.fx < b.fx;
            }
        };
    };

    template<int N, class F>
    result_t<N> optimise(const F & f, const std::array<vec_t<double, N>, N + 1> & ys){
        std::vector<result_t<N>> xs;
        for (const auto & y : ys){
            xs.emplace_back(y, f(y));
        }

        bool quit = false;
        while (!quit){
            // order
            std::sort(xs.begin(), xs.end(), result_t<N>::comparator_t()); 

            // calculate centroid
            vec_t<double, N> x0;
            for (const auto & x : xs){
                x0 += x.x;
            }
            x0 /= N;

            // reflection
            auto xr = x0 + alpha * (x0 - xs[N].x);
            auto fxr = f(xr);
            if (xs[0].fx <= fxr && fxr < xs[N - 1].fx){
                xs[N] = result_t<N>(xr, fxr);
                continue;
            }

            // expansion
            if (fxr < xs[0].fx){
                auto xe = x0 + gamma * (xr - x0);
                auto fxe = f(xe);
                
                if (fxe < fxr){
                    xs[N] = result_t<N>(xe, fxe);
                } else {
                    xs[N] = result_t<N>(xr, fxr);
                }
                continue
            }

            // contraction
            auto xc = x0 + rho * (xs[N].x - x0);
            auto fxc = f(xc);
            if (fxc < xs[N].fx){
                xs[N] = result_t<N>(xc, fxc);
                continue;
            }   

            // shrink
            auto x1 = xs[0].x;
            for (auto & r : xs){
                r.x = x1 + sigma * (r.x - x1);
                r.fx = f(r.x);
            }
        }

        return result_t(vec3_t::forward(), 0.0);  
    }  
}}}

#endif
