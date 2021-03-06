#include "maths/matrix.h"

typedef struct srph_matrix_item {
    uint32_t i;
    double a;
} srph_matrix_item;

static int comparator(const void * _a, const void * _b){
    srph_matrix_item * a = (srph_matrix_item *) _a;
    srph_matrix_item * b = (srph_matrix_item *) _b;
    return (a->i > b->i) - (a->i < b->i);
}

static uint32_t index_flatten(srph_matrix * m, uint32_t x, uint32_t y){
    return m->width * y + x;
}

void srph_array_create(srph_matrix * m, uint32_t width, uint32_t height){
    m->width = width;
    m->height = height;
    m->_is_sorted = true;

    srph_array_create(&m->xs, sizeof(srph_matrix_item)); 
}

void srph_matrix_destroy(srph_matrix * m){
    if (m == NULL){
        return;
    }

    srph_array_destroy(&m->xs);
}

double srph_matrix_at(srph_matrix * m, uint32_t x, uint32_t y){
    if (!m->_is_sorted){
        srph_array_sort(&m->xs, comparator);
        m->_is_sorted = false;
    }

    srph_matrix_item key = { .i = index_flatten(m, x, y) };
    void * value = srph_array_find(&m->xs, (void *) &key, comparator);
    if (value == NULL){
        return 0.0;
    } else {
        return ((srph_matrix_item *) value)->a;
    }
}

void srph_matrix_set(srph_matrix * m, uint32_t x, uint32_t y, double a){
    srph_matrix_item x = { .i = index_flatten(m, x, y), .a = a };
    srph_array_push_back(&m->xs, &x);
    m->_is_sorted = false;
}
