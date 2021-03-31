//
// Created by millie on 31/03/2021.
//
#ifndef SERAPHIM_TEST_QUAT
#define SERAPHIM_TEST_QUAT

#include "maths/quat.h"

extern inline int test_quat_identity(){
    srph_quat q = srph_quat_identity;
    vec3 x;
    x.x = 1.0;
    x.y = 2.0;
    x.z = 3.0;
    vec3 x1;
    srph_quat_rotate(&q, &x1, &x);

    srph_vec3_print(&x);
    printf("\n");
    srph_vec3_print(&x1);
    return 1;
}

#endif