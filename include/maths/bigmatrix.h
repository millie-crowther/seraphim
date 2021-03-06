#ifndef SERAPHIM_MATRIX_H
#define SERAPHIM_MATRIX_H

#include <stdint.h>

#include "core/array.h"

typedef struct srph_matrix {
    uint32_t width;
    uint32_t height;
    srph_array xs;

    bool _is_sorted;
    bool _is_symmetric;
} srph_matrix;

void srph_matrix_create(srph_matrix * m, uint32_t width, uint32_t height);
void srph_matrix_create_symmetric(srph_matrix * m, uint32_t size);
void srph_matrix_destroy(srph_matrix * m);

double srph_matrix_at(srph_matrix * m, uint32_t x, uint32_t y);
void srph_matrix_set(srph_matrix * m, uint32_t x, uint32_t y, double a);

#endif
