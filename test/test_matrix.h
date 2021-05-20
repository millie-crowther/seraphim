//
// Created by millie on 05/04/2021.
//

#ifndef SERAPHIM_TEST_TEST_MATRIX_H
#define SERAPHIM_TEST_TEST_MATRIX_H

#include "test_header.h"
#include "maths/matrix.h"
#include "../common/maths.h"

extern inline const char * test_matrix_inverse_identity(){
    mat3 i;
    mat3_inverse(&i, &mat3_identity);

    for (int j = 0; j < 9; j++){
        if (i.v[j] != mat3_identity.v[j]){
            return TEST_FAILURE;
        }
    }

    return TEST_SUCCESS;
}

extern inline const char * test_matrix_inverse_scale(){
    double diagonal[3] = { 1.0, 2.0, 3.0 };
    mat3 s = mat3_identity;
    for (int i = 0; i < 9; i++){
        if (i / 3 == i % 3){
            s.v[i] = diagonal[i / 3];
        } else {
            s.v[i] = 0.0;
        }
    }

    mat3 si;
    mat3_inverse(&si, &s);

    for (int i = 0; i < 9; i++){
        if (i / 3 == i % 3){
            if (fabs(si.v[i] - 1.0 / diagonal[i / 3]) > 0.00001){
                return TEST_FAILURE;
            }
        } else {
            if (s.v[i] != 0.0){
                return TEST_FAILURE;
            }
        }
    }

    return TEST_SUCCESS;
}

extern inline const char * test_matrix_diagonal_determinant(){
    double diagonal[3] = { 1.0, 2.0, 3.0 };
    mat3 s = mat3_identity;
    for (int i = 0; i < 9; i++){
        if (i / 3 == i % 3){
            s.v[i] = diagonal[i / 3];
        } else {
            s.v[i] = 0.0;
        }
    }

    double diagonal_product = 1.0;
    for (int i = 0; i < 3; i++){
        diagonal_product *= diagonal[i];
    }

    TEST_ASSERT(
        mat3_determinant(&s) == diagonal_product,
        "determinant of diagonal matrix must be product of elements"
    );
    return TEST_SUCCESS;
}

extern inline const char * test_matrix_determinant(){
    mat3 m = {{
        1.0, 3.0, 4.0,
        -0.4, 1.3, 0.5,
        -1.0, 3.4, 2.4
    }};

    double det = mat3_determinant(&m);
    TEST_ASSERT(fabs(det - 2.56) < 0.0000001, "incorrect matrix determinant");
    return TEST_SUCCESS;
}

extern inline const char * test_matrix_apply_identity(){
    vec3 x = {{ 1.0, 2.0, 3.0 }};
    vec3 m;
    vec3_multiply_mat3(&m, &x, &mat3_identity);
    TEST_ASSERT(x.x == m.x && x.y == m.y && x.z == m.z, "vectors not equal after rotation by identity");
    return TEST_SUCCESS;
}

extern inline const char * test_matrix_apply_rotation(){
    vec3 x = {{ 1.0, 0.0, 0.0 }};
    double theta = srph::constant::pi / 2;
    mat3 r = {{
        cos(theta),  sin(theta), 0.0,
        -sin(theta), cos(theta), 0.0,
        0.0,   0.0,  1.0
    }};
    vec3 rx;
    vec3_multiply_mat3(&rx, &x, &r);
    TEST_ASSERT(fabs(rx.x) + fabs(rx.z) + fabs(rx.y - 1) < 0.0001, "incorrect rotation");
    return TEST_SUCCESS;
}

extern inline const char * test_matrix_transpose(){
    mat3 m = {{
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0,
        7.0, 8.0, 9.0
    }};

    mat3 mt = {{
        1.0, 4.0, 7.0,
        2.0, 5.0, 8.0,
        3.0, 6.0, 9.0
    }};

    mat3 mt_test;
    mat3_transpose(&mt_test, &m);

    for (int i = 0; i < 9; i++){
        if (mt.v[i] != mt_test.v[i]){
            return TEST_FAILURE;
        }
    }

    return TEST_SUCCESS;
}

extern inline const char * test_matrix_transpose_symmetric(){
    mat3 m = {{
        1.0, 2.0, 3.0,
        2.0, 3.0, 4.0,
        3.0, 4.0, 5.0
    }};

    mat3 mt;
    mat3_transpose(&mt, &m);

    for (int i = 0; i < 9; i++){
        TEST_ASSERT(m.v[i] == mt.v[i], "Symmetric matrix must be the same after transpose.");
    }
    return TEST_SUCCESS;
}

extern inline const char * test_matrix_multiply_diagonal(){
    mat3 a = {{
        1.0, 0.0, 0.0,
        0.0, 2.0, 0.0,
        0.0, 0.0, 3.0
    }};

    mat3 b = {{
        2.0, 0.0, 0.0,
        0.0, 3.0, 0.0,
        0.0, 0.0, 4.0
    }};

    mat3 ab;
    mat3_multiply(&ab, &a, &b);

    double expected_diagonal[3] = { 2.0, 6.0, 12.0 };
    for (int i = 0; i < 9; i++){
        if (i / 3 == i % 3){
            TEST_ASSERT(ab.v[i] == expected_diagonal[i / 3], "incorrect diagonal");
        } else {
            TEST_ASSERT(ab.v[i] == 0.0, "non zero non diagonal");
        }
    }
    return TEST_SUCCESS;
}

extern inline const char * test_matrix_multiply(){
    mat3 a = {{
        3, 1, 5,
        2, 4, 2,
        4, 3, 4
    }};

    mat3 b = {{
        4, 4, 2,
        8, 6, 2,
        2,  9, 3
    }};

    mat3 ab = {{
        28, 26, 36,
        44, 38, 60,
        36, 47, 40
    }};

    mat3 ab_test;

    mat3_multiply(&ab_test, &a, &b);

    for (int i = 0; i < 9; i++){
        TEST_ASSERT(ab.v[i] == ab_test.v[i], "elements not equal");
    }
    return TEST_SUCCESS;
}

extern inline const char * test_matrix_inverse_multiply_identity(){
    mat3 m = {{
        -1.0, 2.0, 5.0,
        1.6, -3.4, 1.4,
        -0.5, 1.4, -2.0
    }};

    mat3 mi;
    mat3_inverse(&mi, &m);

    mat3 i;
    mat3_multiply(&i, &mi, &m);

    for (int j = 0; j < 9; j++){
        if (j / 3 == j % 3){
            TEST_ASSERT(fabs(i.v[j] - 1) < 0.00001, "diagonal element not 1");
        } else {
            TEST_ASSERT(fabs(i.v[j]) < 0.00001, "non diagonal element not zero");
        }
    }

    return TEST_SUCCESS;
}

#endif //SERAPHIM_TEST_TEST_MATRIX_H
