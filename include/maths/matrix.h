#ifndef MATHS_VECTOR_H
#define MATHS_VECTOR_H

#include "core/constant.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <type_traits>

namespace srph {
    template<class T, int M, int N>
    class matrix_t : protected std::array<T, M * N> {
    private:
        using super_t = std::array<T, M * N>;

        template<int K, int P, typename... Xs>
        void construct(const matrix_t<T, P, 1> &x, Xs... xs) {
            static_assert(K + P <= M * N, "Too much data in matrix constructor");
            static_assert(K + P == M * N || sizeof...(Xs) > 0,
                          "Not enough data in matrix constructor");

            std::copy(x.begin(), x.end(), this->data() + K);

            if constexpr (sizeof...(Xs) != 0) {
                construct<K + P>(xs...);
            }
        }

        template<int K, typename... Xs>
        void construct(const T &x, Xs... xs) {
            construct<K>(matrix_t<T, 1, 1>(x), xs...);
        }

    public:
        // constructors
        matrix_t() : matrix_t(T(0)) {}

        matrix_t(const T &x) { this->fill(x); }

        template<class X, class... Xs>
        matrix_t(const X &x, Xs... xs) {
            construct<0>(x, xs...);
        }

        template<class S>
        matrix_t<decltype(T() + S()), M, N> operator+(const matrix_t<S, M, N> &x) const {
            matrix_t<decltype(T() + S()), M, N> r;
            std::transform(this->begin(), this->end(), x.begin(), r.begin(),
                           std::plus());
            return r;
        }

        template<class S>
        matrix_t<decltype(T() - S()), M, N> operator-(const matrix_t<S, M, N> &x) const {
            matrix_t<decltype(T() - S()), M, N> r;
            std::transform(this->begin(), this->end(), x.begin(), r.begin(),
                           std::minus());
            return r;
        }

        matrix_t<T, M, N> operator/(const matrix_t<T, M, N> &x) const {
            matrix_t<T, M, N> r;
            std::transform(this->begin(), this->end(), x.begin(), r.begin(),
                           std::divides<T>());
            return r;
        }

        matrix_t<T, M, N> operator+(const T &x) const {
            return *this + matrix_t<T, M, N>(x);
        }

        template<class S>
        matrix_t<decltype(T() * S()), M, N> operator*(const S &x) const {
            matrix_t<decltype(T() * S()), M, N> ms;
            for (int i = 0; i < M * N; i++) {
                ms[i] = (*this)[i] * x;
            }
            return ms;
        }

        T operator[](int i) const { return super_t::operator[](i); }

        T &operator[](int i) { return super_t::operator[](i); }

        typename std::array<T, M * N>::iterator begin() { return super_t::begin(); }

        typename std::array<T, M * N>::iterator end() { return super_t::end(); }

        typename std::array<T, M * N>::const_iterator begin() const {
            return super_t::begin();
        }

        typename std::array<T, M * N>::const_iterator end() const {
            return super_t::end();
        }
    };

    template<class T, int N> using vec_t = matrix_t<T, N, 1>;

//    typedef vec_t<uint32_t, 2> u32vec2_t;

    typedef vec_t<double, 3> vec3_t;
}
#endif
