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

        std::pair<aabb_t<T, D>, aabb_t<T, D>> bisect() const {
            auto aabbs = std::make_pair(*this, *this);
            auto s = get_size();                  

            int max_i = 0;
            for (int i = 1; i < D; i++){
                if (s[i] > s[max_i]){
                    max_i = i;
                }
            }

            aabbs.first .max[max_i] -= s[max_i];
            aabbs.second.min[max_i] += s[max_i];

            return aabbs;
        }
        
        aabb_t<T, D> operator&&(const aabb_t<T, D> & a) const {
            return aabb_t<T, D>(vec::max(min, a.min), vec::min(max, a.max));
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
