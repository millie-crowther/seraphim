#ifndef AABB_H
#define AABB_H

#include "matrix.h"

#include <limits>
#include <random>

namespace srph {
    template<class T, uint8_t D>
    class aabb_t {
    private:
        vec_t<T, D> min;
        vec_t<T, D> max;

    public:
        aabb_t() : aabb_t(
            vec_t<T, D>(std::numeric_limits<T>::max()),
            vec_t<T, D>(std::numeric_limits<T>::min())
        ){}
        
        aabb_t(const vec_t<T, D> & min, const vec_t<T, D> & max){
            this->min = min;
            this->max = max;
        }

        bool contains(const vec_t<T, D> & x) const {
            return vec::all(x >= min && x <= max);
        } 

        void capture_point(const vec_t<T, D> & c){
            min = vec::min(min, c);
            max = vec::max(max, c); 
        }
        
        vec_t<T, D> get_size() const {
            return max / 2 - min / 2;
        }

        vec_t<T, D> get_max() const {
            return max;
        }

        vec_t<T, D> get_min() const {
            return min;
        }

        vec_t<T, D> get_centre() const {
            return max / 2 + min / 2;
        }

        vec_t<T, D> get_vertex(int i) const {
            vec_t<T, D> v = min;
            for (int j = 0; j < D; j++){
                if (i & (1 << j)){
                    v[j] = max[j];
                }
            }

            return v;
        }

        
        aabb_t<T, D> operator&&(const aabb_t<T, D> & a) const {
            return aabb_t<T, D>(vec::max(min, a.min), vec::min(max, a.max));
        }

        aabb_t<T, D> operator||(const aabb_T<T, D> & a) const {
            return aabb_t<T, D>(vec::min(min, a.min), vec::max(max, a.max));
        }

        bool is_valid() const {
            return vec::all(min <= max);
        }

        void enlarge(const T & t){
            min -= t;
            max += t;
        }
    };
    
    using aabb2_t = aabb_t<double, 2>;
    using aabb3_t = aabb_t<double, 3>;
    using aabb4_t = aabb_t<double, 4>;
}

#endif
