//
// Created by millie on 07/04/2021.
//

#ifndef SERAPHIM_MATHS_H
#define SERAPHIM_MATHS_H

/*
 * this file and the corresponding source file are modified from this repo: https://github.com/felselva/mathc
 * originally published under the license reproduced below
 *
Copyright © 2018 Felipe Ferreira da Silva

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use of
this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim
     that you wrote the original software. If you use this software in a
     product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <float.h>

#define VEC2_SIZE 2
#define VEC3_SIZE 3
#define VEC4_SIZE 4
#define QUAT_SIZE 4
#define MAT2_SIZE 4
#define MAT3_SIZE 9
#define MAT4_SIZE 16

typedef int32_t mint_t;
typedef double mfloat_t;

#define MINT_MAX INT32_MAX
#define MINT_MIN INT32_MIN

#define MPI 3.14159265358979323846
#define MPI_2 1.57079632679489661923
#define MPI_4 0.78539816339744830962
#define MFLT_EPSILON DBL_EPSILON
#define MFABS fabs
#define MFMIN fmin
#define MFMAX fmax
#define MSQRT sqrt
#define MSIN sin
#define MCOS cos
#define MACOS acos
#define MASIN asin
#define MTAN tan
#define MATAN2 atan2
#define MPOW pow
#define MFLOOR floor
#define MCEIL ceil
#define MROUND round
#define MFLOAT_C(c) c

typedef union vec2i {
    struct {
        mint_t x;
        mint_t y;
    };
    mint_t v[VEC2_SIZE];
} vec2i;

typedef union vec3i {
    struct {
        mint_t x;
        mint_t y;
        mint_t z;
    };
    mint_t v[VEC3_SIZE];
} vec3i;

typedef union vec4i {
    struct {
        mint_t x;
        mint_t y;
        mint_t z;
        mint_t w;
    };
    mint_t v[VEC4_SIZE];
} vec4i;

typedef union vec2 {
    struct {
        mfloat_t x;
        mfloat_t y;
    };
    mfloat_t v[VEC2_SIZE];
} vec2;

typedef union vec3 {
    struct {
        mfloat_t x;
        mfloat_t y;
        mfloat_t z;
    };
    mfloat_t v[VEC3_SIZE];
} vec3;

typedef union vec4 {
    struct {
        mfloat_t x;
        mfloat_t y;
        mfloat_t z;
        mfloat_t w;
    };
    mfloat_t v[VEC4_SIZE];
} vec4;

typedef union quat {
    struct {
        mfloat_t x;
        mfloat_t y;
        mfloat_t z;
        mfloat_t w;
    };
    mfloat_t v[QUAT_SIZE];
} quat;

/*
Matrix 2×2 representation:
0/m11 2/m12
1/m21 3/m22
*/
typedef union mat2 {
    struct {
        mfloat_t m11;
        mfloat_t m21;
        mfloat_t m12;
        mfloat_t m22;
    };
    mfloat_t v[MAT2_SIZE];
} mat2;

/*
Matrix 3×3 representation:
0/m11 3/m12 6/m13
1/m21 4/m22 7/m23
2/m31 5/m32 8/m33
*/
typedef union mat3 {
    struct {
        mfloat_t m11;
        mfloat_t m21;
        mfloat_t m31;
        mfloat_t m12;
        mfloat_t m22;
        mfloat_t m32;
        mfloat_t m13;
        mfloat_t m23;
        mfloat_t m33;
    };
    mfloat_t v[MAT3_SIZE];
} mat3;

/*
Matrix 4×4 representation:
0/m11 4/m12  8/m13 12/m14
1/m21 5/m22  9/m23 13/m24
2/m31 6/m32 10/m33 14/m34
3/m41 7/m42 11/m43 15/m44
*/
typedef union mat4 {
    struct {
        mfloat_t m11;
        mfloat_t m21;
        mfloat_t m31;
        mfloat_t m41;
        mfloat_t m12;
        mfloat_t m22;
        mfloat_t m32;
        mfloat_t m42;
        mfloat_t m13;
        mfloat_t m23;
        mfloat_t m33;
        mfloat_t m43;
        mfloat_t m14;
        mfloat_t m24;
        mfloat_t m34;
        mfloat_t m44;
    };
    mfloat_t v[MAT4_SIZE];
} mat4;

