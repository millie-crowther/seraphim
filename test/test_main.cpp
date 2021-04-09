//
// Created by millie on 31/03/2021.
//
#include <stdio.h>

#include "test_header.h"
#include "test_sdf.h"
#include "test_matrix.h"
#include "test_quat.h"
#include "test_transform.h"

int main(){
    int passed_tests = 0;
    int total_tests = 0;

    RUN_TEST(test_quat_identity);
    RUN_TEST(test_quat_to_matrix_identity);
    RUN_TEST(test_quat_from_angle_axis);
    RUN_TEST(test_quat_to_mat3);
    RUN_TEST(test_quat_to_mat4);
    RUN_TEST(test_quat_to_matrix_and_back);
    RUN_TEST(test_quat_multiply);
    RUN_TEST(test_quat_inverse_mult);

    RUN_TEST(test_sdf_inertia_tensor_cube);

    RUN_TEST(test_matrix_inverse_identity);
    RUN_TEST(test_matrix_inverse_scale);
    RUN_TEST(test_matrix_diagonal_determinant);
    RUN_TEST(test_matrix_determinant);
    RUN_TEST(test_matrix_apply_identity);
    RUN_TEST(test_matrix_apply_rotation);
    RUN_TEST(test_matrix_transpose_symmetric);
    RUN_TEST(test_matrix_multiply_diagonal);
    RUN_TEST(test_matrix_multiply);
    RUN_TEST(test_matrix_transpose);
    RUN_TEST(test_matrix_inverse_multiply_identity);

    RUN_TEST(test_transform_matrix);

    printf("Total tests run: %d\n", total_tests);
    printf("Total tests passed: %d\n", passed_tests);
    printf("Test pass rate: %.2f%%\n", (double) passed_tests * 100.0 / (double) total_tests);
}
