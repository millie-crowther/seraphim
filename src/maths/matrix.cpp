#include "maths/bigmatrix.h"

#include <stdlib.h>

typedef struct srph_matrix_item {
    uint32_t i;
    double a;
} srph_matrix_item;

static int comparator(const void * _a, const void * _b){
    srph_matrix_item * a = (srph_matrix_item *) _a;
    srph_matrix_item * b = (srph_matrix_item *) _b;
    return (a->i > b->i) - (a->i < b->i);
}

static uint32_t index(srph_matrix * m, uint32_t x, uint32_t y){
    if (m->_is_symmetric && x > y){
        return index(m, y, x);
    }

    return m->width * y + x;
}

void srph_matrix_create(srph_matrix * m, uint32_t width, uint32_t height){
    m->width = width;
    m->height = height;
    m->_is_symmetric = false;

    srph_set_create(&m->s, comparator, sizeof(srph_matrix_item)); 
}

void srph_matrix_create_symmetric(srph_matrix * m, uint32_t size){
    srph_matrix_create(m, size, size);
    m->_is_symmetric = true;
}

void srph_matrix_destroy(srph_matrix * m){
    if (m != NULL){
        srph_set_destroy(&m->s);
    }
}

double srph_matrix_at(srph_matrix * m, uint32_t x, uint32_t y){
    srph_matrix_item key = { .i = index(m, x, y) };
    double * v = (double *) srph_set_find(&m->s, &key);
    return v == NULL ? 0 : *v;
}

void srph_matrix_set(srph_matrix * m, uint32_t x, uint32_t y, double a){
    srph_matrix_item key = { .i = index(m, x, y), .a = a };
    if (a == 0.0){
        srph_set_remove(&m->s, &key); 
    } else {
        srph_set_insert(&m->s, &key);
    }
}
