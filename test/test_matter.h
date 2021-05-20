//
// Created by millie on 06/04/2021.
//

#ifndef SERAPHIM_TEST_MATTER_H
#define SERAPHIM_TEST_MATTER_H

#include "../backend/sdf.h"
#include "../common/matter.h"
#include "../backend/platonic.h"

extern inline const char * test_matter_inertia_tensor_uniform_cube(){
    double r = 0.5;
    vec3 cube_size = {{r, r, r}};
    srph_sdf * cube_sdf = srph_sdf_cuboid_create(&cube_size);

    srph_material material;
    material.static_friction = 0.2;
    material.dynamic_friction = 0.1;
    material.density = 700.0;
    material.restitution = 0.5;

    srph_matter m;
    srph_matter_init(&m, cube_sdf, &material, NULL, true, false);
    mat3 * i = srph_matter_inertia_tensor(&m);

    // TODO: improve this such that a lower tolerance can be used
    double tolerance = 0.01;

    for (int j = 0; j < 9; j++){
        if (j / 3 == j % 3){
            double expected = material.density * (r * 2) * (r * 2) / 6;
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


#endif //SERAPHIM_TEST_MATTER_H
