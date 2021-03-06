#ifndef SERAPHIM_ARRAY_H
#define SERAPHIM_ARRAY_H

#include <stdint.h>

typedef int (*srph_comparator)(const void *, const void*);

typedef struct srph_array {
    uint32_t size;
    uint32_t capacity;
    uint32_t element_size;
    void * _data;
} srph_array;

void srph_array_create(srph_array * a, uint32_t element_size);
void srph_array_destroy(srph_array * a);

void srph_array_push_back(srph_array * a, const void * data);
void srph_array_pop_back(srph_array * a, void * data);

void * srph_array_first(const srph_array * a);
void * srph_array_last(const srph_array * a);
void * srph_array_end(const srph_array * a);

void * srph_array_at(const srph_array * a, uint32_t i);

bool srph_array_is_empty(const srph_array * a);

void srph_array_sort(srph_array * a, srph_comparator cmp); 
void * srph_array_find(srph_array * a, void * key, srph_comparator cmp);

#endif
