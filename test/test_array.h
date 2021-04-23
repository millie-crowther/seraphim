//
// Created by millie on 14/04/2021.
//

#ifndef SERAPHIM_TEST_ARRAY_H
#define SERAPHIM_TEST_ARRAY_H

#include "test_header.h"

#include "core/array.h"

static int int_comparator(const void * _a, const void * _b){
    int a = *(int *) _a;
    int b = *(int *) _b;

    if (a < b){
        return -1;
    } else if (a > b){
        return 1;
    } else {
        return 0;
    }
}

extern inline const char * test_int_array_quick_sort(){
    srph_array(int) xs {};
    srph_array_init(&xs);

    for (int i = 0; i < 5; i++){
        srph_array_push_back(&xs);
    }

    xs.data[0] = 4;
    xs.data[1] = 3;
    xs.data[2] = 2;
    xs.data[3] = 1;
    xs.data[4] = 0;

    srph_array_sort(&xs, int_comparator);

    for (size_t i = 0; i < xs.size; i++){
        TEST_ASSERT(xs.data[i] == (int) i, "array not sorted properly");
    }

    return TEST_SUCCESS;
}

#endif //SERAPHIM_TEST_ARRAY_H
