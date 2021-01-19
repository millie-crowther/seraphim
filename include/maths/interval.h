#ifndef SERAPHIM_INTERVAL_H
#define SERAPHIM_INTERVAL_H

#include <limits>

namespace srph {
    template<class T>
    class interval_t {
    private:
        T min;
        T max;

    public:
        interval_t() : min(std::numeric_limits<T>::max()), max(std::numeric_limits<T>::min()){}

        interval_t(const T & _min, const T & _max) : min(_min), max(_max){}
       
        auto contains(const T & x) const {
            return x >= min && x < max;
        }

        auto is_valid() const {
            return contains(get_centre());
        }
 
        T get_size() const {
            return max / 2 - min / 2;
        }

        T get_max() const {
            return max;
        }

        T get_min() const {
            return min;
        }

        T get_centre() const {
            return max / 2 + min / 2;
        }
        
        interval_t<T> operator&&(const interval_t<T> & a) const {
            return interval_t<T>(std::max(min, a.min), std::min(max, a.max));
        }

        interval_t<T> operator||(const interval_t<T> & a) const {
            return interval_t<T>(std::min(min, a.min), std::max(max, a.max));
        }

        void enlarge(const T & t){
            *this += interval_t<T>(-t, t);
        }

        void capture(const T & t){
            min = std::min(min, t);
            max = std::max(max, t);
        }

        interval_t<T> operator*(const interval_t<T> & i) const {
            T x1y1 = min * i.min;
            T x1y2 = min * i.max;       
            T x2y1 = max * i.min;
            T x2y2 = max * i.max;

            return interval_t<T>(
                std::min(std::min(x1y1, x1y2), std::min(x2y1, x2y2)),       
                std::max(std::max(x1y1, x1y2), std::max(x2y1, x2y2))
            );       
        }

        interval_t<T> operator+(const interval_t<T> & i) const {
            return interval_t<T>(min + i.min, max + i.max);
        }

        void operator+=(const interval_t<T> & i){
            *this = *this + i;
        }
    };
}

#endif
