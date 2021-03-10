#ifndef SERAPHIM_SET_H
#define SERAPHIM_SET_H

#include <stdint.h>

typedef int (*srph_set_comparator)(const void *, const void *);

typedef struct srph_set_node {
    srph_set_node * children[2];
    bool colour;

    char data[];
} srph_set_node;

typedef struct srph_set {
    srph_set_node * root;
    uint32_t element_size;
    srph_set_comparator cmp;
} srph_set;


void srph_set_create(srph_set * s, srph_set_comparator cmp, uint32_t element_size);
void srph_set_destroy(srph_set * s);

void * srph_set_find(srph_set * s, void * key);
bool srph_set_contains(srph_set * s, void * data);
void srph_set_insert(srph_set * s, void * data);

void srph_set_delete(srph_set * s, void * data);

#endif