static const vec3 vec3_zero = {{ 0.0, 0.0, 0.0}};
static const vec3 vec3_right = {{ 1.0, 0.0, 0.0}};
static const vec3 vec3_up = {{ 0.0, 1.0, 0.0}};
static const vec3 vec3_forward = {{ 0.0, 0.0, 1.0}};

static const quat quat_identity = {{ 0.0, 0.0, 0.0, 1.0 }};

static const mat3 mat3_identity = {{
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0
}};

mint_t clampi(mint_t value, mint_t min, mint_t max);

#define MRADIANS(degrees) (degrees * MPI / MFLOAT_C(180.0))
#define MDEGREES(radians) (radians * MFLOAT_C(180.0) / MPI)
bool nearly_equal(mfloat_t a, mfloat_t b, mfloat_t epsilon);
mfloat_t to_radians(mfloat_t degrees);
mfloat_t to_degrees(mfloat_t radians);
mfloat_t clampf(mfloat_t value, mfloat_t min, mfloat_t max);

bool vec2i_is_zero(vec2i *v0);
bool vec2i_is_equal(vec2i *v0, vec2i *v1);
vec2i *vec2i_assign(vec2i *result, vec2i *v0);
vec2i *vec2i_assign_vec2(vec2i *result, vec2 *v0);
vec2i *vec2i_zero(vec2i *result);
vec2i *vec2i_one(vec2i *result);
vec2i *vec2i_sign(vec2i *result, vec2i *v0);
vec2i *vec2i_add(vec2i *result, vec2i *v0, vec2i *v1);
vec2i *vec2i_add_i(vec2i *result, vec2i *v0, mint_t i);
vec2i *vec2i_subtract(vec2i *result, vec2i *v0, vec2i *v1);
vec2i *vec2i_subtract_i(vec2i *result, vec2i *v0, mint_t i);
vec2i *vec2i_multiply(vec2i *result, vec2i *v0, vec2i *v1);
vec2i *vec2i_multiply_i(vec2i *result, vec2i *v0, mint_t i);
vec2i *vec2i_divide(vec2i *result, vec2i *v0, vec2i *v1);
vec2i *vec2i_divide_i(vec2i *result, vec2i *v0, mint_t i);
vec2i *vec2i_snap(vec2i *result, vec2i *v0, vec2i *v1);
vec2i *vec2i_snap_i(vec2i *result, vec2i *v0, mint_t i);
vec2i *vec2i_negative(vec2i *result, vec2i *v0);
vec2i *vec2i_abs(vec2i *result, vec2i *v0);
vec2i *vec2i_max(vec2i *result, vec2i *v0, vec2i *v1);
vec2i *vec2i_min(vec2i *result, vec2i *v0, vec2i *v1);
vec2i *vec2i_clamp(vec2i *result, vec2i *v0, vec2i *v1, vec2i *v2);
vec2i *vec2i_tangent(vec2i *result, vec2i *v0);
bool vec3i_is_zero(vec3i *v0);
bool vec3i_is_equal(vec3i *v0, vec3i *v1);
vec3i *vec3i_assign(vec3i *result, vec3i *v0);
vec3i *vec3i_assign_vec3(vec3i *result, vec3 *v0);
vec3i *vec3i_zero(vec3i *result);
vec3i *vec3i_one(vec3i *result);
vec3i *vec3i_sign(vec3i *result, vec3i *v0);
vec3i *vec3i_add(vec3i *result, vec3i *v0, vec3i *v1);
vec3i *vec3i_add_i(vec3i *result, vec3i *v0, mint_t i);
vec3i *vec3i_subtract(vec3i *result, vec3i *v0, vec3i *v1);
vec3i *vec3i_subtract_i(vec3i *result, vec3i *v0, mint_t i);
vec3i *vec3i_multiply(vec3i *result, vec3i *v0, vec3i *v1);
vec3i *vec3i_multiply_i(vec3i *result, vec3i *v0, mint_t i);
vec3i *vec3i_divide(vec3i *result, vec3i *v0, vec3i *v1);
vec3i *vec3i_divide_i(vec3i *result, vec3i *v0, mint_t i);
vec3i *vec3i_snap(vec3i *result, vec3i *v0, vec3i *v1);
vec3i *vec3i_snap_i(vec3i *result, vec3i *v0, mint_t i);
vec3i *vec3i_cross(vec3i *result, vec3i *v0, vec3i *v1);
vec3i *vec3i_negative(vec3i *result, vec3i *v0);
vec3i *vec3i_abs(vec3i *result, vec3i *v0);
vec3i *vec3i_max(vec3i *result, vec3i *v0, vec3i *v1);
vec3i *vec3i_min(vec3i *result, vec3i *v0, vec3i *v1);
vec3i *vec3i_clamp(vec3i *result, vec3i *v0, vec3i *v1, vec3i *v2);
bool vec4i_is_zero(vec4i *v0);
bool vec4i_is_equal(vec4i *v0, vec4i *v1);
vec4i *vec4i_assign(vec4i *result, vec4i *v0);
vec4i *vec4i_assign_vec4(vec4i *result, vec4 *v0);
vec4i *vec4i_zero(vec4i *result);
vec4i *vec4i_one(vec4i *result);
vec4i *vec4i_sign(vec4i *result, vec4i *v0);
vec4i *vec4i_add(vec4i *result, vec4i *v0, vec4i *v1);
vec4i *vec4i_add_i(vec4i *result, vec4i *v0, mint_t i);
vec4i *vec4i_subtract(vec4i *result, vec4i *v0, vec4i *v1);
vec4i *vec4i_subtract_i(vec4i *result, vec4i *v0, mint_t i);
vec4i *vec4i_multiply(vec4i *result, vec4i *v0, vec4i *v1);
vec4i *vec4i_multiply_i(vec4i *result, vec4i *v0, mint_t i);
vec4i *vec4i_divide(vec4i *result, vec4i *v0, vec4i *v1);
vec4i *vec4i_divide_i(vec4i *result, vec4i *v0, mint_t i);
vec4i *vec4i_snap(vec4i *result, vec4i *v0, vec4i *v1);
vec4i *vec4i_snap_i(vec4i *result, vec4i *v0, mint_t i);
vec4i *vec4i_negative(vec4i *result, vec4i *v0);
vec4i *vec4i_abs(vec4i *result, vec4i *v0);
vec4i *vec4i_max(vec4i *result, vec4i *v0, vec4i *v1);
vec4i *vec4i_min(vec4i *result, vec4i *v0, vec4i *v1);
vec4i *vec4i_clamp(vec4i *result, vec4i *v0, vec4i *v1, vec4i *v2);
bool vec2_is_zero(vec2 *v0);
bool vec2_is_equal(vec2 *v0, vec2 *v1);
vec2 *vec2_new(vec2 *result, mfloat_t x, mfloat_t y);
vec2 *vec2_assign(vec2 *result, vec2 *v0);
vec2 *vec2_assign_vec2i(vec2 *result, vec2i *v0);
vec2 *vec2_zero(vec2 *result);
vec2 *vec2_one(vec2 *result);
vec2 *vec2_sign(vec2 *result, vec2 *v0);
vec2 *vec2_add(vec2 *result, vec2 *v0, vec2 *v1);
vec2 *vec2_add_f(vec2 *result, vec2 *v0, mfloat_t f);
vec2 *vec2_subtract(vec2 *result, vec2 *v0, vec2 *v1);
vec2 *vec2_subtract_f(vec2 *result, vec2 *v0, mfloat_t f);
vec2 *vec2_multiply(vec2 *result, vec2 *v0, vec2 *v1);
vec2 *vec2_multiply_f(vec2 *result, vec2 *v0, mfloat_t f);
vec2 *vec2_multiply_mat2(vec2 *result, vec2 *v0, mat2 *m0);
vec2 *vec2_divide(vec2 *result, vec2 *v0, vec2 *v1);
vec2 *vec2_divide_f(vec2 *result, vec2 *v0, mfloat_t f);
vec2 *vec2_snap(vec2 *result, vec2 *v0, vec2 *v1);
vec2 *vec2_snap_f(vec2 *result, vec2 *v0, mfloat_t f);
vec2 *vec2_negative(vec2 *result, vec2 *v0);
vec2 *vec2_abs(vec2 *result, vec2 *v0);
vec2 *vec2_floor(vec2 *result, vec2 *v0);
vec2 *vec2_ceil(vec2 *result, vec2 *v0);
vec2 *vec2_round(vec2 *result, vec2 *v0);
vec2 *vec2_max(vec2 *result, vec2 *v0, vec2 *v1);
vec2 *vec2_min(vec2 *result, vec2 *v0, vec2 *v1);
vec2 *vec2_clamp(vec2 *result, vec2 *v0, vec2 *v1, vec2 *v2);
vec2 *vec2_normalize(vec2 *result, vec2 *v0);
mfloat_t vec2_dot(vec2 *v0, vec2 *v1);
vec2 *vec2_project(vec2 *result, vec2 *v0, vec2 *v1);
vec2 *vec2_slide(vec2 *result, vec2 *v0, vec2 *normal);
vec2 *vec2_reflect(vec2 *result, vec2 *v0, vec2 *normal);
vec2 *vec2_tangent(vec2 *result, vec2 *v0);
vec2 *vec2_rotate(vec2 *result, vec2 *v0, mfloat_t f);
vec2 *vec2_lerp(vec2 *result, vec2 *v0, vec2 *v1, mfloat_t f);
vec2 *vec2_bezier3(vec2 *result, vec2 *v0, vec2 *v1, vec2 *v2, mfloat_t f);
vec2 *vec2_bezier4(vec2 *result, vec2 *v0, vec2 *v1, vec2 *v2, vec2 *v3, mfloat_t f);
mfloat_t vec2_angle(vec2 *v0);
mfloat_t vec2_length(vec2 *v0);
mfloat_t vec2_length_squared(vec2 *v0);
mfloat_t vec2_distance(vec2 *v0, vec2 *v1);
mfloat_t vec2_distance_squared(vec2 *v0, vec2 *v1);
bool vec3_is_zero(vec3 *v0);
bool vec3_is_equal(vec3 *v0, vec3 *v1);
vec3 *vec3_assign(vec3 *result, vec3 *v0);
vec3 *vec3_assign_vec3i(vec3 *result, vec3i *v0);
vec3 *vec3_one(vec3 *result);
vec3 *vec3_sign(vec3 *result, vec3 *v0);
vec3 *vec3_add(vec3 *result, const vec3 *v0, const vec3 *v1);
vec3 *vec3_add_f(vec3 *result, vec3 *v0, mfloat_t f);
vec3 *vec3_subtract(vec3 *result, const vec3 *v0, const vec3 *v1);
vec3 *vec3_subtract_f(vec3 *result, vec3 *v0, mfloat_t f);
vec3 *vec3_multiply(vec3 *result, vec3 *v0, vec3 *v1);
vec3 *vec3_multiply_f(vec3 *result, const vec3 *v0, mfloat_t f);
vec3 *vec3_multiply_mat3(vec3 *result, const vec3 *v0, const mat3 *m0);
vec3 *vec3_multiply_quat(vec3 *result, const vec3 *v0, const quat *q);
vec3 *vec3_divide(vec3 *result, vec3 *v0, vec3 *v1);
vec3 *vec3_divide_f(vec3 *result, vec3 *v0, mfloat_t f);
vec3 *vec3_snap(vec3 *result, vec3 *v0, vec3 *v1);
vec3 *vec3_snap_f(vec3 *result, vec3 *v0, mfloat_t f);
vec3 *vec3_negative(vec3 *result, vec3 *v0);
vec3 *vec3_abs(vec3 *result, vec3 *v0);
vec3 *vec3_floor(vec3 *result, vec3 *v0);
vec3 *vec3_ceil(vec3 *result, vec3 *v0);
vec3 *vec3_round(vec3 *result, vec3 *v0);
vec3 *vec3_max(vec3 *result, vec3 *v0, vec3 *v1);
vec3 *vec3_min(vec3 *result, vec3 *v0, vec3 *v1);
vec3 *vec3_clamp(vec3 *result, vec3 *v0, vec3 *v1, vec3 *v2);
vec3 *vec3_cross(vec3 *result, const vec3 *v0, const vec3 *v1);
vec3 *vec3_normalize(vec3 *result, const vec3 *v0);
mfloat_t vec3_dot(const vec3 *v0, const vec3 *v1);
vec3 *vec3_project(vec3 *result, vec3 *v0, vec3 *v1);
vec3 *vec3_slide(vec3 *result, vec3 *v0, vec3 *normal);
vec3 *vec3_reflect(vec3 *result, vec3 *v0, vec3 *normal);
vec3 *vec3_rotate(vec3 *result, vec3 *v0, vec3 *ra, mfloat_t f);
vec3 *vec3_lerp(vec3 *result, vec3 *v0, vec3 *v1, mfloat_t f);
vec3 *vec3_bezier3(vec3 *result, vec3 *v0, vec3 *v1, vec3 *v2, mfloat_t f);
vec3 *vec3_bezier4(vec3 *result, vec3 *v0, vec3 *v1, vec3 *v2, vec3 *v3, mfloat_t f);
mfloat_t vec3_length(const vec3 *v0);
mfloat_t vec3_length_squared(vec3 *v0);
mfloat_t vec3_distance(const vec3 *v0, const vec3 *v1);
mfloat_t vec3_distance_squared(vec3 *v0, vec3 *v1);
bool vec4_is_zero(vec4 *v0);
bool vec4_is_equal(vec4 *v0, vec4 *v1);
vec4 *vec4_new(vec4 *result, mfloat_t x, mfloat_t y, mfloat_t z, mfloat_t w);
vec4 *vec4_assign(vec4 *result, vec4 *v0);
vec4 *vec4_assign_vec4i(vec4 *result, vec4i *v0);
vec4 *vec4_zero(vec4 *result);
vec4 *vec4_one(vec4 *result);
vec4 *vec4_sign(vec4 *result, vec4 *v0);
vec4 *vec4_add(vec4 *result, vec4 *v0, vec4 *v1);
vec4 *vec4_add_f(vec4 *result, vec4 *v0, mfloat_t f);
vec4 *vec4_subtract(vec4 *result, vec4 *v0, vec4 *v1);
vec4 *vec4_subtract_f(vec4 *result, vec4 *v0, mfloat_t f);
vec4 *vec4_multiply(vec4 *result, vec4 *v0, vec4 *v1);
vec4 *vec4_multiply_f(vec4 *result, vec4 *v0, mfloat_t f);
vec4 *vec4_multiply_mat4(vec4 *result, vec4 *v0, mat4 *m0);
vec4 *vec4_divide(vec4 *result, vec4 *v0, vec4 *v1);
vec4 *vec4_divide_f(vec4 *result, vec4 *v0, mfloat_t f);
vec4 *vec4_snap(vec4 *result, vec4 *v0, vec4 *v1);
vec4 *vec4_snap_f(vec4 *result, vec4 *v0, mfloat_t f);
vec4 *vec4_negative(vec4 *result, vec4 *v0);
vec4 *vec4_abs(vec4 *result, vec4 *v0);
vec4 *vec4_floor(vec4 *result, vec4 *v0);
vec4 *vec4_ceil(vec4 *result, vec4 *v0);
vec4 *vec4_round(vec4 *result, vec4 *v0);
vec4 *vec4_max(vec4 *result, vec4 *v0, vec4 *v1);
vec4 *vec4_min(vec4 *result, vec4 *v0, vec4 *v1);
vec4 *vec4_clamp(vec4 *result, vec4 *v0, vec4 *v1, vec4 *v2);
vec4 *vec4_normalize(vec4 *result, vec4 *v0);
vec4 *vec4_lerp(vec4 *result, vec4 *v0, vec4 *v1, mfloat_t f);
bool quat_is_zero(quat *q0);
bool quat_is_equal(quat *q0, quat *q1);
quat *quat_new(quat *result, mfloat_t x, mfloat_t y, mfloat_t z, mfloat_t w);
quat *quat_assign(quat *result, quat *q0);
quat *quat_zero(quat *result);
quat *quat_null(quat *result);
quat *quat_multiply(quat *result, const quat *q0, const quat *q1);
quat *quat_multiply_f(quat *result, quat *q0, mfloat_t f);
quat *quat_divide(quat *result, quat *q0, quat *q1);
quat *quat_divide_f(quat *result, quat *q0, mfloat_t f);
quat *quat_negative(quat *result, quat *q0);
quat *quat_conjugate(quat *result, quat *q0);
quat *quat_inverse(quat *result, quat *q0);
quat *quat_normalize(quat *result, quat *q0);
mfloat_t quat_dot(quat *q0, quat *q1);
quat *quat_power(quat *result, quat *q0, mfloat_t exponent);
quat *quat_from_axis_angle(quat *result, const vec3 *v0, mfloat_t angle);
quat *quat_from_vec3(quat *result, const vec3 *v0, const vec3 *v1);
quat *quat_from_euler_angles(quat *result, const vec3 * e);
quat *quat_from_mat4(quat *result, mat4 *m0);
quat *quat_lerp(quat *result, quat *q0, quat *q1, mfloat_t f);
quat *quat_slerp(quat *result, quat *q0, quat *q1, mfloat_t f);
mfloat_t quat_length(quat *q0);
mfloat_t quat_length_squared(quat *q0);
mfloat_t quat_angle(quat *q0, quat *q1);
mat2 *mat2_new(mat2 *result, mfloat_t m11, mfloat_t m12, mfloat_t m21, mfloat_t m22);
mat2 *mat2_zero(mat2 *result);
mat2 *mat2_identity(mat2 *result);
mfloat_t mat2_determinant(mat2 *m0);
mat2 *mat2_assign(mat2 *result, mat2 *m0);
mat2 *mat2_negative(mat2 *result, mat2 *m0);
mat2 *mat2_transpose(mat2 *result, mat2 *m0);
mat2 *mat2_cofactor(mat2 *result, mat2 *m0);
mat2 *mat2_adjugate(mat2 *result, mat2 *m0);
mat2 *mat2_multiply(mat2 *result, mat2 *m0, mat2 *m1);
mat2 *mat2_multiply_f(mat2 *result, mat2 *m0, mfloat_t f);
mat2 *mat2_inverse(mat2 *result, mat2 *m0);
mat2 *mat2_scaling(mat2 *result, vec2 *v0);
mat2 *mat2_scale(mat2 *result, mat2 *m0, vec2 *v0);
mat2 *mat2_rotation_z(mat2 *result, mfloat_t f);
mat2 *mat2_lerp(mat2 *result, mat2 *m0, mat2 *m1, mfloat_t f);
mat3 *mat3_new(mat3 *result, mfloat_t m11, mfloat_t m12, mfloat_t m13, mfloat_t m21, mfloat_t m22, mfloat_t m23, mfloat_t m31, mfloat_t m32, mfloat_t m33);
mat3 *mat3_zero(mat3 *result);
mfloat_t mat3_determinant(mat3 *m0);
mat3 *mat3_assign(mat3 *result, mat3 *m0);
mat3 *mat3_negative(mat3 *result, mat3 *m0);
mat3 *mat3_transpose(mat3 *result, mat3 *m0);
mat3 *mat3_cofactor(mat3 *result, mat3 *m0);
mat3 *mat3_multiply(mat3 *result, mat3 *m0, mat3 *m1);
mat3 *mat3_multiply_f(mat3 *result, mat3 *m0, mfloat_t f);
mat3 *mat3_inverse(mat3 *result, const mat3 *m0);
mat3 *mat3_scaling(mat3 *result, vec3 *v0);
mat3 *mat3_scale(mat3 *result, mat3 *m0, vec3 *v0);
mat3 *mat3_rotation_x(mat3 *result, mfloat_t f);
mat3 *mat3_rotation_y(mat3 *result, mfloat_t f);
mat3 *mat3_rotation_z(mat3 *result, mfloat_t f);
mat3 *mat3_rotation_axis(mat3 *result, vec3 *v0, mfloat_t f);
mat3 *mat3_rotation_quat(mat3 *result, const quat *q0);
mat3 *mat3_lerp(mat3 *result, mat3 *m0, mat3 *m1, mfloat_t f);
mat4 *mat4_new(mat4 *result, mfloat_t m11, mfloat_t m12, mfloat_t m13, mfloat_t m14, mfloat_t m21, mfloat_t m22, mfloat_t m23, mfloat_t m24, mfloat_t m31, mfloat_t m32, mfloat_t m33, mfloat_t m34, mfloat_t m41, mfloat_t m42, mfloat_t m43, mfloat_t m44);
mat4 *mat4_zero(mat4 *result);
mat4 *mat4_identity(mat4 *result);
mfloat_t mat4_determinant(mat4 *m0);
mat4 *mat4_assign(mat4 *result, mat4 *m0);
mat4 *mat4_negative(mat4 *result, mat4 *m0);
mat4 *mat4_transpose(mat4 *result, mat4 *m0);
mat4 *mat4_cofactor(mat4 *result, mat4 *m0);
mat4 *mat4_rotation_x(mat4 *result, mfloat_t f);
mat4 *mat4_rotation_y(mat4 *result, mfloat_t f);
mat4 *mat4_rotation_z(mat4 *result, mfloat_t f);
mat4 *mat4_rotation_axis(mat4 *result, vec3 *v0, mfloat_t f);
mat4 *mat4_rotation_quat(mat4 *result, const quat *q0);
mat4 *mat4_translation(mat4 *result, const mat4 *m0, const vec3 *v0);
mat4 *mat4_translate(mat4 *result, mat4 *m0, vec3 *v0);
mat4 *mat4_scaling(mat4 *result, mat4 *m0, vec3 *v0);
mat4 *mat4_scale(mat4 *result, mat4 *m0, vec3 *v0);
mat4 *mat4_multiply(mat4 *result, mat4 *m0, mat4 *m1);
mat4 *mat4_multiply_f(mat4 *result, mat4 *m0, mfloat_t f);
mat4 *mat4_inverse(mat4 *result, mat4 *m0);
mat4 *mat4_lerp(mat4 *result, mat4 *m0, mat4 *m1, mfloat_t f);
mat4 *mat4_look_at(mat4 *result, vec3 *position, vec3 *target, vec3 *up);
mat4 *mat4_ortho(mat4 *result, mfloat_t l, mfloat_t r, mfloat_t b, mfloat_t t, mfloat_t n, mfloat_t f);
mat4 *mat4_perspective(mat4 *result, mfloat_t fov_y, mfloat_t aspect, mfloat_t n, mfloat_t f);
mat4 *mat4_perspective_fov(mat4 *result, mfloat_t fov, mfloat_t w, mfloat_t h, mfloat_t n, mfloat_t f);
mat4 *mat4_perspective_infinite(mat4 *result, mfloat_t fov_y, mfloat_t aspect, mfloat_t n);

