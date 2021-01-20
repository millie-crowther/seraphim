#ifndef SERAPHIM_INTERVAL_H
#define SERAPHIM_INTERVAL_H

#include <limits>

namespace srph {
    template<class T>
    class interval_t {
    private:
        T lower;
        T upper;

    public:
        interval_t() : lower(std::numeric_limits<T>::max()), upper(std::numeric_limits<T>::min()){}
        interval_t(const T & x) : interval_t(x, x){}
        interval_t(const T & l, const T & u) : lower(l), upper(u){}
       
        bool contains(const T & x) const {
            return x >= lower && x < upper;
        }

        T get_upper() const {
            return upper;
        }

        T get_lower() const {
            return lower;
        }

        void capture(const T & t){
            lower = std::min(lower, t);
            upper = std::max(upper, t);
        }

        interval_t<T> operator*(const interval_t<T> & i) const {
            T x1y1 = lower * i.lower;
            T x1y2 = lower * i.upper;       
            T x2y1 = upper * i.lower;
            T x2y2 = upper * i.upper;

            return interval_t<T>(
                std::min(std::min(x1y1, x1y2), std::min(x2y1, x2y2)),       
                std::max(std::max(x1y1, x1y2), std::max(x2y1, x2y2))
            );       
        }

        interval_t<T> operator+(const interval_t<T> & i) const {
            return interval_t<T>(lower + i.lower, upper + i.upper);
        }

        void operator+=(const interval_t<T> & i){
            *this = *this + i;
        }
    };
}

#endif
