//
// Created by millie on 12/04/2021.
//

/*
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

#include "maths.h"

#include <math.h>

#define MAP_UNARY_FUNCTION(f, fx, x, n)                                             \
    for (int loop_index = 0; loop_index < (n); loop_index++) {                      \
        (fx)[loop_index] = f((x)[loop_index]);                                      \
    }

#define MAP_BINARY_OP(op, ab, a, b, n)                                              \
    for (int loop_index = 0; loop_index < (n); loop_index++) {                      \
        (ab)[loop_index] = (a)[loop_index] op(b)[loop_index];                       \
    }

mint_t clampi(mint_t value, mint_t min, mint_t max) {
    if (value < min) {
        value = min;
    } else if (value > max) {
        value = max;
    }
    return value;
}

bool nearly_equal(mfloat_t a, mfloat_t b, mfloat_t epsilon) {
    bool result = false;
    if (a == b) {
        result = true;
    } else if (MFABS(a - b) <= epsilon) {
        result = true;
    }
    return result;
}

mfloat_t to_radians(mfloat_t degrees) { return MRADIANS(degrees); }

mfloat_t to_degrees(mfloat_t radians) { return MDEGREES(radians); }

mfloat_t clampf(mfloat_t value, mfloat_t min, mfloat_t max) {
    if (value < min) {
        value = min;
    } else if (value > max) {
        value = max;
    }
    return value;
}

bool _vec2i_is_zero(mint_t *v0) { return v0[0] == 0 && v0[1] == 0; }

bool _vec2i_is_equal(mint_t *v0, mint_t *v1) {
    return v0[0] == v1[0] && v0[1] == v1[1];
}

mint_t *_vec2i_assign(mint_t *result, mint_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    return result;
}

mint_t *_vec2i_assign_vec2(mint_t *result, mfloat_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    return result;
}

mint_t *_vec2i_sign(mint_t *result, mint_t *v0) {
    if (v0[0] > 0) {
        result[0] = 1;
    } else if (v0[0] < 0) {
        result[0] = -1;
    } else {
        result[0] = 0;
    }
    if (v0[1] > 0) {
        result[1] = 1;
    } else if (v0[1] < 0) {
        result[1] = -1;
    } else {
        result[1] = 0;
    }
    return result;
}

mint_t *_vec2i_subtract(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = v0[0] - v1[0];
    result[1] = v0[1] - v1[1];
    return result;
}

mint_t *_vec2i_subtract_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = v0[0] - i;
    result[1] = v0[1] - i;
    return result;
}

mint_t *_vec2i_multiply(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = v0[0] * v1[0];
    result[1] = v0[1] * v1[1];
    return result;
}

mint_t *_vec2i_divide(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = v0[0] / v1[0];
    result[1] = v0[1] / v1[1];
    return result;
}

mint_t *_vec2i_divide_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = v0[0] / i;
    result[1] = v0[1] / i;
    return result;
}

mint_t *_vec2i_snap(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = (v0[0] / v1[0]) * v1[0];
    result[1] = (v0[1] / v1[1]) * v1[1];
    return result;
}

mint_t *_vec2i_snap_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = (v0[0] / i) * i;
    result[1] = (v0[1] / i) * i;
    return result;
}

mint_t *_vec2i_abs(mint_t *result, mint_t *v0) {
    result[0] = v0[0];
    if (result[0] < 0) {
        result[0] = -result[0];
    }
    result[1] = v0[1];
    if (result[1] < 0) {
        result[1] = -result[1];
    }
    return result;
}

mint_t *_vec2i_max(mint_t *result, mint_t *v0, mint_t *v1) {
    if (v0[0] > v1[0]) {
        result[0] = v0[0];
    } else {
        result[0] = v1[0];
    }
    if (v0[1] > v1[1]) {
        result[1] = v0[1];
    } else {
        result[1] = v1[1];
    }
    return result;
}

mint_t *_vec2i_min(mint_t *result, mint_t *v0, mint_t *v1) {
    if (v0[0] < v1[0]) {
        result[0] = v0[0];
    } else {
        result[0] = v1[0];
    }
    if (v0[1] < v1[1]) {
        result[1] = v0[1];
    } else {
        result[1] = v1[1];
    }
    return result;
}

mint_t *_vec2i_clamp(mint_t *result, mint_t *v0, mint_t *v1, mint_t *v2) {
    _vec2i_min(result, v0, v1);
    _vec2i_max(result, v0, v2);
    return result;
}

mint_t *_vec2i_tangent(mint_t *result, mint_t *v0) {
    mint_t a0 = v0[0];
    mint_t a1 = v0[1];
    result[0] = a1;
    result[1] = -a0;
    return result;
}

bool _vec3i_is_zero(mint_t *v0) { return v0[0] == 0 && v0[1] == 0 && v0[2] == 0; }

bool _vec3i_is_equal(mint_t *v0, mint_t *v1) {
    return v0[0] == v1[0] && v0[1] == v1[1] && v0[2] == v1[2];
}

mint_t *_vec3i_assign(mint_t *result, mint_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    result[2] = v0[2];
    return result;
}

mint_t *_vec3i_assign_vec3(mint_t *result, mfloat_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    result[2] = v0[2];
    return result;
}

mint_t *_vec3i_sign(mint_t *result, mint_t *v0) {
    if (v0[0] > 0) {
        result[0] = 1;
    } else if (v0[0] < 0) {
        result[0] = -1;
    } else {
        result[0] = 0;
    }
    if (v0[1] > 0) {
        result[1] = 1;
    } else if (v0[1] < 0) {
        result[1] = -1;
    } else {
        result[1] = 0;
    }
    if (v0[2] > 0) {
        result[2] = 1;
    } else if (v0[2] < 0) {
        result[2] = -1;
    } else {
        result[2] = 0;
    }
    return result;
}

mint_t *_vec3i_subtract(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = v0[0] - v1[0];
    result[1] = v0[1] - v1[1];
    result[2] = v0[2] - v1[2];
    return result;
}

mint_t *_vec3i_subtract_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = v0[0] - i;
    result[1] = v0[1] - i;
    result[2] = v0[2] - i;
    return result;
}

mint_t *_vec3i_multiply(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = v0[0] * v1[0];
    result[1] = v0[1] * v1[1];
    result[2] = v0[2] * v1[2];
    return result;
}

mint_t *_vec3i_divide(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = v0[0] / v1[0];
    result[1] = v0[1] / v1[1];
    result[2] = v0[2] / v1[2];
    return result;
}

mint_t *_vec3i_divide_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = v0[0] / i;
    result[1] = v0[1] / i;
    result[2] = v0[2] / i;
    return result;
}

mint_t *_vec3i_snap(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = (v0[0] / v1[0]) * v1[0];
    result[1] = (v0[1] / v1[1]) * v1[1];
    result[2] = (v0[2] / v1[2]) * v1[2];
    return result;
}

mint_t *_vec3i_snap_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = (v0[0] / i) * i;
    result[1] = (v0[1] / i) * i;
    result[2] = (v0[2] / i) * i;
    return result;
}

mint_t *_vec3i_cross(mint_t *result, mint_t *v0, mint_t *v1) {
    mint_t cross[VEC3_SIZE];
    cross[0] = v0[1] * v1[2] - v0[2] * v1[1];
    cross[1] = v0[2] * v1[0] - v0[0] * v1[2];
    cross[2] = v0[0] * v1[1] - v0[1] * v1[0];
    result[0] = cross[0];
    result[1] = cross[1];
    result[2] = cross[2];
    return result;
}

mint_t *_vec3i_abs(mint_t *result, mint_t *v0) {
    result[0] = v0[0];
    if (result[0] < 0) {
        result[0] = -result[0];
    }
    result[1] = v0[1];
    if (result[1] < 0) {
        result[1] = -result[1];
    }
    result[2] = v0[2];
    if (result[2] < 0) {
        result[2] = -result[2];
    }
    return result;
}

mint_t *_vec3i_max(mint_t *result, mint_t *v0, mint_t *v1) {
    if (v0[0] > v1[0]) {
        result[0] = v0[0];
    } else {
        result[0] = v1[0];
    }
    if (v0[1] > v1[1]) {
        result[1] = v0[1];
    } else {
        result[1] = v1[1];
    }
    if (v0[2] > v1[2]) {
        result[2] = v0[2];
    } else {
        result[2] = v1[2];
    }
    return result;
}

mint_t *_vec3i_min(mint_t *result, mint_t *v0, mint_t *v1) {
    if (v0[0] < v1[0]) {
        result[0] = v0[0];
    } else {
        result[0] = v1[0];
    }
    if (v0[1] < v1[1]) {
        result[1] = v0[1];
    } else {
        result[1] = v1[1];
    }
    if (v0[2] < v1[2]) {
        result[2] = v0[2];
    } else {
        result[2] = v1[2];
    }
    return result;
}

mint_t *_vec3i_clamp(mint_t *result, mint_t *v0, mint_t *v1, mint_t *v2) {
    _vec3i_min(result, v0, v1);
    _vec3i_max(result, v0, v2);
    return result;
}

bool _vec4i_is_zero(mint_t *v0) {
    return v0[0] == 0 && v0[1] == 0 && v0[2] == 0 && v0[3] == 0;
}

bool _vec4i_is_equal(mint_t *v0, mint_t *v1) {
    return v0[0] == v1[0] && v0[1] == v1[1] && v0[2] == v1[2] && v0[3] == v1[3];
}

mint_t *_vec4i_assign(mint_t *result, mint_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    result[2] = v0[2];
    result[3] = v0[3];
    return result;
}

mint_t *_vec4i_assign_vec4(mint_t *result, mfloat_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    result[2] = v0[2];
    result[3] = v0[3];
    return result;
}

mint_t *_vec4i_sign(mint_t *result, mint_t *v0) {
    if (v0[0] > 0) {
        result[0] = 1;
    } else if (v0[0] < 0) {
        result[0] = -1;
    } else {
        result[0] = 0;
    }
    if (v0[1] > 0) {
        result[1] = 1;
    } else if (v0[1] < 0) {
        result[1] = -1;
    } else {
        result[1] = 0;
    }
    if (v0[2] > 0) {
        result[2] = 1;
    } else if (v0[2] < 0) {
        result[2] = -1;
    } else {
        result[2] = 0;
    }
    if (v0[3] > 0) {
        result[3] = 1;
    } else if (v0[3] < 0) {
        result[3] = -1;
    } else {
        result[3] = 0;
    }
    return result;
}

mint_t *_vec4i_add(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = v0[0] + v1[0];
    result[1] = v0[1] + v1[1];
    result[2] = v0[2] + v1[2];
    result[3] = v0[3] + v1[3];
    return result;
}

mint_t *_vec4i_add_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = v0[0] + i;
    result[1] = v0[1] + i;
    result[2] = v0[2] + i;
    result[3] = v0[3] + i;
    return result;
}

mint_t *_vec4i_subtract(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = v0[0] - v1[0];
    result[1] = v0[1] - v1[1];
    result[2] = v0[2] - v1[2];
    result[3] = v0[3] - v1[3];
    return result;
}

mint_t *_vec4i_subtract_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = v0[0] - i;
    result[1] = v0[1] - i;
    result[2] = v0[2] - i;
    result[3] = v0[3] - i;
    return result;
}

mint_t *_vec4i_multiply(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = v0[0] * v1[0];
    result[1] = v0[1] * v1[1];
    result[2] = v0[2] * v1[2];
    result[3] = v0[3] * v1[3];
    return result;
}

mint_t *_vec4i_multiply_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = v0[0] * i;
    result[1] = v0[1] * i;
    result[2] = v0[2] * i;
    result[3] = v0[3] * i;
    return result;
}

mint_t *_vec4i_divide(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = v0[0] / v1[0];
    result[1] = v0[1] / v1[1];
    result[2] = v0[2] / v1[2];
    result[3] = v0[3] / v1[3];
    return result;
}

mint_t *_vec4i_divide_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = v0[0] / i;
    result[1] = v0[1] / i;
    result[2] = v0[2] / i;
    result[3] = v0[3] / i;
    return result;
}

mint_t *_vec4i_snap(mint_t *result, mint_t *v0, mint_t *v1) {
    result[0] = (v0[0] / v1[0]) * v1[0];
    result[1] = (v0[1] / v1[1]) * v1[1];
    result[2] = (v0[2] / v1[2]) * v1[2];
    result[3] = (v0[3] / v1[3]) * v1[3];
    return result;
}

mint_t *_vec4i_snap_i(mint_t *result, mint_t *v0, mint_t i) {
    result[0] = (v0[0] / i) * i;
    result[1] = (v0[1] / i) * i;
    result[2] = (v0[2] / i) * i;
    result[3] = (v0[3] / i) * i;
    return result;
}

mint_t *_vec4i_abs(mint_t *result, mint_t *v0) {
    result[0] = v0[0];
    if (result[0] < 0) {
        result[0] = -result[0];
    }
    result[1] = v0[1];
    if (result[1] < 0) {
        result[1] = -result[1];
    }
    result[2] = v0[2];
    if (result[2] < 0) {
        result[2] = -result[2];
    }
    result[3] = v0[3];
    if (result[3] < 0) {
        result[3] = -result[3];
    }
    return result;
}

mint_t *_vec4i_max(mint_t *result, mint_t *v0, mint_t *v1) {
    if (v0[0] > v1[0]) {
        result[0] = v0[0];
    } else {
        result[0] = v1[0];
    }
    if (v0[1] > v1[1]) {
        result[1] = v0[1];
    } else {
        result[1] = v1[1];
    }
    if (v0[2] > v1[2]) {
        result[2] = v0[2];
    } else {
        result[2] = v1[2];
    }
    if (v0[3] > v1[3]) {
        result[3] = v0[3];
    } else {
        result[3] = v1[3];
    }
    return result;
}

mint_t *_vec4i_min(mint_t *result, mint_t *v0, mint_t *v1) {
    if (v0[0] < v1[0]) {
        result[0] = v0[0];
    } else {
        result[0] = v1[0];
    }
    if (v0[1] < v1[1]) {
        result[1] = v0[1];
    } else {
        result[1] = v1[1];
    }
    if (v0[2] < v1[2]) {
        result[2] = v0[2];
    } else {
        result[2] = v1[2];
    }
    if (v0[3] < v1[3]) {
        result[3] = v0[3];
    } else {
        result[3] = v1[3];
    }
    return result;
}

mint_t *_vec4i_clamp(mint_t *result, mint_t *v0, mint_t *v1, mint_t *v2) {
    _vec4i_min(result, v0, v1);
    _vec4i_max(result, v0, v2);
    return result;
}

bool _vec2_is_zero(mfloat_t *v0) {
    return MFABS(v0[0]) < MFLT_EPSILON && MFABS(v0[1]) < MFLT_EPSILON;
}

bool _vec2_is_equal(mfloat_t *v0, mfloat_t *v1) {
    return MFABS(v0[0] - v1[0]) < MFLT_EPSILON &&
           MFABS(v0[1] - v1[1]) < MFLT_EPSILON;
}

mfloat_t *_vec2_assign(mfloat_t *result, mfloat_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    return result;
}

mfloat_t *_vec2_assign_vec2i(mfloat_t *result, const mint_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    return result;
}

mfloat_t *_vec2_sign(mfloat_t *result, const mfloat_t *v0) {
    if (v0[0] > MFLOAT_C(0.0)) {
        result[0] = MFLOAT_C(1.0);
    } else if (v0[0] < MFLOAT_C(0.0)) {
        result[0] = -MFLOAT_C(1.0);
    } else {
        result[0] = MFLOAT_C(0.0);
    }
    if (v0[1] > MFLOAT_C(0.0)) {
        result[1] = MFLOAT_C(1.0);
    } else if (v0[1] < MFLOAT_C(0.0)) {
        result[1] = -MFLOAT_C(1.0);
    } else {
        result[1] = MFLOAT_C(0.0);
    }
    return result;
}

mfloat_t *_vec2_add(mfloat_t *result, const mfloat_t *v0, mfloat_t *v1) {
    result[0] = v0[0] + v1[0];
    result[1] = v0[1] + v1[1];
    return result;
}

mfloat_t *_vec2_add_f(mfloat_t *result, const mfloat_t *v0, mfloat_t f) {
    result[0] = v0[0] + f;
    result[1] = v0[1] + f;
    return result;
}

mfloat_t *_vec2_subtract(mfloat_t *result, const mfloat_t *v0, mfloat_t *v1) {
    result[0] = v0[0] - v1[0];
    result[1] = v0[1] - v1[1];
    return result;
}

mfloat_t *_vec2_subtract_f(mfloat_t *result, const mfloat_t *v0, mfloat_t f) {
    result[0] = v0[0] - f;
    result[1] = v0[1] - f;
    return result;
}

mfloat_t *_vec2_multiply(mfloat_t *result, mfloat_t *v0, const mfloat_t *v1) {
    result[0] = v0[0] * v1[0];
    result[1] = v0[1] * v1[1];
    return result;
}

mfloat_t *_vec2_multiply_f(mfloat_t *result, const mfloat_t *v0, mfloat_t f) {
    result[0] = v0[0] * f;
    result[1] = v0[1] * f;
    return result;
}

mfloat_t *_vec2_multiply_mat2(mfloat_t *result, const mfloat_t *v0, mfloat_t *m0) {
    mfloat_t x = v0[0];
    mfloat_t y = v0[1];
    result[0] = m0[0] * x + m0[2] * y;
    result[1] = m0[1] * x + m0[3] * y;
    return result;
}

mfloat_t *_vec2_divide(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = v0[0] / v1[0];
    result[1] = v0[1] / v1[1];
    return result;
}

mfloat_t *_vec2_divide_f(mfloat_t *result, mfloat_t *v0, mfloat_t f) {
    result[0] = v0[0] / f;
    result[1] = v0[1] / f;
    return result;
}

mfloat_t *_vec2_snap(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = MFLOOR(v0[0] / v1[0]) * v1[0];
    result[1] = MFLOOR(v0[1] / v1[1]) * v1[1];
    return result;
}

mfloat_t *_vec2_snap_f(mfloat_t *result, mfloat_t *v0, mfloat_t f) {
    result[0] = MFLOOR(v0[0] / f) * f;
    result[1] = MFLOOR(v0[1] / f) * f;
    return result;
}

mfloat_t *_vec2_max(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = MFMAX(v0[0], v1[0]);
    result[1] = MFMAX(v0[1], v1[1]);
    return result;
}

mfloat_t *_vec2_min(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = MFMIN(v0[0], v1[0]);
    result[1] = MFMIN(v0[1], v1[1]);
    return result;
}

mfloat_t *_vec2_clamp(mfloat_t *result, mfloat_t *v0, mfloat_t *v1, mfloat_t *v2) {
    _vec2_min(result, v0, v1);
    _vec2_max(result, v0, v2);
    return result;
}

mfloat_t *_vec2_normalize(mfloat_t *result, mfloat_t *v0) {
    mfloat_t l = MSQRT(v0[0] * v0[0] + v0[1] * v0[1]);
    result[0] = v0[0] / l;
    result[1] = v0[1] / l;
    return result;
}

mfloat_t _vec2_dot(mfloat_t *v0, mfloat_t *v1) {
    return v0[0] * v1[0] + v0[1] * v1[1];
}

mfloat_t *_vec2_project(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    mfloat_t d = _vec2_dot(v1, v1);
    mfloat_t s = _vec2_dot(v0, v1) / d;
    result[0] = v1[0] * s;
    result[1] = v1[1] * s;
    return result;
}

mfloat_t *_vec2_slide(mfloat_t *result, mfloat_t *v0, mfloat_t *normal) {
    mfloat_t d = _vec2_dot(v0, normal);
    result[0] = v0[0] - normal[0] * d;
    result[1] = v0[1] - normal[1] * d;
    return result;
}

mfloat_t *_vec2_reflect(mfloat_t *result, mfloat_t *v0, mfloat_t *normal) {
    mfloat_t d = MFLOAT_C(2.0) * _vec2_dot(v0, normal);
    result[0] = normal[0] * d - v0[0];
    result[1] = normal[1] * d - v0[1];
    return result;
}

mfloat_t *_vec2_tangent(mfloat_t *result, const mfloat_t *v0) {
    mfloat_t a0 = v0[0];
    mfloat_t a1 = v0[1];
    result[0] = a1;
    result[1] = -a0;
    return result;
}

mfloat_t *_vec2_rotate(mfloat_t *result, const mfloat_t *v0, mfloat_t f) {
    mfloat_t cs = MCOS(f);
    mfloat_t sn = MSIN(f);
    mfloat_t x = v0[0];
    mfloat_t y = v0[1];
    result[0] = x * cs - y * sn;
    result[1] = x * sn + y * cs;
    return result;
}

mfloat_t *_vec2_lerp(mfloat_t *result, mfloat_t *v0, mfloat_t *v1, mfloat_t f) {
    result[0] = v0[0] + (v1[0] - v0[0]) * f;
    result[1] = v0[1] + (v1[1] - v0[1]) * f;
    return result;
}

mfloat_t *_vec2_bezier3(mfloat_t *result, mfloat_t *v0, mfloat_t *v1, mfloat_t *v2,
                        mfloat_t f) {
    mfloat_t tmp0[VEC2_SIZE];
    mfloat_t tmp1[VEC2_SIZE];
    _vec2_lerp(tmp0, v0, v1, f);
    _vec2_lerp(tmp1, v1, v2, f);
    _vec2_lerp(result, tmp0, tmp1, f);
    return result;
}

mfloat_t *_vec2_bezier4(mfloat_t *result, mfloat_t *v0, mfloat_t *v1, mfloat_t *v2,
                        mfloat_t *v3, mfloat_t f) {
    mfloat_t tmp0[VEC2_SIZE];
    mfloat_t tmp1[VEC2_SIZE];
    mfloat_t tmp2[VEC2_SIZE];
    mfloat_t tmp3[VEC2_SIZE];
    mfloat_t tmp4[VEC2_SIZE];
    _vec2_lerp(tmp0, v0, v1, f);
    _vec2_lerp(tmp1, v1, v2, f);
    _vec2_lerp(tmp2, v2, v3, f);
    _vec2_lerp(tmp3, tmp0, tmp1, f);
    _vec2_lerp(tmp4, tmp1, tmp2, f);
    _vec2_lerp(result, tmp3, tmp4, f);
    return result;
}

mfloat_t _vec2_angle(mfloat_t *v0) { return MATAN2(v0[1], v0[0]); }

mfloat_t _vec2_distance_squared(mfloat_t *v0, mfloat_t *v1) {
    return (v0[0] - v1[0]) * (v0[0] - v1[0]) + (v0[1] - v1[1]) * (v0[1] - v1[1]);
}

bool _vec2_linear_independent(const mfloat_t *v0, const mfloat_t *v1) {
    return (v0[0] * v1[1] - v1[0] * v0[1]) != 0;
}

mfloat_t **_vec2_orthonormalization(mfloat_t result[2][2], mfloat_t basis[2][2]) {
    mfloat_t v0[2];
    mfloat_t v1[2];

    for (int32_t i = 0; i < 2; ++i) {
        v0[i] = basis[0][i];
        v1[i] = basis[1][i];
    }

    if (!_vec2_linear_independent(v0, v1)) {
        return (mfloat_t **)result;
    }

    mfloat_t proju1[2];
    mfloat_t u0[2];
    mfloat_t u1[2];

    for (int32_t i = 0; i < 2; ++i) {
        u0[i] = v0[i];
    }

    _vec2_project(proju1, v1, v0);
    _vec2_subtract(u1, v1, proju1);
    _vec2_normalize(result[0], u0);
    _vec2_normalize(result[1], u1);

    return (mfloat_t **)result;
}

bool _vec3_is_zero(mfloat_t *v0) {
    return MFABS(v0[0]) < MFLT_EPSILON && MFABS(v0[1]) < MFLT_EPSILON &&
           MFABS(v0[2]) < MFLT_EPSILON;
}

bool _vec3_is_equal(mfloat_t *v0, mfloat_t *v1) {
    return MFABS(v0[0] - v1[0]) < MFLT_EPSILON &&
           MFABS(v0[1] - v1[1]) < MFLT_EPSILON &&
           MFABS(v0[2] - v1[2]) < MFLT_EPSILON;
}

mfloat_t *_vec3_assign(mfloat_t *result, const mfloat_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    result[2] = v0[2];
    return result;
}

mfloat_t *_vec3_assign_vec3i(mfloat_t *result, const mint_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    result[2] = v0[2];
    return result;
}

mfloat_t *_vec3_sign(mfloat_t *result, const mfloat_t *v0) {
    if (v0[0] > MFLOAT_C(0.0)) {
        result[0] = MFLOAT_C(1.0);
    } else if (v0[0] < 0) {
        result[0] = -MFLOAT_C(1.0);
    } else {
        result[0] = MFLOAT_C(0.0);
    }
    if (v0[1] > MFLOAT_C(0.0)) {
        result[1] = MFLOAT_C(1.0);
    } else if (v0[1] < 0) {
        result[1] = -MFLOAT_C(1.0);
    } else {
        result[1] = MFLOAT_C(0.0);
    }
    if (v0[2] > MFLOAT_C(0.0)) {
        result[2] = MFLOAT_C(1.0);
    } else if (v0[2] < 0) {
        result[2] = -MFLOAT_C(1.0);
    } else {
        result[2] = MFLOAT_C(0.0);
    }
    return result;
}

mfloat_t *_vec3_add(mfloat_t *result, const mfloat_t *v0, const mfloat_t *v1) {
    result[0] = v0[0] + v1[0];
    result[1] = v0[1] + v1[1];
    result[2] = v0[2] + v1[2];
    return result;
}

mfloat_t *_vec3_subtract(mfloat_t *result, const mfloat_t *v0, const mfloat_t *v1) {
    result[0] = v0[0] - v1[0];
    result[1] = v0[1] - v1[1];
    result[2] = v0[2] - v1[2];
    return result;
}

mfloat_t *_vec3_subtract_f(mfloat_t *result, const mfloat_t *v0, mfloat_t f) {
    result[0] = v0[0] - f;
    result[1] = v0[1] - f;
    result[2] = v0[2] - f;
    return result;
}

mfloat_t *_vec3_multiply(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = v0[0] * v1[0];
    result[1] = v0[1] * v1[1];
    result[2] = v0[2] * v1[2];
    return result;
}

mfloat_t *_vec3_multiply_mat3(mfloat_t *result, const mfloat_t *v0,
                              const mfloat_t *m0) {
    mfloat_t x = v0[0];
    mfloat_t y = v0[1];
    mfloat_t z = v0[2];
    result[0] = m0[0] * x + m0[3] * y + m0[6] * z;
    result[1] = m0[1] * x + m0[4] * y + m0[7] * z;
    result[2] = m0[2] * x + m0[5] * y + m0[8] * z;
    return result;
}

mfloat_t *_vec3_divide(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = v0[0] / v1[0];
    result[1] = v0[1] / v1[1];
    result[2] = v0[2] / v1[2];
    return result;
}

mfloat_t *_vec3_divide_f(mfloat_t *result, const mfloat_t *v0, mfloat_t f) {
    result[0] = v0[0] / f;
    result[1] = v0[1] / f;
    result[2] = v0[2] / f;
    return result;
}

mfloat_t *_vec3_snap(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = MFLOOR(v0[0] / v1[0]) * v1[0];
    result[1] = MFLOOR(v0[1] / v1[1]) * v1[1];
    result[2] = MFLOOR(v0[2] / v1[2]) * v1[2];
    return result;
}

mfloat_t *_vec3_snap_f(mfloat_t *result, const mfloat_t *v0, mfloat_t f) {
    result[0] = MFLOOR(v0[0] / f) * f;
    result[1] = MFLOOR(v0[1] / f) * f;
    result[2] = MFLOOR(v0[2] / f) * f;
    return result;
}

mfloat_t *_vec3_max(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = MFMAX(v0[0], v1[0]);
    result[1] = MFMAX(v0[1], v1[1]);
    result[2] = MFMAX(v0[2], v1[2]);
    return result;
}

mfloat_t *_vec3_min(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = MFMIN(v0[0], v1[0]);
    result[1] = MFMIN(v0[1], v1[1]);
    result[2] = MFMIN(v0[2], v1[2]);
    return result;
}

mfloat_t *_vec3_clamp(mfloat_t *result, mfloat_t *v0, mfloat_t *v1, mfloat_t *v2) {
    _vec3_min(result, v0, v1);
    _vec3_max(result, v0, v2);
    return result;
}

mfloat_t *_vec3_cross(mfloat_t *result, const mfloat_t *v0, const mfloat_t *v1) {
    mfloat_t cross[VEC3_SIZE];
    cross[0] = v0[1] * v1[2] - v0[2] * v1[1];
    cross[1] = v0[2] * v1[0] - v0[0] * v1[2];
    cross[2] = v0[0] * v1[1] - v0[1] * v1[0];
    result[0] = cross[0];
    result[1] = cross[1];
    result[2] = cross[2];
    return result;
}

mfloat_t *_vec3_normalize(mfloat_t *result, const mfloat_t *v0) {
    mfloat_t l = MSQRT(v0[0] * v0[0] + v0[1] * v0[1] + v0[2] * v0[2]);
    result[0] = v0[0] / l;
    result[1] = v0[1] / l;
    result[2] = v0[2] / l;
    return result;
}

mfloat_t _vec3_dot(const mfloat_t *v0, const mfloat_t *v1) {
    return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}

mfloat_t *_vec3_project(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    mfloat_t d = _vec3_dot(v1, v1);
    mfloat_t s = _vec3_dot(v0, v1) / d;
    result[0] = v1[0] * s;
    result[1] = v1[1] * s;
    result[2] = v1[2] * s;
    return result;
}

mfloat_t *_vec3_slide(mfloat_t *result, mfloat_t *v0, mfloat_t *normal) {
    mfloat_t d = _vec3_dot(v0, normal);
    result[0] = v0[0] - normal[0] * d;
    result[1] = v0[1] - normal[1] * d;
    result[2] = v0[2] - normal[2] * d;
    return result;
}

mfloat_t *_vec3_reflect(mfloat_t *result, mfloat_t *v0, mfloat_t *normal) {
    mfloat_t d = MFLOAT_C(2.0) * _vec3_dot(v0, normal);
    result[0] = normal[0] * d - v0[0];
    result[1] = normal[1] * d - v0[1];
    result[2] = normal[2] * d - v0[2];
    return result;
}

mfloat_t *_vec3_rotate(mfloat_t *result, mfloat_t *v0, mfloat_t *ra, mfloat_t f) {
    mfloat_t cs;
    mfloat_t sn;
    mfloat_t x;
    mfloat_t y;
    mfloat_t z;
    mfloat_t rx;
    mfloat_t ry;
    mfloat_t rz;
    cs = MCOS(f);
    sn = MSIN(f);
    x = v0[0];
    y = v0[1];
    z = v0[2];
    _vec3_normalize(ra, ra);
    rx = ra[0];
    ry = ra[1];
    rz = ra[2];
    result[0] = x * (cs + rx * rx * (1 - cs)) + y * (rx * ry * (1 - cs) - rz * sn) +
                z * (rx * rz * (1 - cs) + ry * sn);
    result[1] = x * (ry * rx * (1 - cs) + rz * sn) + y * (cs + ry * ry * (1 - cs)) +
                z * (ry * rz * (1 - cs) - rx * sn);
    result[2] = x * (rz * rx * (1 - cs) - ry * sn) +
                y * (rz * ry * (1 - cs) + rx * sn) + z * (cs + rz * rz * (1 - cs));
    return result;
}

mfloat_t *_vec3_lerp(mfloat_t *result, mfloat_t *v0, mfloat_t *v1, mfloat_t f) {
    result[0] = v0[0] + (v1[0] - v0[0]) * f;
    result[1] = v0[1] + (v1[1] - v0[1]) * f;
    result[2] = v0[2] + (v1[2] - v0[2]) * f;
    return result;
}

mfloat_t *_vec3_bezier3(mfloat_t *result, mfloat_t *v0, mfloat_t *v1, mfloat_t *v2,
                        mfloat_t f) {
    mfloat_t tmp0[VEC3_SIZE];
    mfloat_t tmp1[VEC3_SIZE];
    _vec3_lerp(tmp0, v0, v1, f);
    _vec3_lerp(tmp1, v1, v2, f);
    _vec3_lerp(result, tmp0, tmp1, f);
    return result;
}

mfloat_t *_vec3_bezier4(mfloat_t *result, mfloat_t *v0, mfloat_t *v1, mfloat_t *v2,
                        mfloat_t *v3, mfloat_t f) {
    mfloat_t tmp0[VEC3_SIZE];
    mfloat_t tmp1[VEC3_SIZE];
    mfloat_t tmp2[VEC3_SIZE];
    mfloat_t tmp3[VEC3_SIZE];
    mfloat_t tmp4[VEC3_SIZE];
    _vec3_lerp(tmp0, v0, v1, f);
    _vec3_lerp(tmp1, v1, v2, f);
    _vec3_lerp(tmp2, v2, v3, f);
    _vec3_lerp(tmp3, tmp0, tmp1, f);
    _vec3_lerp(tmp4, tmp1, tmp2, f);
    _vec3_lerp(result, tmp3, tmp4, f);
    return result;
}

mfloat_t _vec3_length_squared(const mfloat_t *v0) {
    return v0[0] * v0[0] + v0[1] * v0[1] + v0[2] * v0[2];
}

bool _vec3_linear_independent(mfloat_t *v0, mfloat_t *v1, mfloat_t *v2) {
    return v0[0] * v1[1] * v2[2] + v0[1] * v1[2] * v2[0] + v0[2] * v1[0] * v2[1] -
           v0[2] * v1[1] * v2[0] - v0[1] * v1[0] * v2[2] - v0[0] * v1[2] * v2[1];
}

mfloat_t **_vec3_orthonormalization(mfloat_t result[3][3], mfloat_t basis[3][3]) {
    mfloat_t v0[3];
    mfloat_t v1[3];
    mfloat_t v2[3];

    for (int32_t i = 0; i < 3; ++i) {
        v0[i] = basis[0][i];
        v1[i] = basis[1][i];
        v2[i] = basis[2][i];
    }

    if (!_vec3_linear_independent(v0, v1, v2)) {
        return (mfloat_t **)result;
    }

    mfloat_t proj[3];
    mfloat_t u0[3];
    mfloat_t u1[3];
    mfloat_t u2[3];

    for (int32_t i = 0; i < 3; ++i) {
        u0[i] = v0[i];
    }

    _vec3_project(proj, v1, u0);
    _vec3_subtract(u1, v1, proj);

    _vec3_project(proj, v2, u0);
    _vec3_subtract(u2, v2, proj);
    _vec3_project(proj, v2, u1);
    _vec3_subtract(u2, u2, proj);

    _vec3_normalize(result[0], u0);
    _vec3_normalize(result[1], u1);
    _vec3_normalize(result[2], u2);

    return (mfloat_t **)result;
}

bool _vec4_is_zero(mfloat_t *v0) {
    return MFABS(v0[0]) < MFLT_EPSILON && MFABS(v0[1]) < MFLT_EPSILON &&
           MFABS(v0[2]) < MFLT_EPSILON && MFABS(v0[3]) < MFLT_EPSILON;
}

bool _vec4_is_equal(mfloat_t *v0, mfloat_t *v1) {
    return MFABS(v0[0] - v1[0]) < MFLT_EPSILON &&
           MFABS(v0[1] - v1[1]) < MFLT_EPSILON &&
           MFABS(v0[2] - v1[2]) < MFLT_EPSILON &&
           MFABS(v0[3] - v1[3]) < MFLT_EPSILON;
}

mfloat_t *_vec4_assign(mfloat_t *result, mfloat_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    result[2] = v0[2];
    result[3] = v0[3];
    return result;
}

mfloat_t *_vec4_assign_vec4i(mfloat_t *result, mint_t *v0) {
    result[0] = v0[0];
    result[1] = v0[1];
    result[2] = v0[2];
    result[3] = v0[3];
    return result;
}

mfloat_t *_vec4_sign(mfloat_t *result, mfloat_t *v0) {
    if (v0[0] > MFLOAT_C(0.0)) {
        result[0] = MFLOAT_C(1.0);
    } else if (v0[0] < 0) {
        result[0] = -MFLOAT_C(1.0);
    } else {
        result[0] = MFLOAT_C(0.0);
    }
    if (v0[1] > MFLOAT_C(0.0)) {
        result[1] = MFLOAT_C(1.0);
    } else if (v0[1] < 0) {
        result[1] = -MFLOAT_C(1.0);
    } else {
        result[1] = MFLOAT_C(0.0);
    }
    if (v0[2] > MFLOAT_C(0.0)) {
        result[2] = MFLOAT_C(1.0);
    } else if (v0[2] < 0) {
        result[2] = -MFLOAT_C(1.0);
    } else {
        result[2] = MFLOAT_C(0.0);
    }
    if (v0[3] > MFLOAT_C(0.0)) {
        result[3] = MFLOAT_C(1.0);
    } else if (v0[3] < 0) {
        result[3] = -MFLOAT_C(1.0);
    } else {
        result[3] = MFLOAT_C(0.0);
    }
    return result;
}

mfloat_t *_vec4_add(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = v0[0] + v1[0];
    result[1] = v0[1] + v1[1];
    result[2] = v0[2] + v1[2];
    result[3] = v0[3] + v1[3];
    return result;
}

mfloat_t *_vec4_subtract(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = v0[0] - v1[0];
    result[1] = v0[1] - v1[1];
    result[2] = v0[2] - v1[2];
    result[3] = v0[3] - v1[3];
    return result;
}

mfloat_t *_vec4_subtract_f(mfloat_t *result, mfloat_t *v0, mfloat_t f) {
    result[0] = v0[0] - f;
    result[1] = v0[1] - f;
    result[2] = v0[2] - f;
    result[3] = v0[3] - f;
    return result;
}

mfloat_t *_vec4_multiply(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = v0[0] * v1[0];
    result[1] = v0[1] * v1[1];
    result[2] = v0[2] * v1[2];
    result[3] = v0[3] * v1[3];
    return result;
}

mfloat_t *_vec4_multiply_mat4(mfloat_t *result, mfloat_t *v0, mfloat_t *m0) {
    mfloat_t x = v0[0];
    mfloat_t y = v0[1];
    mfloat_t z = v0[2];
    mfloat_t w = v0[3];
    result[0] = m0[0] * x + m0[4] * y + m0[8] * z + m0[12] * w;
    result[1] = m0[1] * x + m0[5] * y + m0[9] * z + m0[13] * w;
    result[2] = m0[2] * x + m0[6] * y + m0[10] * z + m0[14] * w;
    result[3] = m0[3] * x + m0[7] * y + m0[11] * z + m0[15] * w;
    return result;
}

mfloat_t *_vec4_divide(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = v0[0] / v1[0];
    result[1] = v0[1] / v1[1];
    result[2] = v0[2] / v1[2];
    result[3] = v0[3] / v1[3];
    return result;
}

mfloat_t *_vec4_divide_f(mfloat_t *result, mfloat_t *v0, mfloat_t f) {
    result[0] = v0[0] / f;
    result[1] = v0[1] / f;
    result[2] = v0[2] / f;
    result[3] = v0[3] / f;
    return result;
}

mfloat_t *_vec4_snap(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = MFLOOR(v0[0] / v1[0]) * v1[0];
    result[1] = MFLOOR(v0[1] / v1[1]) * v1[1];
    result[2] = MFLOOR(v0[2] / v1[2]) * v1[2];
    result[3] = MFLOOR(v0[3] / v1[3]) * v1[3];
    return result;
}

mfloat_t *_vec4_snap_f(mfloat_t *result, mfloat_t *v0, mfloat_t f) {
    result[0] = MFLOOR(v0[0] / f) * f;
    result[1] = MFLOOR(v0[1] / f) * f;
    result[2] = MFLOOR(v0[2] / f) * f;
    result[3] = MFLOOR(v0[3] / f) * f;
    return result;
}

mfloat_t *_vec4_max(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = MFMAX(v0[0], v1[0]);
    result[1] = MFMAX(v0[1], v1[1]);
    result[2] = MFMAX(v0[2], v1[2]);
    result[3] = MFMAX(v0[3], v1[3]);
    return result;
}

mfloat_t *_vec4_min(mfloat_t *result, mfloat_t *v0, mfloat_t *v1) {
    result[0] = MFMIN(v0[0], v1[0]);
    result[1] = MFMIN(v0[1], v1[1]);
    result[2] = MFMIN(v0[2], v1[2]);
    result[3] = MFMIN(v0[3], v1[3]);
    return result;
}

mfloat_t *_vec4_clamp(mfloat_t *result, mfloat_t *v0, mfloat_t *v1, mfloat_t *v2) {
    _vec4_min(result, v0, v1);
    _vec4_max(result, v0, v2);
    return result;
}

mfloat_t *_vec4_normalize(mfloat_t *result, mfloat_t *v0) {
    mfloat_t l =
        MSQRT(v0[0] * v0[0] + v0[1] * v0[1] + v0[2] * v0[2] + v0[3] * v0[3]);
    result[0] = v0[0] / l;
    result[1] = v0[1] / l;
    result[2] = v0[2] / l;
    result[3] = v0[3] / l;
    return result;
}

mfloat_t *_vec4_lerp(mfloat_t *result, mfloat_t *v0, mfloat_t *v1, mfloat_t f) {
    result[0] = v0[0] + (v1[0] - v0[0]) * f;
    result[1] = v0[1] + (v1[1] - v0[1]) * f;
    result[2] = v0[2] + (v1[2] - v0[2]) * f;
    result[3] = v0[3] + (v1[3] - v0[3]) * f;
    return result;
}

bool _quat_is_zero(mfloat_t *q0) {
    return MFABS(q0[0]) < MFLT_EPSILON && MFABS(q0[1]) < MFLT_EPSILON &&
           MFABS(q0[2]) < MFLT_EPSILON && MFABS(q0[3]) < MFLT_EPSILON;
}

bool _quat_is_equal(mfloat_t *q0, mfloat_t *q1) {
    return MFABS(q0[0] - q1[0]) < MFLT_EPSILON &&
           MFABS(q0[1] - q1[1]) < MFLT_EPSILON &&
           MFABS(q0[2] - q1[2]) < MFLT_EPSILON &&
           MFABS(q0[3] - q1[3]) < MFLT_EPSILON;
}

mfloat_t *_quat_new(mfloat_t *result, mfloat_t x, mfloat_t y, mfloat_t z,
                    mfloat_t w) {
    result[0] = x;
    result[1] = y;
    result[2] = z;
    result[3] = w;
    return result;
}

mfloat_t *_quat_assign(mfloat_t *result, mfloat_t *q0) {
    result[0] = q0[0];
    result[1] = q0[1];
    result[2] = q0[2];
    result[3] = q0[3];
    return result;
}

mfloat_t *_quat_multiply_f(mfloat_t *result, mfloat_t *q0, mfloat_t f) {
    result[0] = q0[0] * f;
    result[1] = q0[1] * f;
    result[2] = q0[2] * f;
    result[3] = q0[3] * f;
    return result;
}

mfloat_t *_quat_divide(mfloat_t *result, mfloat_t *q0, mfloat_t *q1) {
    mfloat_t x = q0[0];
    mfloat_t y = q0[1];
    mfloat_t z = q0[2];
    mfloat_t w = q0[3];
    mfloat_t ls = q1[0] * q1[0] + q1[1] * q1[1] + q1[8] * q1[8] + q1[3] * q1[3];
    mfloat_t normalized_x = -q1[0] / ls;
    mfloat_t normalized_y = -q1[1] / ls;
    mfloat_t normalized_z = -q1[8] / ls;
    mfloat_t normalized_w = q1[3] / ls;
    result[0] =
        x * normalized_w + normalized_x * w + (y * normalized_z - z * normalized_y);
    result[1] =
        y * normalized_w + normalized_y * w + (z * normalized_x - x * normalized_z);
    result[2] =
        z * normalized_w + normalized_z * w + (x * normalized_y - y * normalized_x);
    result[3] =
        w * normalized_w - (x * normalized_x + y * normalized_y + z * normalized_z);
    return result;
}

mfloat_t *_quat_divide_f(mfloat_t *result, mfloat_t *q0, mfloat_t f) {
    result[0] = q0[0] / f;
    result[1] = q0[1] / f;
    result[2] = q0[2] / f;
    result[3] = q0[3] / f;
    return result;
}

mfloat_t *_quat_negative(mfloat_t *result, mfloat_t *q0) {
    result[0] = -q0[0];
    result[1] = -q0[1];
    result[2] = -q0[2];
    result[3] = -q0[3];
    return result;
}

mfloat_t *_quat_conjugate(mfloat_t *result, mfloat_t *q0) {
    result[0] = -q0[0];
    result[1] = -q0[1];
    result[2] = -q0[2];
    result[3] = q0[3];
    return result;
}

mfloat_t *_quat_inverse(mfloat_t *result, mfloat_t *q0) {
    mfloat_t l = MFLOAT_C(1.0) /
                 (q0[0] * q0[0] + q0[1] * q0[1] + q0[2] * q0[2] + q0[3] * q0[3]);
    result[0] = -q0[0] * l;
    result[1] = -q0[1] * l;
    result[2] = -q0[2] * l;
    result[3] = q0[3] * l;
    return result;
}

mfloat_t *_quat_normalize(mfloat_t *result, mfloat_t *q0) {
    mfloat_t l = MFLOAT_C(1.0) / MSQRT(q0[0] * q0[0] + q0[1] * q0[1] +
                                       q0[2] * q0[2] + q0[3] * q0[3]);
    result[0] = q0[0] * l;
    result[1] = q0[1] * l;
    result[2] = q0[2] * l;
    result[3] = q0[3] * l;
    return result;
}

mfloat_t _quat_dot(mfloat_t *q0, mfloat_t *q1) {
    return q0[0] * q1[0] + q0[1] * q1[1] + q0[2] * q1[2] + q0[3] * q1[3];
}

mfloat_t *_quat_power(mfloat_t *result, mfloat_t *q0, mfloat_t exponent) {
    if (MFABS(q0[3]) < MFLOAT_C(1.0) - MFLT_EPSILON) {
        mfloat_t alpha = MACOS(q0[3]);
        mfloat_t new_alpha = alpha * exponent;
        mfloat_t s = MSIN(new_alpha) / MSIN(alpha);
        result[0] = result[0] * s;
        result[1] = result[1] * s;
        result[2] = result[2] * s;
        result[3] = MCOS(new_alpha);
    } else {
        result[0] = q0[0];
        result[1] = q0[1];
        result[2] = q0[1];
        result[3] = q0[3];
    }
    return result;
}

mfloat_t *_quat_from_axis_angle(mfloat_t *result, const mfloat_t *v0,
                                mfloat_t angle) {
    mfloat_t half = angle * MFLOAT_C(0.5);
    mfloat_t s = MSIN(half);
    result[0] = v0[0] * s;
    result[1] = v0[1] * s;
    result[2] = v0[2] * s;
    result[3] = MCOS(half);
    return result;
}

mfloat_t *_quat_from_vec3(mfloat_t *result, const mfloat_t *v0, const mfloat_t *v1) {
    mfloat_t cross[VEC3_SIZE];
    mfloat_t d = _vec3_dot(v0, v1);
    mfloat_t a_ls = _vec3_length_squared(v0);
    mfloat_t b_ls = _vec3_length_squared(v0);
    _vec3_cross(cross, v0, v1);
    _quat_new(result, cross[0], cross[1], cross[1], d + MSQRT(a_ls * b_ls));
    _quat_normalize(result, result);
    return result;
}

mfloat_t *_quat_from_mat4(mfloat_t *result, mfloat_t *m0) {
    mfloat_t scale = m0[0] + m0[5] + m0[10];
    if (scale > MFLOAT_C(0.0)) {
        mfloat_t sr = MSQRT(scale + MFLOAT_C(1.0));
        result[3] = sr * MFLOAT_C(0.5);
        sr = MFLOAT_C(0.5) / sr;
        result[0] = (m0[9] - m0[6]) * sr;
        result[1] = (m0[2] - m0[8]) * sr;
        result[2] = (m0[4] - m0[1]) * sr;
    } else if ((m0[0] >= m0[5]) && (m0[0] >= m0[10])) {
        mfloat_t sr = MSQRT(MFLOAT_C(1.0) + m0[0] - m0[5] - m0[10]);
        mfloat_t half = MFLOAT_C(0.5) / sr;
        result[0] = MFLOAT_C(0.5) * sr;
        result[1] = (m0[4] + m0[1]) * half;
        result[2] = (m0[8] + m0[2]) * half;
        result[3] = (m0[9] - m0[6]) * half;
    } else if (m0[5] > m0[10]) {
        mfloat_t sr = MSQRT(MFLOAT_C(1.0) + m0[5] - m0[0] - m0[10]);
        mfloat_t half = MFLOAT_C(0.5) / sr;
        result[0] = (m0[1] + m0[4]) * half;
        result[1] = MFLOAT_C(0.5) * sr;
        result[2] = (m0[6] + m0[9]) * half;
        result[3] = (m0[2] - m0[8]) * half;
    } else {
        mfloat_t sr = MSQRT(MFLOAT_C(1.0) + m0[10] - m0[0] - m0[5]);
        mfloat_t half = MFLOAT_C(0.5) / sr;
        result[0] = (m0[2] + m0[8]) * half;
        result[1] = (m0[6] + m0[9]) * half;
        result[2] = MFLOAT_C(0.5) * sr;
        result[3] = (m0[4] - m0[1]) * half;
    }
    return result;
}

mfloat_t *_quat_lerp(mfloat_t *result, mfloat_t *q0, mfloat_t *q1, mfloat_t f) {
    result[0] = q0[0] + (q1[0] - q0[0]) * f;
    result[1] = q0[1] + (q1[1] - q0[1]) * f;
    result[2] = q0[2] + (q1[2] - q0[2]) * f;
    result[3] = q0[3] + (q1[3] - q0[3]) * f;
    return result;
}

mfloat_t *_quat_slerp(mfloat_t *result, mfloat_t *q0, mfloat_t *q1, mfloat_t f) {
    mfloat_t tmp1[QUAT_SIZE];
    mfloat_t d = _quat_dot(q0, q1);
    mfloat_t f0;
    mfloat_t f1;
    _quat_assign(tmp1, q1);
    if (d < MFLOAT_C(0.0)) {
        _quat_negative(tmp1, tmp1);
        d = -d;
    }
    if (d > MFLOAT_C(0.9995)) {
        f0 = MFLOAT_C(1.0) - f;
        f1 = f;
    } else {
        mfloat_t theta = MACOS(d);
        mfloat_t sin_theta = MSIN(theta);
        f0 = MSIN((MFLOAT_C(1.0) - f) * theta) / sin_theta;
        f1 = MSIN(f * theta) / sin_theta;
    }
    result[0] = q0[0] * f0 + tmp1[0] * f1;
    result[1] = q0[1] * f0 + tmp1[1] * f1;
    result[2] = q0[2] * f0 + tmp1[2] * f1;
    result[3] = q0[3] * f0 + tmp1[3] * f1;
    return result;
}

mfloat_t _quat_length(mfloat_t *q0) {
    return MSQRT(q0[0] * q0[0] + q0[1] * q0[1] + q0[2] * q0[2] + q0[3] * q0[3]);
}

mfloat_t _quat_length_squared(mfloat_t *q0) {
    return q0[0] * q0[0] + q0[1] * q0[1] + q0[2] * q0[2] + q0[3] * q0[3];
}

mfloat_t _quat_angle(mfloat_t *q0, mfloat_t *q1) {
    mfloat_t s = MSQRT(_quat_length_squared(q0) * _quat_length_squared(q1));
    s = MFLOAT_C(1.0) / s;
    return MACOS(_quat_dot(q0, q1) * s);
}

mfloat_t _mat2_determinant(mfloat_t *m0) { return m0[0] * m0[3] - m0[2] * m0[1]; }

mfloat_t *_mat2_assign(mfloat_t *result, mfloat_t *m0) {
    result[0] = m0[0];
    result[1] = m0[1];
    result[2] = m0[2];
    result[3] = m0[3];
    return result;
}

mfloat_t *_mat2_transpose(mfloat_t *result, mfloat_t *m0) {
    mfloat_t transposed[MAT2_SIZE];
    transposed[0] = m0[0];
    transposed[1] = m0[2];
    transposed[2] = m0[1];
    transposed[3] = m0[3];
    result[0] = transposed[0];
    result[1] = transposed[1];
    result[2] = transposed[2];
    result[3] = transposed[3];
    return result;
}

mfloat_t *_mat2_cofactor(mfloat_t *result, mfloat_t *m0) {
    mfloat_t cofactor[MAT2_SIZE];
    cofactor[0] = m0[3];
    cofactor[1] = -m0[2];
    cofactor[2] = -m0[1];
    cofactor[3] = m0[0];
    result[0] = cofactor[0];
    result[1] = cofactor[1];
    result[2] = cofactor[2];
    result[3] = cofactor[3];
    return result;
}

mfloat_t *_mat2_adjugate(mfloat_t *result, mfloat_t *m0) {
    mfloat_t adjugate[MAT2_SIZE];
    adjugate[0] = m0[3];
    adjugate[1] = -m0[1];
    adjugate[2] = -m0[2];
    adjugate[3] = m0[0];
    result[0] = adjugate[0];
    result[1] = adjugate[1];
    result[2] = adjugate[2];
    result[3] = adjugate[3];
    return result;
}

mfloat_t *_mat2_multiply(mfloat_t *result, mfloat_t *m0, mfloat_t *m1) {
    mfloat_t multiplied[MAT3_SIZE];
    multiplied[0] = m0[0] * m1[0] + m0[2] * m1[1];
    multiplied[1] = m0[1] * m1[0] + m0[3] * m1[1];
    multiplied[2] = m0[0] * m1[2] + m0[2] * m1[3];
    multiplied[3] = m0[1] * m1[2] + m0[3] * m1[3];
    result[0] = multiplied[0];
    result[1] = multiplied[1];
    result[2] = multiplied[2];
    result[3] = multiplied[3];
    return result;
}

mfloat_t *_mat2_multiply_f(mfloat_t *result, mfloat_t *m0, mfloat_t f) {
    result[0] = m0[0] * f;
    result[1] = m0[1] * f;
    result[2] = m0[2] * f;
    result[3] = m0[3] * f;
    return result;
}

mfloat_t *_mat2_inverse(mfloat_t *result, mfloat_t *m0) {
    mfloat_t inverse[MAT2_SIZE];
    mfloat_t det = _mat2_determinant(m0);
    _mat2_cofactor(inverse, m0);
    _mat2_multiply_f(inverse, inverse, MFLOAT_C(1.0) / det);
    result[0] = inverse[0];
    result[1] = inverse[1];
    result[2] = inverse[2];
    result[3] = inverse[3];
    return result;
}

mfloat_t *_mat2_scaling(mfloat_t *result, mfloat_t *v0) {
    result[0] = v0[0];
    result[3] = v0[1];
    return result;
}

mfloat_t *_mat2_scale(mfloat_t *result, mfloat_t *m0, mfloat_t *v0) {
    result[0] = m0[0] * v0[0];
    result[3] = m0[3] * v0[1];
    return result;
}

mfloat_t *_mat2_rotation_z(mfloat_t *result, mfloat_t f) {
    mfloat_t c = MCOS(f);
    mfloat_t s = MSIN(f);
    result[0] = c;
    result[1] = s;
    result[2] = -s;
    result[3] = c;
    return result;
}

mfloat_t *_mat2_lerp(mfloat_t *result, mfloat_t *m0, mfloat_t *m1, mfloat_t f) {
    result[0] = m0[0] + (m1[0] - m0[0]) * f;
    result[1] = m0[1] + (m1[1] - m0[1]) * f;
    result[2] = m0[2] + (m1[2] - m0[2]) * f;
    result[3] = m0[3] + (m1[3] - m0[3]) * f;
    return result;
}

mfloat_t _mat3_determinant(mfloat_t *m0) {
    mfloat_t m11 = m0[0];
    mfloat_t m21 = m0[1];
    mfloat_t m31 = m0[2];
    mfloat_t m12 = m0[3];
    mfloat_t m22 = m0[4];
    mfloat_t m32 = m0[5];
    mfloat_t m13 = m0[6];
    mfloat_t m23 = m0[7];
    mfloat_t m33 = m0[8];
    mfloat_t determinant = m11 * m22 * m33 + m12 * m23 * m31 + m13 * m21 * m32 -
                           m11 * m23 * m32 - m12 * m21 * m33 - m13 * m22 * m31;
    return determinant;
}

mfloat_t *_mat3_assign(mfloat_t *result, mfloat_t *m0) {
    result[0] = m0[0];
    result[1] = m0[1];
    result[2] = m0[2];
    result[3] = m0[3];
    result[4] = m0[4];
    result[5] = m0[5];
    result[6] = m0[6];
    result[7] = m0[7];
    result[8] = m0[8];
    return result;
}

mfloat_t *_mat3_transpose(mfloat_t *result, mfloat_t *m0) {
    mfloat_t transposed[MAT4_SIZE];
    transposed[0] = m0[0];
    transposed[1] = m0[3];
    transposed[2] = m0[6];
    transposed[3] = m0[1];
    transposed[4] = m0[4];
    transposed[5] = m0[7];
    transposed[6] = m0[2];
    transposed[7] = m0[5];
    transposed[8] = m0[8];
    result[0] = transposed[0];
    result[1] = transposed[1];
    result[2] = transposed[2];
    result[3] = transposed[3];
    result[4] = transposed[4];
    result[5] = transposed[5];
    result[6] = transposed[6];
    result[7] = transposed[7];
    result[8] = transposed[8];
    return result;
}

mfloat_t *_mat3_cofactor(mfloat_t *result, mfloat_t *m0) {
    mfloat_t cofactor[MAT3_SIZE];
    mfloat_t minor[MAT2_SIZE];
    minor[0] = m0[4];
    minor[1] = m0[5];
    minor[2] = m0[7];
    minor[3] = m0[8];
    cofactor[0] = _mat2_determinant(minor);
    minor[0] = m0[3];
    minor[1] = m0[5];
    minor[2] = m0[6];
    minor[3] = m0[8];
    cofactor[1] = -_mat2_determinant(minor);
    minor[0] = m0[3];
    minor[1] = m0[4];
    minor[2] = m0[6];
    minor[3] = m0[7];
    cofactor[2] = _mat2_determinant(minor);
    minor[0] = m0[1];
    minor[1] = m0[2];
    minor[2] = m0[7];
    minor[3] = m0[8];
    cofactor[3] = -_mat2_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[2];
    minor[2] = m0[6];
    minor[3] = m0[8];
    cofactor[4] = _mat2_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[1];
    minor[2] = m0[6];
    minor[3] = m0[7];
    cofactor[5] = -_mat2_determinant(minor);
    minor[0] = m0[1];
    minor[1] = m0[2];
    minor[2] = m0[4];
    minor[3] = m0[5];
    cofactor[6] = _mat2_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[2];
    minor[2] = m0[3];
    minor[3] = m0[5];
    cofactor[7] = -_mat2_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[1];
    minor[2] = m0[3];
    minor[3] = m0[4];
    cofactor[8] = _mat2_determinant(minor);
    result[0] = cofactor[0];
    result[1] = cofactor[1];
    result[2] = cofactor[2];
    result[3] = cofactor[3];
    result[4] = cofactor[4];
    result[5] = cofactor[5];
    result[6] = cofactor[6];
    result[7] = cofactor[7];
    result[8] = cofactor[8];
    return result;
}

mfloat_t *_mat3_multiply(mfloat_t *result, mfloat_t *m0, mfloat_t *m1) {
    mfloat_t multiplied[MAT3_SIZE];
    multiplied[0] = m0[0] * m1[0] + m0[3] * m1[1] + m0[6] * m1[2];
    multiplied[1] = m0[1] * m1[0] + m0[4] * m1[1] + m0[7] * m1[2];
    multiplied[2] = m0[2] * m1[0] + m0[5] * m1[1] + m0[8] * m1[2];
    multiplied[3] = m0[0] * m1[3] + m0[3] * m1[4] + m0[6] * m1[5];
    multiplied[4] = m0[1] * m1[3] + m0[4] * m1[4] + m0[7] * m1[5];
    multiplied[5] = m0[2] * m1[3] + m0[5] * m1[4] + m0[8] * m1[5];
    multiplied[6] = m0[0] * m1[6] + m0[3] * m1[7] + m0[6] * m1[8];
    multiplied[7] = m0[1] * m1[6] + m0[4] * m1[7] + m0[7] * m1[8];
    multiplied[8] = m0[2] * m1[6] + m0[5] * m1[7] + m0[8] * m1[8];
    result[0] = multiplied[0];
    result[1] = multiplied[1];
    result[2] = multiplied[2];
    result[3] = multiplied[3];
    result[4] = multiplied[4];
    result[5] = multiplied[5];
    result[6] = multiplied[6];
    result[7] = multiplied[7];
    result[8] = multiplied[8];
    return result;
}

mfloat_t *_mat3_scaling(mfloat_t *result, mfloat_t *v0) {
    result[0] = v0[0];
    result[4] = v0[1];
    result[8] = v0[2];
    return result;
}

mfloat_t *_mat3_scale(mfloat_t *result, const mfloat_t *m0, const mfloat_t *v0) {
    result[0] = m0[0] * v0[0];
    result[4] = m0[4] * v0[1];
    result[8] = m0[8] * v0[2];
    return result;
}

mfloat_t *_mat3_rotation_x(mfloat_t *result, mfloat_t f) {
    mfloat_t c = MCOS(f);
    mfloat_t s = MSIN(f);
    result[4] = c;
    result[5] = s;
    result[7] = -s;
    result[8] = c;
    return result;
}

mfloat_t *_mat3_rotation_y(mfloat_t *result, mfloat_t f) {
    mfloat_t c = MCOS(f);
    mfloat_t s = MSIN(f);
    result[0] = c;
    result[2] = -s;
    result[6] = s;
    result[8] = c;
    return result;
}

mfloat_t *_mat3_rotation_z(mfloat_t *result, mfloat_t f) {
    mfloat_t c = MCOS(f);
    mfloat_t s = MSIN(f);
    result[0] = c;
    result[1] = s;
    result[3] = -s;
    result[4] = c;
    return result;
}

mfloat_t *_mat3_rotation_axis(mfloat_t *result, mfloat_t *v0, mfloat_t f) {
    mfloat_t c = MCOS(f);
    mfloat_t s = MSIN(f);
    mfloat_t one_c = MFLOAT_C(1.0) - c;
    mfloat_t x = v0[0];
    mfloat_t y = v0[4];
    mfloat_t z = v0[8];
    mfloat_t xx = x * x;
    mfloat_t xy = x * y;
    mfloat_t xz = x * z;
    mfloat_t yy = y * y;
    mfloat_t yz = y * z;
    mfloat_t zz = z * z;
    mfloat_t l = xx + yy + zz;
    mfloat_t sqrt_l = MSQRT(l);
    result[0] = (xx + (yy + zz) * c) / l;
    result[1] = (xy * one_c + v0[2] * sqrt_l * s) / l;
    result[2] = (xz * one_c - v0[1] * sqrt_l * s) / l;
    result[3] = (xy * one_c - v0[2] * sqrt_l * s) / l;
    result[4] = (yy + (xx + zz) * c) / l;
    result[5] = (yz * one_c + v0[0] * sqrt_l * s) / l;
    result[6] = (xz * one_c + v0[1] * sqrt_l * s) / l;
    result[7] = (yz * one_c - v0[0] * sqrt_l * s) / l;
    result[8] = (zz + (xx + yy) * c) / l;
    return result;
}

mfloat_t *_mat3_rotation_quat(mfloat_t *result, const mfloat_t *q0) {
    mfloat_t xx = q0[0] * q0[0];
    mfloat_t yy = q0[1] * q0[1];
    mfloat_t zz = q0[2] * q0[2];
    mfloat_t xy = q0[0] * q0[1];
    mfloat_t zw = q0[2] * q0[3];
    mfloat_t xz = q0[0] * q0[2];
    mfloat_t yw = q0[1] * q0[3];
    mfloat_t yz = q0[1] * q0[2];
    mfloat_t xw = q0[0] * q0[3];
    result[0] = MFLOAT_C(1.0) - MFLOAT_C(2.0) * (yy + zz);
    result[1] = MFLOAT_C(2.0) * (xy + zw);
    result[2] = MFLOAT_C(2.0) * (xz - yw);
    result[3] = MFLOAT_C(2.0) * (xy - zw);
    result[4] = MFLOAT_C(1.0) - MFLOAT_C(2.0) * (xx + zz);
    result[5] = MFLOAT_C(2.0) * (yz + xw);
    result[6] = MFLOAT_C(2.0) * (xz + yw);
    result[7] = MFLOAT_C(2.0) * (yz - xw);
    result[8] = MFLOAT_C(1.0) - MFLOAT_C(2.0) * (xx + yy);
    return result;
}

mfloat_t *_mat3_lerp(mfloat_t *result, mfloat_t *m0, mfloat_t *m1, mfloat_t f) {
    result[0] = m0[0] + (m1[0] - m0[0]) * f;
    result[1] = m0[1] + (m1[1] - m0[1]) * f;
    result[2] = m0[2] + (m1[2] - m0[2]) * f;
    result[3] = m0[3] + (m1[3] - m0[3]) * f;
    result[4] = m0[4] + (m1[4] - m0[4]) * f;
    result[5] = m0[5] + (m1[5] - m0[5]) * f;
    result[6] = m0[6] + (m1[6] - m0[6]) * f;
    result[7] = m0[7] + (m1[7] - m0[7]) * f;
    result[8] = m0[8] + (m1[8] - m0[8]) * f;
    return result;
}

mfloat_t _mat4_determinant(mfloat_t *m0) {
    mfloat_t m11 = m0[0];
    mfloat_t m21 = m0[1];
    mfloat_t m31 = m0[2];
    mfloat_t m41 = m0[3];
    mfloat_t m12 = m0[4];
    mfloat_t m22 = m0[5];
    mfloat_t m32 = m0[6];
    mfloat_t m42 = m0[7];
    mfloat_t m13 = m0[8];
    mfloat_t m23 = m0[9];
    mfloat_t m33 = m0[10];
    mfloat_t m43 = m0[11];
    mfloat_t m14 = m0[12];
    mfloat_t m24 = m0[13];
    mfloat_t m34 = m0[14];
    mfloat_t m44 = m0[15];
    mfloat_t determinant =
        m14 * m23 * m32 * m41 - m13 * m24 * m32 * m41 - m14 * m22 * m33 * m41 +
        m12 * m24 * m33 * m41 + m13 * m22 * m34 * m41 - m12 * m23 * m34 * m41 -
        m14 * m23 * m31 * m42 + m13 * m24 * m31 * m42 + m14 * m21 * m33 * m42 -
        m11 * m24 * m33 * m42 - m13 * m21 * m34 * m42 + m11 * m23 * m34 * m42 +
        m14 * m22 * m31 * m43 - m12 * m24 * m31 * m43 - m14 * m21 * m32 * m43 +
        m11 * m24 * m32 * m43 + m12 * m21 * m34 * m43 - m11 * m22 * m34 * m43 -
        m13 * m22 * m31 * m44 + m12 * m23 * m31 * m44 + m13 * m21 * m32 * m44 -
        m11 * m23 * m32 * m44 - m12 * m21 * m33 * m44 + m11 * m22 * m33 * m44;
    return determinant;
}

mfloat_t *_mat4_assign(mfloat_t *result, mfloat_t *m0) {
    result[0] = m0[0];
    result[1] = m0[1];
    result[2] = m0[2];
    result[3] = m0[3];
    result[4] = m0[4];
    result[5] = m0[5];
    result[6] = m0[6];
    result[7] = m0[7];
    result[8] = m0[8];
    result[9] = m0[9];
    result[10] = m0[10];
    result[11] = m0[11];
    result[12] = m0[12];
    result[13] = m0[13];
    result[14] = m0[14];
    result[15] = m0[15];
    return result;
}

mfloat_t *_mat4_transpose(mfloat_t *result, mfloat_t *m0) {
    mfloat_t transposed[MAT4_SIZE];
    transposed[0] = m0[0];
    transposed[1] = m0[4];
    transposed[2] = m0[8];
    transposed[3] = m0[12];
    transposed[4] = m0[1];
    transposed[5] = m0[5];
    transposed[6] = m0[9];
    transposed[7] = m0[13];
    transposed[8] = m0[2];
    transposed[9] = m0[6];
    transposed[10] = m0[10];
    transposed[11] = m0[14];
    transposed[12] = m0[3];
    transposed[13] = m0[7];
    transposed[14] = m0[11];
    transposed[15] = m0[15];
    result[0] = transposed[0];
    result[1] = transposed[1];
    result[2] = transposed[2];
    result[3] = transposed[3];
    result[4] = transposed[4];
    result[5] = transposed[5];
    result[6] = transposed[6];
    result[7] = transposed[7];
    result[8] = transposed[8];
    result[9] = transposed[9];
    result[10] = transposed[10];
    result[11] = transposed[11];
    result[12] = transposed[12];
    result[13] = transposed[13];
    result[14] = transposed[14];
    result[15] = transposed[15];
    return result;
}

mfloat_t *_mat4_cofactor(mfloat_t *result, mfloat_t *m0) {
    mfloat_t cofactor[MAT4_SIZE];
    mfloat_t minor[MAT3_SIZE];
    minor[0] = m0[5];
    minor[1] = m0[6];
    minor[2] = m0[7];
    minor[3] = m0[9];
    minor[4] = m0[10];
    minor[5] = m0[11];
    minor[6] = m0[13];
    minor[7] = m0[14];
    minor[8] = m0[15];
    cofactor[0] = _mat3_determinant(minor);
    minor[0] = m0[4];
    minor[1] = m0[6];
    minor[2] = m0[7];
    minor[3] = m0[8];
    minor[4] = m0[10];
    minor[5] = m0[11];
    minor[6] = m0[12];
    minor[7] = m0[14];
    minor[8] = m0[15];
    cofactor[1] = -_mat3_determinant(minor);
    minor[0] = m0[4];
    minor[1] = m0[5];
    minor[2] = m0[7];
    minor[3] = m0[8];
    minor[4] = m0[9];
    minor[5] = m0[11];
    minor[6] = m0[12];
    minor[7] = m0[13];
    minor[8] = m0[15];
    cofactor[2] = _mat3_determinant(minor);
    minor[0] = m0[4];
    minor[1] = m0[5];
    minor[2] = m0[6];
    minor[3] = m0[8];
    minor[4] = m0[9];
    minor[5] = m0[10];
    minor[6] = m0[12];
    minor[7] = m0[13];
    minor[8] = m0[14];
    cofactor[3] = -_mat3_determinant(minor);
    minor[0] = m0[1];
    minor[1] = m0[2];
    minor[2] = m0[3];
    minor[3] = m0[9];
    minor[4] = m0[10];
    minor[5] = m0[11];
    minor[6] = m0[13];
    minor[7] = m0[14];
    minor[8] = m0[15];
    cofactor[4] = -_mat3_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[2];
    minor[2] = m0[3];
    minor[3] = m0[8];
    minor[4] = m0[10];
    minor[5] = m0[11];
    minor[6] = m0[12];
    minor[7] = m0[14];
    minor[8] = m0[15];
    cofactor[5] = _mat3_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[1];
    minor[2] = m0[3];
    minor[3] = m0[8];
    minor[4] = m0[9];
    minor[5] = m0[11];
    minor[6] = m0[12];
    minor[7] = m0[13];
    minor[8] = m0[15];
    cofactor[6] = -_mat3_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[1];
    minor[2] = m0[2];
    minor[3] = m0[8];
    minor[4] = m0[9];
    minor[5] = m0[10];
    minor[6] = m0[12];
    minor[7] = m0[13];
    minor[8] = m0[14];
    cofactor[7] = _mat3_determinant(minor);
    minor[0] = m0[1];
    minor[1] = m0[2];
    minor[2] = m0[3];
    minor[3] = m0[5];
    minor[4] = m0[6];
    minor[5] = m0[7];
    minor[6] = m0[13];
    minor[7] = m0[14];
    minor[8] = m0[15];
    cofactor[8] = _mat3_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[2];
    minor[2] = m0[3];
    minor[3] = m0[4];
    minor[4] = m0[6];
    minor[5] = m0[7];
    minor[6] = m0[12];
    minor[7] = m0[14];
    minor[8] = m0[15];
    cofactor[9] = -_mat3_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[1];
    minor[2] = m0[3];
    minor[3] = m0[4];
    minor[4] = m0[5];
    minor[5] = m0[7];
    minor[6] = m0[12];
    minor[7] = m0[13];
    minor[8] = m0[15];
    cofactor[10] = _mat3_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[1];
    minor[2] = m0[2];
    minor[3] = m0[4];
    minor[4] = m0[5];
    minor[5] = m0[6];
    minor[6] = m0[12];
    minor[7] = m0[13];
    minor[8] = m0[14];
    cofactor[11] = -_mat3_determinant(minor);
    minor[0] = m0[1];
    minor[1] = m0[2];
    minor[2] = m0[3];
    minor[3] = m0[5];
    minor[4] = m0[6];
    minor[5] = m0[7];
    minor[6] = m0[9];
    minor[7] = m0[10];
    minor[8] = m0[11];
    cofactor[12] = -_mat3_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[2];
    minor[2] = m0[3];
    minor[3] = m0[4];
    minor[4] = m0[6];
    minor[5] = m0[7];
    minor[6] = m0[8];
    minor[7] = m0[10];
    minor[8] = m0[11];
    cofactor[13] = _mat3_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[1];
    minor[2] = m0[3];
    minor[3] = m0[4];
    minor[4] = m0[5];
    minor[5] = m0[7];
    minor[6] = m0[8];
    minor[7] = m0[9];
    minor[8] = m0[11];
    cofactor[14] = -_mat3_determinant(minor);
    minor[0] = m0[0];
    minor[1] = m0[1];
    minor[2] = m0[2];
    minor[3] = m0[4];
    minor[4] = m0[5];
    minor[5] = m0[6];
    minor[6] = m0[8];
    minor[7] = m0[9];
    minor[8] = m0[10];
    cofactor[15] = _mat3_determinant(minor);
    result[0] = cofactor[0];
    result[1] = cofactor[1];
    result[2] = cofactor[2];
    result[3] = cofactor[3];
    result[4] = cofactor[4];
    result[5] = cofactor[5];
    result[6] = cofactor[6];
    result[7] = cofactor[7];
    result[8] = cofactor[8];
    result[9] = cofactor[9];
    result[10] = cofactor[10];
    result[11] = cofactor[11];
    result[12] = cofactor[12];
    result[13] = cofactor[13];
    result[14] = cofactor[14];
    result[15] = cofactor[15];
    return result;
}

mfloat_t *_mat4_rotation_x(mfloat_t *result, mfloat_t f) {
    mfloat_t c = MCOS(f);
    mfloat_t s = MSIN(f);
    result[5] = c;
    result[6] = s;
    result[9] = -s;
    result[10] = c;
    return result;
}

mfloat_t *_mat4_rotation_y(mfloat_t *result, mfloat_t f) {
    mfloat_t c = MCOS(f);
    mfloat_t s = MSIN(f);
    result[0] = c;
    result[2] = -s;
    result[8] = s;
    result[10] = c;
    return result;
}

mfloat_t *_mat4_rotation_z(mfloat_t *result, mfloat_t f) {
    mfloat_t c = MCOS(f);
    mfloat_t s = MSIN(f);
    result[0] = c;
    result[1] = s;
    result[4] = -s;
    result[5] = c;
    return result;
}

mfloat_t *_mat4_rotation_axis(mfloat_t *result, mfloat_t *v0, mfloat_t f) {
    mfloat_t c = MCOS(f);
    mfloat_t s = MSIN(f);
    mfloat_t one_c = MFLOAT_C(1.0) - c;
    mfloat_t x = v0[0];
    mfloat_t y = v0[1];
    mfloat_t z = v0[2];
    mfloat_t xx = x * x;
    mfloat_t xy = x * y;
    mfloat_t xz = x * z;
    mfloat_t yy = y * y;
    mfloat_t yz = y * z;
    mfloat_t zz = z * z;
    mfloat_t l = xx + yy + zz;
    mfloat_t sqrt_l = MSQRT(l);
    result[0] = (xx + (yy + zz) * c) / l;
    result[1] = (xy * one_c + v0[2] * sqrt_l * s) / l;
    result[2] = (xz * one_c - v0[1] * sqrt_l * s) / l;
    result[3] = MFLOAT_C(0.0);
    result[4] = (xy * one_c - v0[2] * sqrt_l * s) / l;
    result[5] = (yy + (xx + zz) * c) / l;
    result[6] = (yz * one_c + v0[0] * sqrt_l * s) / l;
    result[7] = MFLOAT_C(0.0);
    result[8] = (xz * one_c + v0[1] * sqrt_l * s) / l;
    result[9] = (yz * one_c - v0[0] * sqrt_l * s) / l;
    result[10] = (zz + (xx + yy) * c) / l;
    result[11] = MFLOAT_C(0.0);
    result[12] = MFLOAT_C(0.0);
    result[13] = MFLOAT_C(0.0);
    result[14] = MFLOAT_C(0.0);
    result[15] = MFLOAT_C(1.0);
    return result;
}

mfloat_t *_mat4_rotation_quat(mfloat_t *result, const mfloat_t *q0) {
    mfloat_t xx = q0[0] * q0[0];
    mfloat_t yy = q0[1] * q0[1];
    mfloat_t zz = q0[2] * q0[2];
    mfloat_t xy = q0[0] * q0[1];
    mfloat_t zw = q0[2] * q0[3];
    mfloat_t xz = q0[0] * q0[2];
    mfloat_t yw = q0[1] * q0[3];
    mfloat_t yz = q0[1] * q0[2];
    mfloat_t xw = q0[0] * q0[3];
    result[0] = MFLOAT_C(1.0) - MFLOAT_C(2.0) * (yy + zz);
    result[1] = MFLOAT_C(2.0) * (xy + zw);
    result[2] = MFLOAT_C(2.0) * (xz - yw);
    result[3] = MFLOAT_C(0.0);
    result[4] = MFLOAT_C(2.0) * (xy - zw);
    result[5] = MFLOAT_C(1.0) - MFLOAT_C(2.0) * (xx + zz);
    result[6] = MFLOAT_C(2.0) * (yz + xw);
    result[7] = MFLOAT_C(0.0);
    result[8] = MFLOAT_C(2.0) * (xz + yw);
    result[9] = MFLOAT_C(2.0) * (yz - xw);
    result[10] = MFLOAT_C(1.0) - MFLOAT_C(2.0) * (xx + yy);
    result[11] = MFLOAT_C(0.0);
    result[12] = MFLOAT_C(0.0);
    result[13] = MFLOAT_C(0.0);
    result[14] = MFLOAT_C(0.0);
    result[15] = MFLOAT_C(1.0);
    return result;
}

mfloat_t *_mat4_translation(mfloat_t *result, const mfloat_t *m0,
                            const mfloat_t *v0) {
    result[0] = m0[0];
    result[1] = m0[1];
    result[2] = m0[2];
    result[3] = m0[3];
    result[4] = m0[4];
    result[5] = m0[5];
    result[6] = m0[6];
    result[7] = m0[7];
    result[8] = m0[8];
    result[9] = m0[9];
    result[10] = m0[10];
    result[11] = m0[11];
    result[12] = v0[0];
    result[13] = v0[1];
    result[14] = v0[2];
    result[15] = m0[15];
    return result;
}

mfloat_t *_mat4_translate(mfloat_t *result, mfloat_t *m0, mfloat_t *v0) {
    result[0] = m0[0];
    result[1] = m0[1];
    result[2] = m0[2];
    result[3] = m0[3];
    result[4] = m0[4];
    result[5] = m0[5];
    result[6] = m0[6];
    result[7] = m0[7];
    result[8] = m0[8];
    result[9] = m0[9];
    result[10] = m0[10];
    result[11] = m0[11];
    result[12] = m0[12] + v0[0];
    result[13] = m0[13] + v0[1];
    result[14] = m0[14] + v0[2];
    result[15] = m0[15];
    return result;
}

mfloat_t *_mat4_scaling(mfloat_t *result, mfloat_t *m0, mfloat_t *v0) {
    result[0] = v0[0];
    result[1] = m0[1];
    result[2] = m0[2];
    result[3] = m0[3];
    result[4] = m0[4];
    result[5] = v0[1];
    result[6] = m0[6];
    result[7] = m0[7];
    result[8] = m0[8];
    result[9] = m0[9];
    result[10] = v0[2];
    result[11] = m0[11];
    result[12] = m0[12];
    result[13] = m0[13];
    result[14] = m0[14];
    result[15] = m0[15];
    return result;
}

mfloat_t *_mat4_scale(mfloat_t *result, mfloat_t *m0, mfloat_t *v0) {
    result[0] = m0[0] * v0[0];
    result[1] = m0[1];
    result[2] = m0[2];
    result[3] = m0[3];
    result[4] = m0[4];
    result[5] = m0[5] * v0[1];
    result[6] = m0[6];
    result[7] = m0[7];
    result[8] = m0[8];
    result[9] = m0[9];
    result[10] = m0[10] * v0[2];
    result[11] = m0[11];
    result[12] = m0[12];
    result[13] = m0[13];
    result[14] = m0[14];
    result[15] = m0[15];
    return result;
}

mfloat_t *_mat4_multiply(mfloat_t *result, mfloat_t *m0, mfloat_t *m1) {
    mfloat_t multiplied[MAT4_SIZE];
    multiplied[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
    multiplied[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
    multiplied[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
    multiplied[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
    multiplied[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
    multiplied[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
    multiplied[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
    multiplied[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
    multiplied[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
    multiplied[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
    multiplied[10] =
        m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
    multiplied[11] =
        m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
    multiplied[12] =
        m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
    multiplied[13] =
        m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
    multiplied[14] =
        m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
    multiplied[15] =
        m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
    result[0] = multiplied[0];
    result[1] = multiplied[1];
    result[2] = multiplied[2];
    result[3] = multiplied[3];
    result[4] = multiplied[4];
    result[5] = multiplied[5];
    result[6] = multiplied[6];
    result[7] = multiplied[7];
    result[8] = multiplied[8];
    result[9] = multiplied[9];
    result[10] = multiplied[10];
    result[11] = multiplied[11];
    result[12] = multiplied[12];
    result[13] = multiplied[13];
    result[14] = multiplied[14];
    result[15] = multiplied[15];
    return result;
}

mfloat_t *_mat4_inverse(mfloat_t *result, mfloat_t *m0) {
    mfloat_t inverse[MAT4_SIZE];
    mfloat_t inverted_determinant;
    mfloat_t m11 = m0[0];
    mfloat_t m21 = m0[1];
    mfloat_t m31 = m0[2];
    mfloat_t m41 = m0[3];
    mfloat_t m12 = m0[4];
    mfloat_t m22 = m0[5];
    mfloat_t m32 = m0[6];
    mfloat_t m42 = m0[7];
    mfloat_t m13 = m0[8];
    mfloat_t m23 = m0[9];
    mfloat_t m33 = m0[10];
    mfloat_t m43 = m0[11];
    mfloat_t m14 = m0[12];
    mfloat_t m24 = m0[13];
    mfloat_t m34 = m0[14];
    mfloat_t m44 = m0[15];
    inverse[0] = m22 * m33 * m44 - m22 * m43 * m34 - m23 * m32 * m44 +
                 m23 * m42 * m34 + m24 * m32 * m43 - m24 * m42 * m33;
    inverse[4] = -m12 * m33 * m44 + m12 * m43 * m34 + m13 * m32 * m44 -
                 m13 * m42 * m34 - m14 * m32 * m43 + m14 * m42 * m33;
    inverse[8] = m12 * m23 * m44 - m12 * m43 * m24 - m13 * m22 * m44 +
                 m13 * m42 * m24 + m14 * m22 * m43 - m14 * m42 * m23;
    inverse[12] = -m12 * m23 * m34 + m12 * m33 * m24 + m13 * m22 * m34 -
                  m13 * m32 * m24 - m14 * m22 * m33 + m14 * m32 * m23;
    inverse[1] = -m21 * m33 * m44 + m21 * m43 * m34 + m23 * m31 * m44 -
                 m23 * m41 * m34 - m24 * m31 * m43 + m24 * m41 * m33;
    inverse[5] = m11 * m33 * m44 - m11 * m43 * m34 - m13 * m31 * m44 +
                 m13 * m41 * m34 + m14 * m31 * m43 - m14 * m41 * m33;
    inverse[9] = -m11 * m23 * m44 + m11 * m43 * m24 + m13 * m21 * m44 -
                 m13 * m41 * m24 - m14 * m21 * m43 + m14 * m41 * m23;
    inverse[13] = m11 * m23 * m34 - m11 * m33 * m24 - m13 * m21 * m34 +
                  m13 * m31 * m24 + m14 * m21 * m33 - m14 * m31 * m23;
    inverse[2] = m21 * m32 * m44 - m21 * m42 * m34 - m22 * m31 * m44 +
                 m22 * m41 * m34 + m24 * m31 * m42 - m24 * m41 * m32;
    inverse[6] = -m11 * m32 * m44 + m11 * m42 * m34 + m12 * m31 * m44 -
                 m12 * m41 * m34 - m14 * m31 * m42 + m14 * m41 * m32;
    inverse[10] = m11 * m22 * m44 - m11 * m42 * m24 - m12 * m21 * m44 +
                  m12 * m41 * m24 + m14 * m21 * m42 - m14 * m41 * m22;
    inverse[14] = -m11 * m22 * m34 + m11 * m32 * m24 + m12 * m21 * m34 -
                  m12 * m31 * m24 - m14 * m21 * m32 + m14 * m31 * m22;
    inverse[3] = -m21 * m32 * m43 + m21 * m42 * m33 + m22 * m31 * m43 -
                 m22 * m41 * m33 - m23 * m31 * m42 + m23 * m41 * m32;
    inverse[7] = m11 * m32 * m43 - m11 * m42 * m33 - m12 * m31 * m43 +
                 m12 * m41 * m33 + m13 * m31 * m42 - m13 * m41 * m32;
    inverse[11] = -m11 * m22 * m43 + m11 * m42 * m23 + m12 * m21 * m43 -
                  m12 * m41 * m23 - m13 * m21 * m42 + m13 * m41 * m22;
    inverse[15] = m11 * m22 * m33 - m11 * m32 * m23 - m12 * m21 * m33 +
                  m12 * m31 * m23 + m13 * m21 * m32 - m13 * m31 * m22;
    inverted_determinant = MFLOAT_C(1.0) / (m11 * inverse[0] + m21 * inverse[4] +
                                            m31 * inverse[8] + m41 * inverse[12]);
    result[0] = inverse[0] * inverted_determinant;
    result[1] = inverse[1] * inverted_determinant;
    result[2] = inverse[2] * inverted_determinant;
    result[3] = inverse[3] * inverted_determinant;
    result[4] = inverse[4] * inverted_determinant;
    result[5] = inverse[5] * inverted_determinant;
    result[6] = inverse[6] * inverted_determinant;
    result[7] = inverse[7] * inverted_determinant;
    result[8] = inverse[8] * inverted_determinant;
    result[9] = inverse[9] * inverted_determinant;
    result[10] = inverse[10] * inverted_determinant;
    result[11] = inverse[11] * inverted_determinant;
    result[12] = inverse[12] * inverted_determinant;
    result[13] = inverse[13] * inverted_determinant;
    result[14] = inverse[14] * inverted_determinant;
    result[15] = inverse[15] * inverted_determinant;
    return result;
}

mfloat_t *_mat4_lerp(mfloat_t *result, mfloat_t *m0, mfloat_t *m1, mfloat_t f) {
    result[0] = m0[0] + (m1[0] - m0[0]) * f;
    result[1] = m0[1] + (m1[1] - m0[1]) * f;
    result[2] = m0[2] + (m1[2] - m0[2]) * f;
    result[3] = m0[3] + (m1[3] - m0[3]) * f;
    result[4] = m0[4] + (m1[4] - m0[4]) * f;
    result[5] = m0[5] + (m1[5] - m0[5]) * f;
    result[6] = m0[6] + (m1[6] - m0[6]) * f;
    result[7] = m0[7] + (m1[7] - m0[7]) * f;
    result[8] = m0[8] + (m1[8] - m0[8]) * f;
    result[9] = m0[9] + (m1[9] - m0[9]) * f;
    result[10] = m0[10] + (m1[10] - m0[10]) * f;
    result[11] = m0[11] + (m1[11] - m0[11]) * f;
    result[12] = m0[12] + (m1[12] - m0[12]) * f;
    result[13] = m0[13] + (m1[13] - m0[13]) * f;
    result[14] = m0[14] + (m1[14] - m0[14]) * f;
    result[15] = m0[15] + (m1[15] - m0[15]) * f;
    return result;
}

mfloat_t *_mat4_look_at(mfloat_t *result, mfloat_t *position, mfloat_t *target,
                        mfloat_t *up) {
    mfloat_t tmp_forward[VEC3_SIZE];
    mfloat_t tmp_side[VEC3_SIZE];
    mfloat_t tmp_up[VEC3_SIZE];
    _vec3_subtract(tmp_forward, target, position);
    _vec3_normalize(tmp_forward, tmp_forward);
    _vec3_cross(tmp_side, tmp_forward, up);
    _vec3_normalize(tmp_side, tmp_side);
    _vec3_cross(tmp_up, tmp_side, tmp_forward);
    result[0] = tmp_side[0];
    result[1] = tmp_up[0];
    result[2] = -tmp_forward[0];
    result[3] = MFLOAT_C(0.0);
    result[4] = tmp_side[1];
    result[5] = tmp_up[1];
    result[6] = -tmp_forward[1];
    result[7] = MFLOAT_C(0.0);
    result[8] = tmp_side[2];
    result[9] = tmp_up[2];
    result[10] = -tmp_forward[2];
    result[11] = MFLOAT_C(0.0);
    result[12] = -_vec3_dot(tmp_side, position);
    result[13] = -_vec3_dot(tmp_up, position);
    result[14] = _vec3_dot(tmp_forward, position);
    result[15] = MFLOAT_C(1.0);
    return result;
}

mfloat_t *_mat4_ortho(mfloat_t *result, mfloat_t l, mfloat_t r, mfloat_t b,
                      mfloat_t t, mfloat_t n, mfloat_t f) {
    result[0] = MFLOAT_C(2.0) / (r - l);
    result[1] = MFLOAT_C(0.0);
    result[2] = MFLOAT_C(0.0);
    result[3] = MFLOAT_C(0.0);
    result[4] = MFLOAT_C(0.0);
    result[5] = MFLOAT_C(2.0) / (t - b);
    result[6] = MFLOAT_C(0.0);
    result[7] = MFLOAT_C(0.0);
    result[8] = MFLOAT_C(0.0);
    result[9] = MFLOAT_C(0.0);
    result[10] = -MFLOAT_C(2.0) / (f - n);
    result[11] = MFLOAT_C(0.0);
    result[12] = -((r + l) / (r - l));
    result[13] = -((t + b) / (t - b));
    result[14] = -((f + n) / (f - n));
    result[15] = MFLOAT_C(1.0);
    return result;
}

mfloat_t *_mat4_perspective(mfloat_t *result, mfloat_t fov_y, mfloat_t aspect,
                            mfloat_t n, mfloat_t f) {
    mfloat_t tan_half_fov_y = MFLOAT_C(1.0) / MTAN(fov_y * MFLOAT_C(0.5));
    result[0] = MFLOAT_C(1.0) / aspect * tan_half_fov_y;
    result[1] = MFLOAT_C(0.0);
    result[2] = MFLOAT_C(0.0);
    result[3] = MFLOAT_C(0.0);
    result[4] = MFLOAT_C(0.0);
    result[5] = MFLOAT_C(1.0) / tan_half_fov_y;
    result[6] = MFLOAT_C(0.0);
    result[7] = MFLOAT_C(0.0);
    result[8] = MFLOAT_C(0.0);
    result[9] = MFLOAT_C(0.0);
    result[10] = f / (n - f);
    result[11] = -MFLOAT_C(1.0);
    result[12] = MFLOAT_C(0.0);
    result[13] = MFLOAT_C(0.0);
    result[14] = -(f * n) / (f - n);
    result[15] = MFLOAT_C(0.0);
    return result;
}

mfloat_t *_mat4_perspective_fov(mfloat_t *result, mfloat_t fov, mfloat_t w,
                                mfloat_t h, mfloat_t n, mfloat_t f) {
    mfloat_t h2 = MCOS(fov * MFLOAT_C(0.5)) / MSIN(fov * MFLOAT_C(0.5));
    mfloat_t w2 = h2 * h / w;
    result[0] = w2;
    result[1] = MFLOAT_C(0.0);
    result[2] = MFLOAT_C(0.0);
    result[3] = MFLOAT_C(0.0);
    result[4] = MFLOAT_C(0.0);
    result[5] = h2;
    result[6] = MFLOAT_C(0.0);
    result[7] = MFLOAT_C(0.0);
    result[8] = MFLOAT_C(0.0);
    result[9] = MFLOAT_C(0.0);
    result[10] = f / (n - f);
    result[11] = -MFLOAT_C(1.0);
    result[12] = MFLOAT_C(0.0);
    result[13] = MFLOAT_C(0.0);
    result[14] = -(f * n) / (f - n);
    result[15] = MFLOAT_C(0.0);
    return result;
}

mfloat_t *_mat4_perspective_infinite(mfloat_t *result, mfloat_t fov_y,
                                     mfloat_t aspect, mfloat_t n) {
    mfloat_t range = MTAN(fov_y * MFLOAT_C(0.5)) * n;
    mfloat_t left = -range * aspect;
    mfloat_t right = range * aspect;
    mfloat_t top = range;
    mfloat_t bottom = -range;
    result[0] = MFLOAT_C(2.0) * n / (right - left);
    result[1] = MFLOAT_C(0.0);
    result[2] = MFLOAT_C(0.0);
    result[3] = MFLOAT_C(0.0);
    result[4] = MFLOAT_C(0.0);
    result[5] = MFLOAT_C(2.0) * n / (top - bottom);
    result[6] = MFLOAT_C(0.0);
    result[7] = MFLOAT_C(0.0);
    result[8] = MFLOAT_C(0.0);
    result[9] = MFLOAT_C(0.0);
    result[10] = -MFLOAT_C(1.0);
    result[11] = -MFLOAT_C(1.0);
    result[12] = MFLOAT_C(0.0);
    result[13] = MFLOAT_C(0.0);
    result[14] = -MFLOAT_C(2.0) * n;
    result[15] = MFLOAT_C(0.0);
    return result;
}

bool vec2i_is_zero(vec2i *v0) { return _vec2i_is_zero((mint_t *)v0); }

bool vec2i_is_equal(vec2i *v0, vec2i *v1) {
    return _vec2i_is_equal((mint_t *)v0, (mint_t *)v1);
}

vec2i *vec2i_assign(vec2i *result, vec2i *v0) {
    return (vec2i *)_vec2i_assign((mint_t *)result, (mint_t *)v0);
}

vec2i *vec2i_assign_vec2(vec2i *result, vec2 *v0) {
    return (vec2i *)_vec2i_assign_vec2((mint_t *)result, (mfloat_t *)v0);
}

vec2i *vec2i_sign(vec2i *result, vec2i *v0) {
    return (vec2i *)_vec2i_sign((mint_t *)result, (mint_t *)v0);
}

void vec2i_add(vec2i *result, vec2i *v0, vec2i *v1) {
    MAP_BINARY_OP(+, result->v, v0->v, v1->v, 2);
}

void vec2i_add_i(vec2i *result, vec2i *v0, mint_t i) {
    MAP_UNARY_FUNCTION(i +, result->v, v0->v, 2);
}

vec2i *vec2i_subtract(vec2i *result, vec2i *v0, vec2i *v1) {
    return (vec2i *)_vec2i_subtract((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec2i *vec2i_subtract_i(vec2i *result, vec2i *v0, mint_t i) {
    return (vec2i *)_vec2i_subtract_i((mint_t *)result, (mint_t *)v0, i);
}

vec2i *vec2i_multiply(vec2i *result, vec2i *v0, vec2i *v1) {
    return (vec2i *)_vec2i_multiply((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

void vec2i_multiply_i(vec2i *result, vec2i *v0, mint_t i) {
    MAP_UNARY_FUNCTION(i *, result->v, v0->v, 2);
}

vec2i *vec2i_divide(vec2i *result, vec2i *v0, vec2i *v1) {
    return (vec2i *)_vec2i_divide((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec2i *vec2i_divide_i(vec2i *result, vec2i *v0, mint_t i) {
    return (vec2i *)_vec2i_divide_i((mint_t *)result, (mint_t *)v0, i);
}

vec2i *vec2i_snap(vec2i *result, vec2i *v0, vec2i *v1) {
    return (vec2i *)_vec2i_snap((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec2i *vec2i_snap_i(vec2i *result, vec2i *v0, mint_t i) {
    return (vec2i *)_vec2i_snap_i((mint_t *)result, (mint_t *)v0, i);
}

void vec2i_negative(vec2i *result, vec2i *v0) {
    MAP_UNARY_FUNCTION(-, result->v, v0->v, 2);
}

void vec2i_abs(vec2i *result, vec2i *v0) {
    _vec2i_abs((mint_t *)result, (mint_t *)v0);
}

vec2i *vec2i_max(vec2i *result, vec2i *v0, vec2i *v1) {
    return (vec2i *)_vec2i_max((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec2i *vec2i_min(vec2i *result, vec2i *v0, vec2i *v1) {
    return (vec2i *)_vec2i_min((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec2i *vec2i_clamp(vec2i *result, vec2i *v0, vec2i *v1, vec2i *v2) {
    return (vec2i *)_vec2i_clamp((mint_t *)result, (mint_t *)v0, (mint_t *)v1,
                                 (mint_t *)v2);
}

vec2i *vec2i_tangent(vec2i *result, vec2i *v0) {
    return (vec2i *)_vec2i_tangent((mint_t *)result, (mint_t *)v0);
}

bool vec3i_is_zero(vec3i *v0) { return _vec3i_is_zero((mint_t *)v0); }

bool vec3i_is_equal(vec3i *v0, vec3i *v1) {
    return _vec3i_is_equal((mint_t *)v0, (mint_t *)v1);
}

vec3i *vec3i_assign(vec3i *result, vec3i *v0) {
    return (vec3i *)_vec3i_assign((mint_t *)result, (mint_t *)v0);
}

vec3i *vec3i_assign_vec3(vec3i *result, vec3 *v0) {
    return (vec3i *)_vec3i_assign_vec3((mint_t *)result, (mfloat_t *)v0);
}

vec3i *vec3i_sign(vec3i *result, vec3i *v0) {
    return (vec3i *)_vec3i_sign((mint_t *)result, (mint_t *)v0);
}

void vec3i_add(vec3i *result, vec3i *v0, vec3i *v1) {
    MAP_BINARY_OP(+, result->v, v0->v, v1->v, 3);
}

void vec3i_add_i(vec3i *result, vec3i *v0, mint_t i) {
    MAP_UNARY_FUNCTION(i +, result->v, v0->v, 3);
}

vec3i *vec3i_subtract(vec3i *result, vec3i *v0, vec3i *v1) {
    return (vec3i *)_vec3i_subtract((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec3i *vec3i_subtract_i(vec3i *result, vec3i *v0, mint_t i) {
    return (vec3i *)_vec3i_subtract_i((mint_t *)result, (mint_t *)v0, i);
}

vec3i *vec3i_multiply(vec3i *result, vec3i *v0, vec3i *v1) {
    return (vec3i *)_vec3i_multiply((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

void vec3i_multiply_i(vec3i *result, vec3i *v0, mint_t i) {
    MAP_UNARY_FUNCTION(i *, result->v, v0->v, 3);
}

vec3i *vec3i_divide(vec3i *result, vec3i *v0, vec3i *v1) {
    return (vec3i *)_vec3i_divide((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec3i *vec3i_divide_i(vec3i *result, vec3i *v0, mint_t i) {
    return (vec3i *)_vec3i_divide_i((mint_t *)result, (mint_t *)v0, i);
}

vec3i *vec3i_snap(vec3i *result, vec3i *v0, vec3i *v1) {
    return (vec3i *)_vec3i_snap((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec3i *vec3i_snap_i(vec3i *result, vec3i *v0, mint_t i) {
    return (vec3i *)_vec3i_snap_i((mint_t *)result, (mint_t *)v0, i);
}

vec3i *vec3i_cross(vec3i *result, vec3i *v0, vec3i *v1) {
    return (vec3i *)_vec3i_cross((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

void vec3i_negative(vec3i *result, vec3i *v0) {
    MAP_UNARY_FUNCTION(-, result->v, v0->v, 3);
}

vec3i *vec3i_abs(vec3i *result, vec3i *v0) {
    return (vec3i *)_vec3i_abs((mint_t *)result, (mint_t *)v0);
}

vec3i *vec3i_max(vec3i *result, vec3i *v0, vec3i *v1) {
    return (vec3i *)_vec3i_max((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec3i *vec3i_min(vec3i *result, vec3i *v0, vec3i *v1) {
    return (vec3i *)_vec3i_min((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec3i *vec3i_clamp(vec3i *result, vec3i *v0, vec3i *v1, vec3i *v2) {
    return (vec3i *)_vec3i_clamp((mint_t *)result, (mint_t *)v0, (mint_t *)v1,
                                 (mint_t *)v2);
}

bool vec4i_is_zero(vec4i *v0) { return _vec4i_is_zero((mint_t *)v0); }

bool vec4i_is_equal(vec4i *v0, vec4i *v1) {
    return _vec4i_is_equal((mint_t *)v0, (mint_t *)v1);
}

vec4i *vec4i_assign(vec4i *result, vec4i *v0) {
    return (vec4i *)_vec4i_assign((mint_t *)result, (mint_t *)v0);
}

vec4i *vec4i_assign_vec4(vec4i *result, vec4 *v0) {
    return (vec4i *)_vec4i_assign_vec4((mint_t *)result, (mfloat_t *)v0);
}

vec4i *vec4i_sign(vec4i *result, vec4i *v0) {
    return (vec4i *)_vec4i_sign((mint_t *)result, (mint_t *)v0);
}

vec4i *vec4i_add(vec4i *result, vec4i *v0, vec4i *v1) {
    return (vec4i *)_vec4i_add((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec4i *vec4i_add_i(vec4i *result, vec4i *v0, mint_t i) {
    return (vec4i *)_vec4i_add_i((mint_t *)result, (mint_t *)v0, i);
}

vec4i *vec4i_subtract(vec4i *result, vec4i *v0, vec4i *v1) {
    return (vec4i *)_vec4i_subtract((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec4i *vec4i_subtract_i(vec4i *result, vec4i *v0, mint_t i) {
    return (vec4i *)_vec4i_subtract_i((mint_t *)result, (mint_t *)v0, i);
}

vec4i *vec4i_multiply(vec4i *result, vec4i *v0, vec4i *v1) {
    return (vec4i *)_vec4i_multiply((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec4i *vec4i_multiply_i(vec4i *result, vec4i *v0, mint_t i) {
    return (vec4i *)_vec4i_multiply_i((mint_t *)result, (mint_t *)v0, i);
}

vec4i *vec4i_divide(vec4i *result, vec4i *v0, vec4i *v1) {
    return (vec4i *)_vec4i_divide((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec4i *vec4i_divide_i(vec4i *result, vec4i *v0, mint_t i) {
    return (vec4i *)_vec4i_divide_i((mint_t *)result, (mint_t *)v0, i);
}

vec4i *vec4i_snap(vec4i *result, vec4i *v0, vec4i *v1) {
    return (vec4i *)_vec4i_snap((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec4i *vec4i_snap_i(vec4i *result, vec4i *v0, mint_t i) {
    return (vec4i *)_vec4i_snap_i((mint_t *)result, (mint_t *)v0, i);
}

void vec4i_negative(vec4i *result, vec4i *v0) {
    MAP_UNARY_FUNCTION(-, result->v, v0->v, 4);
}

vec4i *vec4i_abs(vec4i *result, vec4i *v0) {
    return (vec4i *)_vec4i_abs((mint_t *)result, (mint_t *)v0);
}

vec4i *vec4i_max(vec4i *result, vec4i *v0, vec4i *v1) {
    return (vec4i *)_vec4i_max((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec4i *vec4i_min(vec4i *result, vec4i *v0, vec4i *v1) {
    return (vec4i *)_vec4i_min((mint_t *)result, (mint_t *)v0, (mint_t *)v1);
}

vec4i *vec4i_clamp(vec4i *result, vec4i *v0, vec4i *v1, vec4i *v2) {
    return (vec4i *)_vec4i_clamp((mint_t *)result, (mint_t *)v0, (mint_t *)v1,
                                 (mint_t *)v2);
}

bool vec2_is_zero(vec2 *v0) { return _vec2_is_zero((mfloat_t *)v0); }

bool vec2_is_equal(vec2 *v0, vec2 *v1) {
    return _vec2_is_equal((mfloat_t *)v0, (mfloat_t *)v1);
}

vec2 *vec2_assign(vec2 *result, vec2 *v0) {
    return (vec2 *)_vec2_assign((mfloat_t *)result, (mfloat_t *)v0);
}

vec2 *vec2_assign_vec2i(vec2 *result, vec2i *v0) {
    return (vec2 *)_vec2_assign_vec2i((mfloat_t *)result, (mint_t *)v0);
}

vec2 *vec2_sign(vec2 *result, vec2 *v0) {
    return (vec2 *)_vec2_sign((mfloat_t *)result, (mfloat_t *)v0);
}

vec2 *vec2_add(vec2 *result, vec2 *v0, vec2 *v1) {
    return (vec2 *)_vec2_add((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec2 *vec2_add_f(vec2 *result, vec2 *v0, mfloat_t f) {
    return (vec2 *)_vec2_add_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

vec2 *vec2_subtract(vec2 *result, vec2 *v0, vec2 *v1) {
    return (vec2 *)_vec2_subtract((mfloat_t *)result, (mfloat_t *)v0,
                                  (mfloat_t *)v1);
}

vec2 *vec2_subtract_f(vec2 *result, vec2 *v0, mfloat_t f) {
    return (vec2 *)_vec2_subtract_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

vec2 *vec2_multiply(vec2 *result, vec2 *v0, vec2 *v1) {
    return (vec2 *)_vec2_multiply((mfloat_t *)result, (mfloat_t *)v0,
                                  (mfloat_t *)v1);
}

vec2 *vec2_multiply_f(vec2 *result, vec2 *v0, mfloat_t f) {
    return (vec2 *)_vec2_multiply_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

vec2 *vec2_multiply_mat2(vec2 *result, vec2 *v0, mat2 *m0) {
    return (vec2 *)_vec2_multiply_mat2((mfloat_t *)result, (mfloat_t *)v0,
                                       (mfloat_t *)m0);
}

vec2 *vec2_divide(vec2 *result, vec2 *v0, vec2 *v1) {
    return (vec2 *)_vec2_divide((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec2 *vec2_divide_f(vec2 *result, vec2 *v0, mfloat_t f) {
    return (vec2 *)_vec2_divide_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

vec2 *vec2_snap(vec2 *result, vec2 *v0, vec2 *v1) {
    return (vec2 *)_vec2_snap((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec2 *vec2_snap_f(vec2 *result, vec2 *v0, mfloat_t f) {
    return (vec2 *)_vec2_snap_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

void vec2_negative(vec2 *result, vec2 *v0) {
    MAP_UNARY_FUNCTION(-, result->v, v0->v, 2);
}

void vec2_abs(vec2 *result, vec2 *v0) {
    MAP_UNARY_FUNCTION(MFABS, result->v, v0->v, 2);
}

void vec2_floor(vec2 *result, vec2 *v0) {
    MAP_UNARY_FUNCTION(MFLOOR, result->v, v0->v, 2);
}

void vec2_ceil(vec2 *result, vec2 *v0) {
    MAP_UNARY_FUNCTION(MCEIL, result->v, v0->v, 2);
}

void vec2_round(vec2 *result, vec2 *v0) {
    MAP_UNARY_FUNCTION(MROUND, result->v, v0->v, 2);
}

vec2 *vec2_max(vec2 *result, vec2 *v0, vec2 *v1) {
    return (vec2 *)_vec2_max((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec2 *vec2_min(vec2 *result, vec2 *v0, vec2 *v1) {
    return (vec2 *)_vec2_min((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec2 *vec2_clamp(vec2 *result, vec2 *v0, vec2 *v1, vec2 *v2) {
    return (vec2 *)_vec2_clamp((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1,
                               (mfloat_t *)v2);
}

vec2 *vec2_normalize(vec2 *result, vec2 *v0) {
    return (vec2 *)_vec2_normalize((mfloat_t *)result, (mfloat_t *)v0);
}

mfloat_t vec2_dot(vec2 *v0, vec2 *v1) {
    return _vec2_dot((mfloat_t *)v0, (mfloat_t *)v1);
}

vec2 *vec2_project(vec2 *result, vec2 *v0, vec2 *v1) {
    return (vec2 *)_vec2_project((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec2 *vec2_slide(vec2 *result, vec2 *v0, vec2 *normal) {
    return (vec2 *)_vec2_slide((mfloat_t *)result, (mfloat_t *)v0,
                               (mfloat_t *)normal);
}

vec2 *vec2_reflect(vec2 *result, vec2 *v0, vec2 *normal) {
    return (vec2 *)_vec2_reflect((mfloat_t *)result, (mfloat_t *)v0,
                                 (mfloat_t *)normal);
}

vec2 *vec2_tangent(vec2 *result, vec2 *v0) {
    return (vec2 *)_vec2_tangent((mfloat_t *)result, (mfloat_t *)v0);
}

vec2 *vec2_rotate(vec2 *result, vec2 *v0, mfloat_t f) {
    return (vec2 *)_vec2_rotate((mfloat_t *)result, (mfloat_t *)v0, f);
}

vec2 *vec2_lerp(vec2 *result, vec2 *v0, vec2 *v1, mfloat_t f) {
    return (vec2 *)_vec2_lerp((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1, f);
}

vec2 *vec2_bezier3(vec2 *result, vec2 *v0, vec2 *v1, vec2 *v2, mfloat_t f) {
    return (vec2 *)_vec2_bezier3((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1,
                                 (mfloat_t *)v2, f);
}

vec2 *vec2_bezier4(vec2 *result, vec2 *v0, vec2 *v1, vec2 *v2, vec2 *v3,
                   mfloat_t f) {
    return (vec2 *)_vec2_bezier4((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1,
                                 (mfloat_t *)v2, (mfloat_t *)v3, f);
}

mfloat_t vec2_angle(vec2 *v0) { return _vec2_angle((mfloat_t *)v0); }

mfloat_t vec2_length(vec2 *v0) { return hypot(v0->x, v0->y); }

mfloat_t vec2_length_squared(vec2 *v0) { return vec2_dot(v0, v0); }

mfloat_t vec2_distance(vec2 *v0, vec2 *v1) {
    return MSQRT(vec2_distance_squared(v0, v1));
}

mfloat_t vec2_distance_squared(vec2 *v0, vec2 *v1) {
    return _vec2_distance_squared((mfloat_t *)v0, (mfloat_t *)v1);
}

bool vec3_is_zero(vec3 *v0) { return _vec3_is_zero((mfloat_t *)v0); }

bool vec3_is_equal(vec3 *v0, vec3 *v1) {
    return _vec3_is_equal((mfloat_t *)v0, (mfloat_t *)v1);
}

vec3 *vec3_assign(vec3 *result, vec3 *v0) {
    return (vec3 *)_vec3_assign((mfloat_t *)result, (mfloat_t *)v0);
}

vec3 *vec3_assign_vec3i(vec3 *result, vec3i *v0) {
    return (vec3 *)_vec3_assign_vec3i((mfloat_t *)result, (mint_t *)v0);
}

vec3 *vec3_sign(vec3 *result, vec3 *v0) {
    return (vec3 *)_vec3_sign((mfloat_t *)result, (mfloat_t *)v0);
}

vec3 *vec3_add(vec3 *result, const vec3 *v0, const vec3 *v1) {
    return (vec3 *)_vec3_add((mfloat_t *)result, (const mfloat_t *)v0,
                             (const mfloat_t *)v1);
}

void vec3_add_f(vec3 *result, const vec3 *v0, mfloat_t f) {
    MAP_UNARY_FUNCTION(f +, result->v, v0->v, 3);
}

vec3 *vec3_subtract(vec3 *result, const vec3 *v0, const vec3 *v1) {
    return (vec3 *)_vec3_subtract((mfloat_t *)result, (const mfloat_t *)v0,
                                  (const mfloat_t *)v1);
}

vec3 *vec3_subtract_f(vec3 *result, vec3 *v0, mfloat_t f) {
    return (vec3 *)_vec3_subtract_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

vec3 *vec3_multiply(vec3 *result, vec3 *v0, vec3 *v1) {
    return (vec3 *)_vec3_multiply((mfloat_t *)result, (mfloat_t *)v0,
                                  (mfloat_t *)v1);
}

void vec3_multiply_f(vec3 *result, const vec3 *v0, mfloat_t f) {
    MAP_UNARY_FUNCTION(f *, result->v, v0->v, 3);
}

vec3 *vec3_multiply_mat3(vec3 *result, const vec3 *v0, const mat3 *m0) {
    return (vec3 *)_vec3_multiply_mat3((mfloat_t *)result, (const mfloat_t *)v0,
                                       (const mfloat_t *)m0);
}

vec3 *vec3_divide(vec3 *result, vec3 *v0, vec3 *v1) {
    return (vec3 *)_vec3_divide((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

void vec3_divide_f(vec3 *result, vec3 *v0, mfloat_t f) {
    _vec3_divide_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

vec3 *vec3_snap(vec3 *result, vec3 *v0, vec3 *v1) {
    return (vec3 *)_vec3_snap((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec3 *vec3_snap_f(vec3 *result, vec3 *v0, mfloat_t f) {
    return (vec3 *)_vec3_snap_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

void vec3_negative(vec3 *result, const vec3 *v0) {
    MAP_UNARY_FUNCTION(-, result->v, v0->v, 3);
}

void vec3_abs(vec3 *result, const vec3 *v0) {
    MAP_UNARY_FUNCTION(fabs, result->v, v0->v, 3);
}

void vec3_floor(vec3 *result, vec3 *v0) {
    MAP_UNARY_FUNCTION(MFLOOR, result->v, v0->v, 3);
}

void vec3_ceil(vec3 *result, vec3 *v0) {
    MAP_UNARY_FUNCTION(MCEIL, result->v, v0->v, 3);
}

void vec3_round(vec3 *result, vec3 *v0) {
    MAP_UNARY_FUNCTION(MROUND, result->v, v0->v, 3);
}

vec3 *vec3_max(vec3 *result, vec3 *v0, vec3 *v1) {
    return (vec3 *)_vec3_max((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec3 *vec3_min(vec3 *result, vec3 *v0, vec3 *v1) {
    return (vec3 *)_vec3_min((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec3 *vec3_clamp(vec3 *result, vec3 *v0, vec3 *v1, vec3 *v2) {
    return (vec3 *)_vec3_clamp((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1,
                               (mfloat_t *)v2);
}

vec3 *vec3_cross(vec3 *result, const vec3 *v0, const vec3 *v1) {
    return (vec3 *)_vec3_cross((mfloat_t *)result, (const mfloat_t *)v0,
                               (const mfloat_t *)v1);
}

vec3 *vec3_normalize(vec3 *result, const vec3 *v0) {
    return (vec3 *)_vec3_normalize((mfloat_t *)result, (const mfloat_t *)v0);
}

mfloat_t vec3_dot(const vec3 *v0, const vec3 *v1) {
    return _vec3_dot((const mfloat_t *)v0, (const mfloat_t *)v1);
}

vec3 *vec3_project(vec3 *result, vec3 *v0, vec3 *v1) {
    return (vec3 *)_vec3_project((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec3 *vec3_slide(vec3 *result, vec3 *v0, vec3 *normal) {
    return (vec3 *)_vec3_slide((mfloat_t *)result, (mfloat_t *)v0,
                               (mfloat_t *)normal);
}

vec3 *vec3_reflect(vec3 *result, vec3 *v0, vec3 *normal) {
    return (vec3 *)_vec3_reflect((mfloat_t *)result, (mfloat_t *)v0,
                                 (mfloat_t *)normal);
}

vec3 *vec3_rotate(vec3 *result, vec3 *v0, vec3 *ra, mfloat_t f) {
    return (vec3 *)_vec3_lerp((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)ra, f);
}

vec3 *vec3_lerp(vec3 *result, vec3 *v0, vec3 *v1, mfloat_t f) {
    return (vec3 *)_vec3_lerp((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1, f);
}

vec3 *vec3_bezier3(vec3 *result, vec3 *v0, vec3 *v1, vec3 *v2, mfloat_t f) {
    return (vec3 *)_vec3_bezier3((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1,
                                 (mfloat_t *)v2, f);
}

vec3 *vec3_bezier4(vec3 *result, vec3 *v0, vec3 *v1, vec3 *v2, vec3 *v3,
                   mfloat_t f) {
    return (vec3 *)_vec3_bezier4((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1,
                                 (mfloat_t *)v2, (mfloat_t *)v3, f);
}

mfloat_t vec3_length(const vec3 *v0) { return sqrt(vec3_length_squared(v0)); }

mfloat_t vec3_length_squared(const vec3 *v0) { return vec3_dot(v0, v0); }

mfloat_t vec3_distance(const vec3 *v0, const vec3 *v1) {
    return sqrt(vec3_distance_squared(v0, v1));
}

mfloat_t vec3_distance_squared(const vec3 *v0, const vec3 *v1) {
    vec3 d;
    vec3_subtract(&d, v0, v1);
    return vec3_length_squared(&d);
}

bool vec4_is_zero(vec4 *v0) { return _vec4_is_zero((mfloat_t *)v0); }

bool vec4_is_equal(vec4 *v0, vec4 *v1) {
    return _vec4_is_equal((mfloat_t *)v0, (mfloat_t *)v1);
}

vec4 *vec4_assign(vec4 *result, vec4 *v0) {
    return (vec4 *)_vec4_assign((mfloat_t *)result, (mfloat_t *)v0);
}

vec4 *vec4_assign_vec4i(vec4 *result, vec4i *v0) {
    return (vec4 *)_vec4_assign_vec4i((mfloat_t *)result, (mint_t *)v0);
}

vec4 *vec4_sign(vec4 *result, vec4 *v0) {
    return (vec4 *)_vec4_sign((mfloat_t *)result, (mfloat_t *)v0);
}

vec4 *vec4_add(vec4 *result, vec4 *v0, vec4 *v1) {
    return (vec4 *)_vec4_add((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

void vec4_add_f(vec4 *result, vec4 *v0, mfloat_t f) {
    MAP_UNARY_FUNCTION(f +, result->v, v0->v, 4);
}

vec4 *vec4_subtract(vec4 *result, vec4 *v0, vec4 *v1) {
    return (vec4 *)_vec4_subtract((mfloat_t *)result, (mfloat_t *)v0,
                                  (mfloat_t *)v1);
}

vec4 *vec4_subtract_f(vec4 *result, vec4 *v0, mfloat_t f) {
    return (vec4 *)_vec4_subtract_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

vec4 *vec4_multiply(vec4 *result, vec4 *v0, vec4 *v1) {
    return (vec4 *)_vec4_multiply((mfloat_t *)result, (mfloat_t *)v0,
                                  (mfloat_t *)v1);
}

void vec4_multiply_f(vec4 *result, vec4 *v0, mfloat_t f) {
    MAP_UNARY_FUNCTION(f *, result->v, v0->v, 4);
}

vec4 *vec4_multiply_mat4(vec4 *result, vec4 *v0, mat4 *m0) {
    return (vec4 *)_vec4_multiply_mat4((mfloat_t *)result, (mfloat_t *)v0,
                                       (mfloat_t *)m0);
}

vec4 *vec4_divide(vec4 *result, vec4 *v0, vec4 *v1) {
    return (vec4 *)_vec4_divide((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec4 *vec4_divide_f(vec4 *result, vec4 *v0, mfloat_t f) {
    return (vec4 *)_vec4_divide_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

vec4 *vec4_snap(vec4 *result, vec4 *v0, vec4 *v1) {
    return (vec4 *)_vec4_snap((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec4 *vec4_snap_f(vec4 *result, vec4 *v0, mfloat_t f) {
    return (vec4 *)_vec4_snap_f((mfloat_t *)result, (mfloat_t *)v0, f);
}

void vec4_negative(vec4 *result, vec4 *v0) {
    MAP_UNARY_FUNCTION(-, result->v, v0->v, 4);
}

void vec4_abs(vec4 *result, vec4 *v0) {
    MAP_UNARY_FUNCTION(MFABS, result->v, v0->v, 4);
}

void vec4_floor(vec4 *result, vec4 *v0) {
    MAP_UNARY_FUNCTION(MFLOOR, result->v, v0->v, 4);
}

void vec4_ceil(vec4 *result, vec4 *v0) {
    MAP_UNARY_FUNCTION(MCEIL, result->v, v0->v, 4);
}

void vec4_round(vec4 *result, vec4 *v0) {
    MAP_UNARY_FUNCTION(MROUND, result->v, v0->v, 4);
}

vec4 *vec4_max(vec4 *result, vec4 *v0, vec4 *v1) {
    return (vec4 *)_vec4_max((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec4 *vec4_min(vec4 *result, vec4 *v0, vec4 *v1) {
    return (vec4 *)_vec4_min((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1);
}

vec4 *vec4_clamp(vec4 *result, vec4 *v0, vec4 *v1, vec4 *v2) {
    return (vec4 *)_vec4_clamp((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1,
                               (mfloat_t *)v2);
}

vec4 *vec4_normalize(vec4 *result, vec4 *v0) {
    return (vec4 *)_vec4_normalize((mfloat_t *)result, (mfloat_t *)v0);
}

vec4 *vec4_lerp(vec4 *result, vec4 *v0, vec4 *v1, mfloat_t f) {
    return (vec4 *)_vec4_lerp((mfloat_t *)result, (mfloat_t *)v0, (mfloat_t *)v1, f);
}

bool quat_is_zero(quat *q0) { return _quat_is_zero((mfloat_t *)q0); }

bool quat_is_equal(quat *q0, quat *q1) {
    return _quat_is_equal((mfloat_t *)q0, (mfloat_t *)q1);
}

quat *quat_assign(quat *result, quat *q0) {
    return (quat *)_quat_assign((mfloat_t *)result, (mfloat_t *)q0);
}

quat *quat_multiply(quat *result, const quat *q0, const quat *q1) {
    result->v[0] = q0->v[3] * q1->v[0] + q0->v[0] * q1->v[3] + q0->v[1] * q1->v[2] -
                   q0->v[2] * q1->v[1];
    result->v[1] = q0->v[3] * q1->v[1] + q0->v[1] * q1->v[3] + q0->v[2] * q1->v[0] -
                   q0->v[0] * q1->v[2];
    result->v[2] = q0->v[3] * q1->v[2] + q0->v[2] * q1->v[3] + q0->v[0] * q1->v[1] -
                   q0->v[1] * q1->v[0];
    result->v[3] = q0->v[3] * q1->v[3] - q0->v[0] * q1->v[0] - q0->v[1] * q1->v[1] -
                   q0->v[2] * q1->v[2];
    quat_normalize(result, result);
    return result;
}

quat *quat_multiply_f(quat *result, quat *q0, mfloat_t f) {
    return (quat *)_quat_multiply_f((mfloat_t *)result, (mfloat_t *)q0, f);
}

quat *quat_divide(quat *result, quat *q0, quat *q1) {
    return (quat *)_quat_divide((mfloat_t *)result, (mfloat_t *)q0, (mfloat_t *)q1);
}

quat *quat_divide_f(quat *result, quat *q0, mfloat_t f) {
    return (quat *)_quat_divide_f((mfloat_t *)result, (mfloat_t *)q0, f);
}

quat *quat_negative(quat *result, quat *q0) {
    return (quat *)_quat_negative((mfloat_t *)result, (mfloat_t *)q0);
}

quat *quat_conjugate(quat *result, quat *q0) {
    return (quat *)_quat_conjugate((mfloat_t *)result, (mfloat_t *)q0);
}

quat *quat_inverse(quat *result, quat *q0) {
    return (quat *)_quat_inverse((mfloat_t *)result, (mfloat_t *)q0);
}

quat *quat_normalize(quat *result, quat *q0) {
    return (quat *)_quat_normalize((mfloat_t *)result, (mfloat_t *)q0);
}

mfloat_t quat_dot(quat *q0, quat *q1) {
    return _quat_dot((mfloat_t *)q0, (mfloat_t *)q1);
}

quat *quat_power(quat *result, quat *q0, mfloat_t exponent) {
    return (quat *)_quat_power((mfloat_t *)result, (mfloat_t *)q0, exponent);
}

quat *quat_from_axis_angle(quat *result, const vec3 *v0, mfloat_t angle) {
    if (angle == 0.0) {
        *result = quat_identity;
        return result;
    }

    vec3 vn;
    vec3_normalize(&vn, v0);

    return (quat *)_quat_from_axis_angle((mfloat_t *)result, (const mfloat_t *)&vn,
                                         angle);
}

quat *quat_from_vec3(quat *result, const vec3 *v0, const vec3 *v1) {
    return (quat *)_quat_from_vec3((mfloat_t *)result, (const mfloat_t *)v0,
                                   (const mfloat_t *)v1);
}

quat *quat_from_mat4(quat *result, mat4 *m0) {
    return (quat *)_quat_from_mat4((mfloat_t *)result, (mfloat_t *)m0);
}

quat *quat_lerp(quat *result, quat *q0, quat *q1, mfloat_t f) {
    return (quat *)_quat_lerp((mfloat_t *)result, (mfloat_t *)q0, (mfloat_t *)q1, f);
}

quat *quat_slerp(quat *result, quat *q0, quat *q1, mfloat_t f) {
    return (quat *)_quat_slerp((mfloat_t *)result, (mfloat_t *)q0, (mfloat_t *)q1,
                               f);
}

mfloat_t quat_length(quat *q0) { return sqrt(quat_length_squared(q0)); }

mfloat_t quat_length_squared(quat *q0) {
    return quat_dot(q0, q0);
}

mfloat_t quat_angle(quat *q0, quat *q1) {
    return _quat_angle((mfloat_t *)q0, (mfloat_t *)q1);
}

mfloat_t mat2_determinant(mat2 *m0) { return _mat2_determinant((mfloat_t *)m0); }

mat2 *mat2_assign(mat2 *result, mat2 *m0) {
    return (mat2 *)_mat2_assign((mfloat_t *)result, (mfloat_t *)m0);
}

void mat2_negative(mat2 *result, mat2 *m0) {
    MAP_UNARY_FUNCTION(-, result->v, m0->v, MAT2_SIZE);
}

mat2 *mat2_transpose(mat2 *result, mat2 *m0) {
    return (mat2 *)_mat2_transpose((mfloat_t *)result, (mfloat_t *)m0);
}

mat2 *mat2_cofactor(mat2 *result, mat2 *m0) {
    return (mat2 *)_mat2_cofactor((mfloat_t *)result, (mfloat_t *)m0);
}

mat2 *mat2_adjugate(mat2 *result, mat2 *m0) {
    return (mat2 *)_mat2_adjugate((mfloat_t *)result, (mfloat_t *)m0);
}

mat2 *mat2_multiply(mat2 *result, mat2 *m0, mat2 *m1) {
    return (mat2 *)_mat2_multiply((mfloat_t *)result, (mfloat_t *)m0,
                                  (mfloat_t *)m1);
}

void mat2_multiply_f(mat2 *result, mat2 *m0, mfloat_t f) {
    MAP_UNARY_FUNCTION(f *, result->v, m0->v, MAT2_SIZE);
}

mat2 *mat2_inverse(mat2 *result, mat2 *m0) {
    return (mat2 *)_mat2_inverse((mfloat_t *)result, (mfloat_t *)m0);
}

mat2 *mat2_scaling(mat2 *result, vec2 *v0) {
    return (mat2 *)_mat2_scaling((mfloat_t *)result, (mfloat_t *)v0);
}

mat2 *mat2_scale(mat2 *result, mat2 *m0, vec2 *v0) {
    return (mat2 *)_mat2_scale((mfloat_t *)result, (mfloat_t *)m0, (mfloat_t *)v0);
}

mat2 *mat2_rotation_z(mat2 *result, mfloat_t f) {
    return (mat2 *)_mat2_rotation_z((mfloat_t *)result, f);
}

mat2 *mat2_lerp(mat2 *result, mat2 *m0, mat2 *m1, mfloat_t f) {
    return (mat2 *)_mat2_lerp((mfloat_t *)result, (mfloat_t *)m0, (mfloat_t *)m1, f);
}

mfloat_t mat3_determinant(mat3 *m0) { return _mat3_determinant((mfloat_t *)m0); }

mat3 *mat3_assign(mat3 *result, mat3 *m0) {
    return (mat3 *)_mat3_assign((mfloat_t *)result, (mfloat_t *)m0);
}

void mat3_negative(mat3 *result, mat3 *m0) {
    MAP_UNARY_FUNCTION(-, result->v, m0->v, MAT3_SIZE);
}

mat3 *mat3_transpose(mat3 *result, mat3 *m0) {
    return (mat3 *)_mat3_transpose((mfloat_t *)result, (mfloat_t *)m0);
}

mat3 *mat3_cofactor(mat3 *result, mat3 *m0) {
    return (mat3 *)_mat3_cofactor((mfloat_t *)result, (mfloat_t *)m0);
}

mat3 *mat3_multiply(mat3 *result, mat3 *m0, mat3 *m1) {
    return (mat3 *)_mat3_multiply((mfloat_t *)result, (mfloat_t *)m0,
                                  (mfloat_t *)m1);
}

void mat3_multiply_f(mat3 *result, const mat3 *m0, mfloat_t f) {
    MAP_UNARY_FUNCTION(f *, result->v, m0->v, MAT3_SIZE);
}

mat3 *mat3_inverse(mat3 *result, const mat3 *m0) {
    mat4 m1 = mat4_identity;
    m1.m11 = m0->m11;
    m1.m12 = m0->m12;
    m1.m13 = m0->m13;

    m1.m21 = m0->m21;
    m1.m22 = m0->m22;
    m1.m23 = m0->m23;

    m1.m31 = m0->m31;
    m1.m32 = m0->m32;
    m1.m33 = m0->m33;

    mat4_inverse(&m1, &m1);

    result->m11 = m1.m11;
    result->m12 = m1.m12;
    result->m13 = m1.m13;

    result->m21 = m1.m21;
    result->m22 = m1.m22;
    result->m23 = m1.m23;

    result->m31 = m1.m31;
    result->m32 = m1.m32;
    result->m33 = m1.m33;

    return result;
}

mat3 *mat3_scaling(mat3 *result, vec3 *v0) {
    return (mat3 *)_mat3_scaling((mfloat_t *)result, (mfloat_t *)v0);
}

mat3 *mat3_scale(mat3 *result, const mat3 *m0, const vec3 *v0) {
    return (mat3 *)_mat3_scale((mfloat_t *)result, (const mfloat_t *)m0,
                               (const mfloat_t *)v0);
}

mat3 *mat3_rotation_x(mat3 *result, mfloat_t f) {
    return (mat3 *)_mat3_rotation_x((mfloat_t *)result, f);
}

mat3 *mat3_rotation_y(mat3 *result, mfloat_t f) {
    return (mat3 *)_mat3_rotation_y((mfloat_t *)result, f);
}

mat3 *mat3_rotation_z(mat3 *result, mfloat_t f) {
    return (mat3 *)_mat3_rotation_z((mfloat_t *)result, f);
}

mat3 *mat3_rotation_axis(mat3 *result, vec3 *v0, mfloat_t f) {
    return (mat3 *)_mat3_rotation_axis((mfloat_t *)result, (mfloat_t *)v0, f);
}

mat3 *mat3_rotation_quat(mat3 *result, const quat *q0) {
    return (mat3 *)_mat3_rotation_quat((mfloat_t *)result, (const mfloat_t *)q0);
}

mat3 *mat3_lerp(mat3 *result, mat3 *m0, mat3 *m1, mfloat_t f) {
    return (mat3 *)_mat3_lerp((mfloat_t *)result, (mfloat_t *)m0, (mfloat_t *)m1, f);
}

mfloat_t mat4_determinant(mat4 *m0) { return _mat4_determinant((mfloat_t *)m0); }

mat4 *mat4_assign(mat4 *result, mat4 *m0) {
    return (mat4 *)_mat4_assign((mfloat_t *)result, (mfloat_t *)m0);
}

void mat4_negative(mat4 *result, mat4 *m0) {
    MAP_UNARY_FUNCTION(-, result->v, m0->v, MAT4_SIZE);
}

mat4 *mat4_transpose(mat4 *result, mat4 *m0) {
    return (mat4 *)_mat4_transpose((mfloat_t *)result, (mfloat_t *)m0);
}

mat4 *mat4_cofactor(mat4 *result, mat4 *m0) {
    return (mat4 *)_mat4_cofactor((mfloat_t *)result, (mfloat_t *)m0);
}

mat4 *mat4_rotation_x(mat4 *result, mfloat_t f) {
    return (mat4 *)_mat4_rotation_x((mfloat_t *)result, f);
}

mat4 *mat4_rotation_y(mat4 *result, mfloat_t f) {
    return (mat4 *)_mat4_rotation_y((mfloat_t *)result, f);
}

mat4 *mat4_rotation_z(mat4 *result, mfloat_t f) {
    return (mat4 *)_mat4_rotation_z((mfloat_t *)result, f);
}

mat4 *mat4_rotation_axis(mat4 *result, vec3 *v0, mfloat_t f) {
    return (mat4 *)_mat4_rotation_axis((mfloat_t *)result, (mfloat_t *)v0, f);
}

mat4 *mat4_rotation_quat(mat4 *result, const quat *q0) {
    return (mat4 *)_mat4_rotation_quat((mfloat_t *)result, (const mfloat_t *)q0);
}

mat4 *mat4_translation(mat4 *result, const mat4 *m0, const vec3 *v0) {
    return (mat4 *)_mat4_translation((mfloat_t *)result, (const mfloat_t *)m0,
                                     (const mfloat_t *)v0);
}

mat4 *mat4_translate(mat4 *result, mat4 *m0, vec3 *v0) {
    return (mat4 *)_mat4_translate((mfloat_t *)result, (mfloat_t *)m0,
                                   (mfloat_t *)v0);
}

mat4 *mat4_scaling(mat4 *result, mat4 *m0, vec3 *v0) {
    return (mat4 *)_mat4_scaling((mfloat_t *)result, (mfloat_t *)m0, (mfloat_t *)v0);
}

mat4 *mat4_scale(mat4 *result, mat4 *m0, vec3 *v0) {
    return (mat4 *)_mat4_scale((mfloat_t *)result, (mfloat_t *)m0, (mfloat_t *)v0);
}

mat4 *mat4_multiply(mat4 *result, mat4 *m0, mat4 *m1) {
    return (mat4 *)_mat4_multiply((mfloat_t *)result, (mfloat_t *)m0,
                                  (mfloat_t *)m1);
}

void mat4_multiply_f(mat4 *result, const mat4 *m0, mfloat_t f) {
    MAP_UNARY_FUNCTION(f *, result->v, m0->v, MAT4_SIZE);
}

mat4 *mat4_inverse(mat4 *result, mat4 *m0) {
    return (mat4 *)_mat4_inverse((mfloat_t *)result, (mfloat_t *)m0);
}

mat4 *mat4_lerp(mat4 *result, mat4 *m0, mat4 *m1, mfloat_t f) {
    return (mat4 *)_mat4_lerp((mfloat_t *)result, (mfloat_t *)m0, (mfloat_t *)m1, f);
}

mat4 *mat4_look_at(mat4 *result, vec3 *position, vec3 *target, vec3 *up) {
    return (mat4 *)_mat4_look_at((mfloat_t *)result, (mfloat_t *)position,
                                 (mfloat_t *)target, (mfloat_t *)up);
}

mat4 *mat4_ortho(mat4 *result, mfloat_t l, mfloat_t r, mfloat_t b, mfloat_t t,
                 mfloat_t n, mfloat_t f) {
    return (mat4 *)_mat4_ortho((mfloat_t *)result, l, r, b, t, n, f);
}

mat4 *mat4_perspective(mat4 *result, mfloat_t fov_y, mfloat_t aspect, mfloat_t n,
                       mfloat_t f) {
    return (mat4 *)_mat4_perspective((mfloat_t *)result, fov_y, aspect, n, f);
}

mat4 *mat4_perspective_fov(mat4 *result, mfloat_t fov, mfloat_t w, mfloat_t h,
                           mfloat_t n, mfloat_t f) {
    return (mat4 *)_mat4_perspective_fov((mfloat_t *)result, fov, w, h, n, f);
}

mat4 *mat4_perspective_infinite(mat4 *result, mfloat_t fov_y, mfloat_t aspect,
                                mfloat_t n) {
    return (mat4 *)_mat4_perspective_infinite((mfloat_t *)result, fov_y, aspect, n);
}

mfloat_t quadratic_ease_out(mfloat_t f) { return -f * (f - MFLOAT_C(2.0)); }

mfloat_t quadratic_ease_in(mfloat_t f) { return f * f; }

mfloat_t quadratic_ease_in_out(mfloat_t f) {
    mfloat_t a = MFLOAT_C(0.0);
    if (f < MFLOAT_C(0.5)) {
        a = MFLOAT_C(2.0) * f * f;
    } else {
        a = -MFLOAT_C(2.0) * f * f + MFLOAT_C(4.0) * f - MFLOAT_C(1.0);
    }
    return a;
}

mfloat_t cubic_ease_out(mfloat_t f) {
    mfloat_t a = f - MFLOAT_C(1.0);
    return a * a * a + MFLOAT_C(1.0);
}

mfloat_t cubic_ease_in(mfloat_t f) { return f * f * f; }

mfloat_t cubic_ease_in_out(mfloat_t f) {
    mfloat_t a = MFLOAT_C(0.0);
    if (f < MFLOAT_C(0.5)) {
        a = MFLOAT_C(4.0) * f * f * f;
    } else {
        a = MFLOAT_C(2.0) * f - MFLOAT_C(2.0);
        a = MFLOAT_C(0.5) * a * a * a + MFLOAT_C(1.0);
    }
    return a;
}

mfloat_t quartic_ease_out(mfloat_t f) {
    mfloat_t a = f - MFLOAT_C(1.0);
    return a * a * a * (MFLOAT_C(1.0) - f) + MFLOAT_C(1.0);
}

mfloat_t quartic_ease_in(mfloat_t f) { return f * f * f * f; }

mfloat_t quartic_ease_in_out(mfloat_t f) {
    mfloat_t a = MFLOAT_C(0.0);
    if (f < MFLOAT_C(0.5)) {
        a = MFLOAT_C(8.0) * f * f * f * f;
    } else {
        a = f - MFLOAT_C(1.0);
        a = -MFLOAT_C(8.0) * a * a * a * a + MFLOAT_C(1.0);
    }
    return a;
}

mfloat_t quintic_ease_out(mfloat_t f) {
    mfloat_t a = f - MFLOAT_C(1.0);
    return a * a * a * a * a + MFLOAT_C(1.0);
}

mfloat_t quintic_ease_in(mfloat_t f) { return f * f * f * f * f; }

mfloat_t quintic_ease_in_out(mfloat_t f) {
    mfloat_t a = MFLOAT_C(0.0);
    if (f < MFLOAT_C(0.5)) {
        a = MFLOAT_C(16.0) * f * f * f * f * f;
    } else {
        a = MFLOAT_C(2.0) * f - MFLOAT_C(2.0);
        a = MFLOAT_C(0.5) * a * a * a * a * a + MFLOAT_C(1.0);
    }
    return a;
}

mfloat_t sine_ease_out(mfloat_t f) { return MSIN(f * MPI_2); }

mfloat_t sine_ease_in(mfloat_t f) {
    return MSIN((f - MFLOAT_C(1.0)) * MPI_2) + MFLOAT_C(1.0);
}

mfloat_t sine_ease_in_out(mfloat_t f) {
    return MFLOAT_C(0.5) * (MFLOAT_C(1.0) - MCOS(f * MPI));
}

mfloat_t circular_ease_out(mfloat_t f) { return MSQRT((MFLOAT_C(2.0) - f) * f); }

mfloat_t circular_ease_in(mfloat_t f) {
    return MFLOAT_C(1.0) - MSQRT(MFLOAT_C(1.0) - (f * f));
}

mfloat_t circular_ease_in_out(mfloat_t f) {
    mfloat_t a = MFLOAT_C(0.0);
    if (f < MFLOAT_C(0.5)) {
        a = MFLOAT_C(0.5) *
            (MFLOAT_C(1.0) - MSQRT(MFLOAT_C(1.0) - MFLOAT_C(4.0) * f * f));
    } else {
        a = MFLOAT_C(0.5) * (MSQRT(-(MFLOAT_C(2.0) * f - MFLOAT_C(3.0)) *
                                   (MFLOAT_C(2.0) * f - MFLOAT_C(1.0))) +
                             MFLOAT_C(1.0));
    }
    return a;
}

mfloat_t exponential_ease_out(mfloat_t f) {
    mfloat_t a = f;
    if (MFABS(a) > MFLT_EPSILON) {
        a = MFLOAT_C(1.0) - MPOW(MFLOAT_C(2.0), -MFLOAT_C(10.0) * f);
    }
    return a;
}

mfloat_t exponential_ease_in(mfloat_t f) {
    mfloat_t a = f;
    if (MFABS(a) > MFLT_EPSILON) {
        a = MPOW(MFLOAT_C(2.0), MFLOAT_C(10.0) * (f - MFLOAT_C(1.0)));
    }
    return a;
}

mfloat_t exponential_ease_in_out(mfloat_t f) {
    mfloat_t a = f;
    if (f < MFLOAT_C(0.5)) {
        a = MFLOAT_C(0.5) *
            MPOW(MFLOAT_C(2.0), (MFLOAT_C(20.0) * f) - MFLOAT_C(10.0));
    } else {
        a = -MFLOAT_C(0.5) *
                MPOW(MFLOAT_C(2.0), -MFLOAT_C(20.0) * f + MFLOAT_C(10.0)) +
            MFLOAT_C(1.0);
    }
    return a;
}

mfloat_t elastic_ease_out(mfloat_t f) {
    return MSIN(-MFLOAT_C(13.0) * MPI_2 * (f + MFLOAT_C(1.0))) *
               MPOW(MFLOAT_C(2.0), -MFLOAT_C(10.0) * f) +
           MFLOAT_C(1.0);
}

mfloat_t elastic_ease_in(mfloat_t f) {
    return MSIN(MFLOAT_C(13.0) * MPI_2 * f) *
           MPOW(MFLOAT_C(2.0), MFLOAT_C(10.0) * (f - MFLOAT_C(1.0)));
}

mfloat_t elastic_ease_in_out(mfloat_t f) {
    mfloat_t a = MFLOAT_C(0.0);
    if (f < MFLOAT_C(0.5)) {
        a = MFLOAT_C(0.5) * MSIN(MFLOAT_C(13.0) * MPI_2 * (MFLOAT_C(2.0) * f)) *
            MPOW(MFLOAT_C(2.0),
                 MFLOAT_C(10.0) * ((MFLOAT_C(2.0) * f) - MFLOAT_C(1.0)));
    } else {
        a = MFLOAT_C(0.5) *
            (MSIN(-MFLOAT_C(13.0) * MPI_2 *
                  ((MFLOAT_C(2.0) * f - MFLOAT_C(1.0)) + MFLOAT_C(1.0))) *
                 MPOW(MFLOAT_C(2.0),
                      -MFLOAT_C(10.0) * (MFLOAT_C(2.0) * f - MFLOAT_C(1.0))) +
             MFLOAT_C(2.0));
    }
    return a;
}

mfloat_t back_ease_out(mfloat_t f) {
    mfloat_t a = MFLOAT_C(1.0) - f;
    return MFLOAT_C(1.0) - (a * a * a - a * MSIN(a * MPI));
}

mfloat_t back_ease_in(mfloat_t f) { return f * f * f - f * MSIN(f * MPI); }

mfloat_t back_ease_in_out(mfloat_t f) {
    mfloat_t a = MFLOAT_C(0.0);
    if (f < MFLOAT_C(0.5)) {
        a = MFLOAT_C(2.0) * f;
        a = MFLOAT_C(0.5) * (a * a * a - a * MSIN(a * MPI));
    } else {
        a = (MFLOAT_C(1.0) - (MFLOAT_C(2.0) * f - MFLOAT_C(1.0)));
        a = MFLOAT_C(0.5) * (MFLOAT_C(1.0) - (a * a * a - a * MSIN(f * MPI))) +
            MFLOAT_C(0.5);
    }
    return a;
}

mfloat_t bounce_ease_out(mfloat_t f) {
    mfloat_t a = MFLOAT_C(0.0);
    if (f < MFLOAT_C(4.0) / MFLOAT_C(11.0)) {
        a = (MFLOAT_C(121.0) * f * f) / MFLOAT_C(16.0);
    } else if (f < MFLOAT_C(8.0) / MFLOAT_C(11.0)) {
        a = (MFLOAT_C(363.0) / MFLOAT_C(40.0) * f * f) -
            (MFLOAT_C(99.0) / MFLOAT_C(10.0) * f) + MFLOAT_C(17.0) / MFLOAT_C(5.0);
    } else if (f < MFLOAT_C(9.0) / MFLOAT_C(10.0)) {
        a = (MFLOAT_C(4356.0) / MFLOAT_C(361.0) * f * f) -
            (MFLOAT_C(35442.0) / MFLOAT_C(1805.0) * f) +
            MFLOAT_C(16061.0) / MFLOAT_C(1805.0);
    } else {
        a = (MFLOAT_C(54.0) / MFLOAT_C(5.0) * f * f) -
            (MFLOAT_C(513.0) / MFLOAT_C(25.0) * f) +
            MFLOAT_C(268.0) / MFLOAT_C(25.0);
    }
    return a;
}

mfloat_t bounce_ease_in(mfloat_t f) {
    return MFLOAT_C(1.0) - bounce_ease_out(MFLOAT_C(1.0) - f);
}

mfloat_t bounce_ease_in_out(mfloat_t f) {
    mfloat_t a = MFLOAT_C(0.0);
    if (f < MFLOAT_C(0.5)) {
        a = MFLOAT_C(0.5) * bounce_ease_in(f * MFLOAT_C(2.0));
    } else {
        a = MFLOAT_C(0.5) * bounce_ease_out(f * MFLOAT_C(2.0) - MFLOAT_C(1.0)) +
            MFLOAT_C(0.5);
    }
    return a;
}

vec3 *vec3_multiply_quat(vec3 *result, const vec3 *v0, const quat *q) {
    mat3 m;
    mat3_rotation_quat(&m, q);
    return vec3_multiply_mat3(result, v0, &m);
}

quat *quat_from_euler_angles(quat *result, const vec3 *e) {
    vec3 axis;
    vec3_normalize(&axis, e);
    return quat_from_axis_angle(result, &axis, vec3_length(e));
}

void vec3u_multiply_u(vec3u *result, const vec3u *x, uint32_t u) {
    MAP_UNARY_FUNCTION(u*, result->v, x->v, 3);
}
