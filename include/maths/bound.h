#ifndef SERAPHIM_BOUND
#define SERAPHIM_BOUND

#include "matrix.h"
#include "interval.h"

namespace srph {
    template<class T, int N>
    class bound_t : public vec_t<interval_t<T>, N> {
    public:
        bound_t(){}
    
        bound_t(const vec_t<interval_t<T>, N> & b){
            std::copy(b.begin(), b.end(), this->begin());
        }
    
        template<typename... Xs> 
        bound_t(const interval_t<T> & i, Xs... is) : vec_t<interval_t<T>, N>(i, is...){}
   
        bound_t(const vec_t<T, N> & lower, const vec_t<T, N> & upper){
            for (int i = 0; i < N; i++){
                (*this)[i] = interval_t<T>(lower[i], upper[i]);
            }
        }

        bool intersects(const bound_t<T, N> & b) const {
            for (int i = 0; i < N; i++){
                if (!(*this)[i].intersects(b[i])){
                    return false;
                }
            }
            return true;
        }

        bool contains(const vec_t<T, N> & x) const {
            for (int i = 0; i < N; i++){
                if (!(*this)[i].contains(x[i])){    
                    return false;
                }
            }
            return true;
        }

        bool contains(const bound_t<T, N> & b) const {
            return contains(b.get_lower()) && contains(b.get_upper());
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

        bound_t<T, N> operator&(const bound_t<T, N> & b) const {
            return bound_t<T, N>(
                vec::max(get_lower(), b.get_lower()), vec::min(get_upper(), b.get_upper())
            );
        }        
        
        bound_t<T, N> operator|(const bound_t<T, N> & b) const {
            bound_t<T, N> x = *this;
            x |= b;
            return x;
        }

        void operator|=(const bound_t<T, N> & b){
            for (int i = 0; i < N; i++){
                (*this)[i] |= b[i];
            }
        }

        void enlarge(const T & x){
            *this += interval_t<T>(-x, x);
        }   

        bound_t<T, N> subdivision(int i) const {
            vec_t<T, N> lower = get_lower();
            vec_t<T, N> width = get_width();
           
            for (int j = 0; j < N; j++){
                if (i & (1 << j)){
                    lower[j] += width[j];
                }
            }

            return bound_t<T, N>(lower, lower + width);
        }
        
        vec_t<T, N> vertex(int i) const {
            vec_t<T, N> x = get_lower();
            vec_t<T, N> w = get_width() * 2;
           
            for (int j = 0; j < N; j++){
                if (i & (1 << j)){
                    x[j] += w[j];
                }
            }

            return x;
        }

        int subdivision_index(const vec_t<T, N> & x) const {
            vec_t<bool, N> subdivision = x >= get_midpoint();

            int index = 0;  
            for (int i = 0; i < N; i++){
                if (subdivision[i]){
                    index |= (1 << i);
                }
            }
            
            return index;
        }
    };

    using bound3_t = bound_t<double, 3>;
    using bound4_t = bound_t<double, 4>;
}

#endif
