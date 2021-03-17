#include "core/array.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#define MIN_CAPACITY 1

static void realloc_array(srph_array * a, uint32_t new_capacity){
    a->capacity = new_capacity;
    a->_data = realloc(a->_data, a->capacity * a->element_size); 
}    

void srph_array_create(srph_array * a, uint32_t element_size){
    assert(a != NULL && element_size != 0);

    a->element_size = element_size;
    a->_data = NULL;
    srph_array_clear(a);
}

void srph_array_destroy(srph_array * a){
    if (a != NULL && a->_data != NULL){
        free(a->_data);
    }
}

void * srph_array_first(const srph_array * a){
    return (a == NULL || a->size == 0) ? NULL : a->_data;
}

void * srph_array_last(const srph_array * a){
    return (a == NULL || a->size == 0) ? NULL : srph_array_at(a, a->size - 1);
}

void * srph_array_end(const srph_array * a){
    return a == NULL ? NULL : srph_array_at(a, a->size);
}

void * srph_array_push_back(srph_array * a){
    assert(a != NULL);    

    if (a->size == a->capacity){
        realloc_array(a, a->capacity * 2);
    }

    a->size++;
    return srph_array_last(a);
}

void srph_array_pop_back(srph_array * a, void * data){
    if (a->size == 0){
        return;
    }
    
    if (data != NULL){
        memcpy(data, srph_array_last(a), a->element_size);
    }

    a->size--;

    if (a->size < a->capacity / 2 && a->capacity >= MIN_CAPACITY * 2){
        realloc_array(a, a->capacity / 2);
    } 
}

void * srph_array_at(const srph_array * a, uint32_t i){
    if (a == NULL || a->_data == NULL || i > a->size){
        return NULL;
    }

    return ((uint8_t *) a->_data) + a->element_size * i;
}

bool srph_array_is_empty(const srph_array * a){
    return a == NULL || a->size == 0;
}

void srph_array_sort(srph_array * a, srph_comparator cmp){
    if (a->size < 2){
        return;
    }

    qsort(a->_data, a->size, a->element_size, cmp);
}

void * srph_array_find(srph_array * a, void * key, srph_comparator cmp){
    if (a == NULL || cmp == NULL || a->_data == NULL || a->size == 0){
        return NULL;
    }

    return bsearch(key, a->_data, a->size, a->element_size, cmp);
}

void srph_array_clear(srph_array * a){
    assert (a != NULL);

    a->size = 0;
    realloc_array(a, MIN_CAPACITY);
}
