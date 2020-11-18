#ifndef NELDER_MEAD_H
#define NELDER_MEAD_H

#include "maths/matrix.h"

namespace srph { namespace optimise {
    constexpr double alpha = 1.0;
    constexpr double gamma = 2.0;
    constexpr double rho   = 0.5;
    constexpr double sigma = 0.5;
    constexpr int    max_i = 100;

    template<int N>
    struct result_t {
        bool hit;
        vec_t<double, N> x;
        double fx; 

        result_t(){}
        result_t(bool _hit, const vec_t<double, N> & _x, double _fx) : hit(_hit), x(_x), fx(_fx){}
        
        struct comparator_t {
            bool operator()(const result_t<N> & a, const result_t<N> & b){
                return a.fx < b.fx;
            }
        };
    };

    template<int N, class F, class D>
    result_t<N> newton_raphson(const F & f, const D & dfdx, const vec_t<double, N> & x0){
        vec_t<double, N> x = x0;

        for (int i = 0; i < max_i; i++){
            double fx = f(x);
            std::cout << fx << std::endl;

            if (std::abs(fx) < constant::epsilon){
                return result_t<N>(true, x, fx);
            }

            x -= fx / dfdx(x);
        }

        return result_t<N>(false, x, f(x));
    }

    template<int N, class F>
    result_t<N> nelder_mead(const F & f, const std::array<vec_t<double, N>, N + 1> & ys){
        std::vector<result_t<N>> xs;
        for (const auto & y : ys){
            xs.emplace_back(false, y, f(y));
        }
        std::sort(xs.begin(), xs.end(), typename result_t<N>::comparator_t()); 

        for(int i = 0; i < max_i; i++){
            // terminate
            auto minx = xs[0].x;
            auto maxx = xs[0].x;
            for (int j = 1; j < N + 1; j++){
                minx = vec::min(minx, xs[j].x);
                maxx = vec::max(maxx, xs[j].x);
            }

            if (minx == maxx){
                return result_t(true, xs[0].x, xs[0].fx);
            }

            // order
            std::sort(xs.begin(), xs.end(), typename result_t<N>::comparator_t()); 

            // calculate centroid
            vec_t<double, N> x0;
            for (int j = 0; j < N; j++){
                x0 += xs[j].x;
            }
            x0 /= N;

            // reflection
            auto xr = x0 + alpha * (x0 - xs[N].x);
            double fxr = f(xr);
            if (xs[0].fx <= fxr && fxr < xs[N - 1].fx){
                xs[N] = result_t<N>(false, xr, fxr);
                continue;
            }

            // expansion
            if (fxr < xs[0].fx){
                auto xe = x0 + gamma * (xr - x0);
                auto fxe = f(xe);
                
                if (fxe < fxr){
                    xs[N] = result_t<N>(false, xe, fxe);
                } else {
                    xs[N] = result_t<N>(false, xr, fxr);
                }
                continue;
            }

            // contraction
            auto xc = x0 + rho * (xs[N].x - x0);
            double fxc = f(xc);
            if (fxc < xs[N].fx){
                xs[N] = result_t<N>(false, xc, fxc);
                continue;
            }   

            // shrink
            for (int j = 1; j < N + 1; j++){
                xs[j].x = xs[0].x + sigma * (xs[j].x - xs[0].x);
                xs[j].fx = f(xs[j].x);
            }
        }

        return xs[0]; 
    } 

     
}}

#endif
