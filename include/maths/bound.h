#ifndef SERAPHIM_BOUND
#define SERAPHIM_BOUND

#include "matrix.h"
#include "interval.h"

namespace srph {
    template<class T, int N>
    class bound_t : public vec_t<interval_t<T>, N> {
    public:
        bound_t(){}
        
        bound_t(const vec_t<T, N> & lower, const vec_t<T, N> & upper){
            for (int i = 0; i < N; i++){
                (*this)[i] = interval_t<T>(lower[i], upper[i]);
            }
        }

        bool contains(const vec_t<T, N> & x) const {
            for (int i = 0; i < N; i++){
                if (!(*this)[i].contains(x[i])){    
                    return false;
                }
            }
            return true;
        }

        bool is_valid() const {
            return contains(get_midpoint());
        }

        vec_t<T, N> get_lower() const {
            vec_t<T, N> lower;
            auto f = [](const auto & x){ return x.get_lower(); };
            std::transform(this->begin(), this->end(), lower.begin(), f);
            return lower;
        }
                    
        vec_t<T, N> get_upper() const {
            vec_t<T, N> upper;
            auto f = [](const auto & x){ return x.get_upper(); };
            std::transform(this->begin(), this->end(), upper.begin(), f);
            return upper;
        }

        vec_t<T, N> get_midpoint() const {
            return get_lower() / 2 + get_upper() / 2;
        }

        vec_t<T, N> get_width() const {
            return get_upper() / 2 - get_lower() / 2;
        }

        void capture(const vec_t<T, N> & x){
            for (int i = 0; i < N; i++){
                (*this)[i].capture(x[i]);
            }
        }

        bound_t<T, N> operator&&(const bound_t<T, N> & b) const {
            return bound_t<T, N>(vec::max(get_lower(), b.get_lower()), vec::min(get_upper(), b.get_upper()));
        }        
        
        bound_t<T, N> operator||(const bound_t<T, N> & b) const {
            return bound_t<T, N>(vec::min(get_lower(), b.get_lower()), vec::max(get_upper(), b.get_upper()));
        }    

        void enlarge(const T & x){
            *this += interval_t<T>(-x, x);
        }    
    };


    using bound3_t = bound_t<double, 3>;
    using bound4_t = bound_t<double, 4>;
}

#endif
