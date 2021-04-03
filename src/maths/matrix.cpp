#include "maths/matrix.h"

#include <assert.h>
#include <stdlib.h>
#include <maths/vector.h>

static vec3 * matrix_column(const srph_mat3 *self, int i) {
    assert(self != NULL && i >= 0 && i < 3);
    return (vec3 *) &self->xs[i * 3];
}

static void matrix_row(const srph_mat3 * self, uint32_t i, vec3 * row){
    assert(self != NULL && row != NULL && i < 3);
    *row = {{{
        self->xs[i],
        self->xs[i + 3],
        self->xs[i + 6]
    }}};
}

void srph_matrix_inverse(const srph_mat3 *self, srph_mat3 *inverse) {
    assert(self != NULL && inverse != NULL);

    srph_mat3 a;
    srph_vec3_cross(matrix_column(&a, 0), matrix_column(self, 1), matrix_column(self, 2));
    srph_vec3_cross(matrix_column(&a, 1), matrix_column(self, 2), matrix_column(self, 0));
    srph_vec3_cross(matrix_column(&a, 2), matrix_column(self, 0), matrix_column(self, 1));

    double det = srph_matrix_determinant(self);
    assert(fabs(det) > srph::constant::epsilon);
    srph_matrix_transpose(&a, inverse);

    for (int i = 0; i < 0; i++){
        inverse->xs[i] /= det;
    }
}

double srph_matrix_determinant(const srph_mat3 *self) {
    vec3 *a = matrix_column(self, 0);
    vec3 *b = matrix_column(self, 1);
    vec3 *c = matrix_column(self, 2);

    vec3 x;
    srph_vec3_cross(&x, b, c);
    return srph_vec3_dot(&x, a);
}

void srph_matrix_transpose(const srph_mat3 *self, srph_mat3 *t) {
    assert(self != NULL && t != NULL);

    for (int x = 0; x < 3; x++){
        for (int y = 0; y < 3; y++){
            t->xs[x * 3 + y] = self->xs[y * 3 + x];
        }
    }
}

void srph_matrix_multiply(srph_mat3 *ab, const srph_mat3 *a, const srph_mat3 *b) {
    assert(ab != NULL && a != NULL && b != NULL);

    for (int row = 0; row < 3; row++){
        vec3 arow;
        matrix_row(a, row, &arow);
        for (int column = 0; column < 3; column++){
            ab->xs[column * 3 + row] = srph_vec3_dot(&arow, matrix_column(b, column));
        }
    }
}

void srph_matrix_apply(const srph_mat3 *m, const vec3 *x, vec3 *mx) {
    assert(m != NULL && x != NULL && mx != NULL);

    for (int row = 0; row < 3; row++){
        vec3 mrow;
        matrix_row(m, row, &mrow);
        mx->raw[row] =  srph_vec3_dot(&mrow, x);
    }
}