mfloat_t quadratic_ease_out(mfloat_t f);
mfloat_t quadratic_ease_in(mfloat_t f);
mfloat_t quadratic_ease_in_out(mfloat_t f);
mfloat_t cubic_ease_out(mfloat_t f);
mfloat_t cubic_ease_in(mfloat_t f);
mfloat_t cubic_ease_in_out(mfloat_t f);
mfloat_t quartic_ease_out(mfloat_t f);
mfloat_t quartic_ease_in(mfloat_t f);
mfloat_t quartic_ease_in_out(mfloat_t f);
mfloat_t quintic_ease_out(mfloat_t f);
mfloat_t quintic_ease_in(mfloat_t f);
mfloat_t quintic_ease_in_out(mfloat_t f);
mfloat_t sine_ease_out(mfloat_t f);
mfloat_t sine_ease_in(mfloat_t f);
mfloat_t sine_ease_in_out(mfloat_t f);
mfloat_t circular_ease_out(mfloat_t f);
mfloat_t circular_ease_in(mfloat_t f);
mfloat_t circular_ease_in_out(mfloat_t f);
mfloat_t exponential_ease_out(mfloat_t f);
mfloat_t exponential_ease_in(mfloat_t f);
mfloat_t exponential_ease_in_out(mfloat_t f);
mfloat_t elastic_ease_out(mfloat_t f);
mfloat_t elastic_ease_in(mfloat_t f);
mfloat_t elastic_ease_in_out(mfloat_t f);
mfloat_t back_ease_out(mfloat_t f);
mfloat_t back_ease_in(mfloat_t f);
mfloat_t back_ease_in_out(mfloat_t f);
mfloat_t bounce_ease_out(mfloat_t f);
mfloat_t bounce_ease_in(mfloat_t f);
mfloat_t bounce_ease_in_out(mfloat_t f);

#endif //SERAPHIM_MATHS_H
