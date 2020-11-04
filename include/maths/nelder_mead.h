#ifndef NELDER_MEAD_H
#define NELDER_MEAD_H

#include "maths/matrix.h"

namespace srph { namespace nelder_mead {
    constexpr double alpha = 1.0;
    constexpr double gamma = 2.0;
    constexpr double rho   = 0.5;
    constexpr double sigma = 0.5;
    constexpr int    max_i = 1000;

    template<int N>
    struct result_t {
        bool hit;
        vec_t<double, N> x;
        double fx; 

        result_t() : hit(false){}
        result_t(const vec_t<double, N> & _x, double _fx) : x(_x), fx(_fx), hit(true){}
        
        struct comparator_t {
            bool operator()(const result_t<N> & a, const result_t<N> & b){
                return a.fx < b.fx;
            }
        };
    };

    template<int N, class F>
    result_t<N> minimise(const F & f, const std::array<vec_t<double, N>, N + 1> & ys){
        std::vector<result_t<N>> xs;
        for (const auto & y : ys){
            xs.emplace_back(y, f(y));
        }

        while (int i = 0; i < max_i; i++){
            // order
            std::sort(xs.begin(), xs.end(), result_t<N>::comparator_t()); 

            // calculate centroid
            vec_t<double, N> x0;
            for (const auto & x : xs) x0 += x.x;
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
            for (int j = 1; j < N + 1; j++){
                xs[j].x = xs[0].x + sigma * (xs[j].x - xs[0].x);
                xs[j].fx = f(xs[j].x);
            }

            // terminate
            auto minx = xs[0].x;
            auto maxx = xs[0].x;
            for (int j = 1; j < N + 1; j++){
                minx = vec::min(minx, xs[j].x);
                maxx = vec::max(maxx, xs[j].x);
            }

            if (minx == maxx){
                return xs[0];
            }
        }

        return result_t<N>(); 
    }  
}}

#endif
