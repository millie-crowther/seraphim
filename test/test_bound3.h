//
// Created by millie on 23/04/2021.
//

#ifndef SERAPHIM_TEST_TEST_BOUND3_H
#define SERAPHIM_TEST_TEST_BOUND3_H

#include "test_header.h"

extern inline const char * test_bound3_intersection(){
    srph_bound3 a, b;
    a.lower = {{ 0, 0, 0 }};
    a.upper = {{ 2, 2, 2 }};

    b.lower = {{ 1, 1, 1 }};
    b.upper = {{ 3, 3, 3 }};

    srph_bound3 intersection;
    srph_bound3_intersection(&a, &b, &intersection);

    for (int i = 0; i < 3; i++){
        TEST_ASSERT(intersection.lower.v[i] == b.lower.v[i], "incorrect lower bound");
        TEST_ASSERT(intersection.upper.v[i] == a.upper.v[i], "incorrect upper bound");
    }

    return TEST_SUCCESS;
}

#endif //SERAPHIM_TEST_TEST_BOUND3_H
