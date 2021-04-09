//
// Created by millie on 05/04/2021.
//

#ifndef SERAPHIM_TEST_SDF_H
#define SERAPHIM_TEST_SDF_H

#include "maths/sdf/sdf.h"
#include "maths/sdf/platonic.h"

extern inline const char * test_sdf_inertia_tensor_cube(){
    double r = 0.5;
    vec3 cube_size = {{r, r, r}};
    srph_sdf * cube_sdf = srph_sdf_cuboid_create(&cube_size);

    mat3 * i = srph_sdf_inertia_tensor(cube_sdf);

    // TODO: improve this such that a lower tolerance can be used
    double tolerance = 0.01;

    for (int j = 0; j < 9; j++){
        if (j / 3 == j % 3){
            double expected = (r * 2) * (r * 2) / 6;
            TEST_ASSERT(
                fabs(i->v[j] - expected) < tolerance,
                "incorrect diagonal elements to inertia tensor"
            );
        } else {
            TEST_ASSERT(fabs(i->v[j]) < tolerance, "non-diagonal elements of cube inertia tensor should be zero");
        }
    }

    srph_sdf_destroy(cube_sdf);
    return TEST_SUCCESS;
}

#endif //SERAPHIM_TEST_SDF_H
