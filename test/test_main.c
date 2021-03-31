//
// Created by millie on 31/03/2021.
//
#include <stdio.h>

#include "test_quat.h"

typedef int (*test_func)();

void run_test(test_func f, int * passed_tests, int * total_tests){
    if (f()){
        (*passed_tests)++;
    }
    (*total_tests)++;
}

int main(){
    int passed_tests = 0;
    int total_tests = 0;

    test_func functions[] = {
        test_quat_identity
    };

    for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); i++){
        run_test(functions[i], &passed_tests, &total_tests);
    }

    printf("Total tests run: %d\n", total_tests);
    printf("Total tests passed: %d\n", passed_tests);
    printf("Test pass rate: %f\n", (double) passed_tests * 100.0 / (double) total_tests);
}
