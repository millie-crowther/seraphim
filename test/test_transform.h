//
// Created by millie on 08/04/2021.
//

#ifndef SERAPHIM_TEST_TRANSFORM_H
#define SERAPHIM_TEST_TRANSFORM_H

#include "physics/transform.h"

extern inline const char * test_transform_matrix(){
    srph_transform tf;
    tf.rotation = quat_identity;
    tf.position = {{ 1.0, 2.0, 3.0}};

    mat4 m;
    srph_transform_matrix(&tf, &m);

    mat4 expected = {{
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        1.0, 2.0, 3.0, 1.0
    }};

    for (int i = 0; i < 9; i++){
        TEST_ASSERT(m.v[i] == expected.v[i], "not eq");
    }

    return TEST_SUCCESS;
}

#endif //SERAPHIM_TEST_TRANSFORM_H
