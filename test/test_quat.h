//
// Created by millie on 31/03/2021.
//
#ifndef SERAPHIM_TEST_QUAT
#define SERAPHIM_TEST_QUAT

#include "../common/maths.h"
#include "test_header.h"

extern inline const char * test_quat_identity(){
    quat q = quat_identity;
    vec3 x;
    x.x = 1.0;
    x.y = 2.0;
    x.z = 3.0;
    vec3 m;
    vec3_multiply_quat(&m, &x, &q);

    TEST_ASSERT(x.x == m.x && x.y == m.y && x.z == m.z, "vectors not equal after rotation by identity");
    return TEST_SUCCESS;
}

extern inline const char * test_quat_to_matrix_identity(){
    mat3 m;
    mat3_rotation_quat(&m, &quat_identity);
    for (int i = 0; i < 9; i++){
        TEST_ASSERT(m.v[i] == mat3_identity.v[i], "elements not equal");
    }
    return TEST_SUCCESS;
}

extern inline const char * test_quat_from_angle_axis(){
    vec3 axis = {{1.0, 2.0, 3.0}};
    double angle = 2;
    quat q;
    quat_from_axis_angle(&q, &axis, angle);

    double expected[4] = { 0.2248926, 0.4497852, 0.6746777, 0.5403023 };
    for (int i = 0; i < 4; i++){
        TEST_ASSERT(fabs(q.v[i] - expected[i]) < 0.00001, "elements not equal");
    }
    return TEST_SUCCESS;
}


extern inline const char * test_quat_to_mat3(){
    quat q = {{
        0.2248926, 0.4497852, 0.6746777, 0.5403023
    }};

    mat3 m;
    mat3_rotation_quat(&m, &q);
    mat3 m_expected = {{
        -0.3149935, 0.9313666,-0.1825799,
        -0.5267532, -0.0115335, 0.8499401,
        0.7894999, 0.3639001,  0.4942333
    }};

    for (int i = 0; i < 9; i++){
        TEST_ASSERT(fabs(m.v[i] - m_expected.v[i]) < 0.00001, "elements not equal");
    }

    return TEST_SUCCESS;
}

extern inline const char * test_quat_to_mat4(){
    quat q = {{
          0.2248926, 0.4497852, 0.6746777, 0.5403023
    }};

    mat4 m;
    mat4_rotation_quat(&m, &q);
    mat4 m_expected = {{
           -0.3149935, 0.9313666,-0.1825799, 0.0,
           -0.5267532, -0.0115335, 0.8499401, 0.0,
           0.7894999, 0.3639001,  0.4942333, 0.0,
           0.0, 0.0, 0.0, 1.0
    }};

    for (int i = 0; i < 16; i++){
        TEST_ASSERT(fabs(m.v[i] - m_expected.v[i]) < 0.00001, "elements not equal");
    }

    return TEST_SUCCESS;
}

extern inline const char * test_quat_to_matrix_and_back(){
    quat q = {{
        2.0, -3.4, 0.1, 1.1
    }};
    quat_normalize(&q, &q);
    mat4 m;
    mat4_rotation_quat(&m, &q);

    quat q1;
    quat_from_mat4(&q1, &m);

    for (int i = 0; i < 4; i++){
        TEST_ASSERT(fabs(q.v[i] - q1.v[i] < 0.00001), "not eq");
    }

    return TEST_SUCCESS;
}

extern inline const char * test_quat_multiply(){
    quat q1 = {{
       0.30261376633440124, 0.60522753266880247,-0.72627303920256292, 0.12104550653376051
    }};

    quat q2 = {{
       -0.51306907467650797, 0.11840055569457875,0.31573481518554336 ,   0.78933703796385835
    }};

    quat expected = {{
         0.45384,0.76914,-0.1887,0.40846
    }};

    quat actual;
    quat_multiply(&actual, &q1, &q2);

    for (int i = 0; i < 4; i++){
        TEST_ASSERT(fabs(actual.v[i] - expected.v[i]) < 0.0001, "not eq");
    }

    return TEST_SUCCESS;
}

extern inline const char * test_quat_inverse_mult() {
    quat q = {{
      -0.067728546147859642, 0.2031856384435789, 0.81274255377431559, -0.54182836918287713
    }};

    quat qi;
    quat_inverse(&qi, &q);

    vec3 x = {{1.0, 2.0, 3.0}};
    vec3 y;
    vec3_multiply_quat(&y, &x, &q);
    vec3_multiply_quat(&y, &y, &qi);

    TEST_ASSERT(vec3_distance(&x, &y) < 0.00001, "vectors must be same after undoing rotation");

    return TEST_SUCCESS;
}

#endif