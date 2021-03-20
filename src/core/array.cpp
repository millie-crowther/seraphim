#include "core/array.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

static void check_array(_srph_base_array * a, size_t element_size){
    assert(a != NULL);
    size_t old_capacity = a->capacity;  
    
    while (a->size > a->capacity){
        a->capacity = a->capacity == 0 ? 1 : a->capacity * 2;
    }

    while (a->size < a->capacity / 2){
        a->capacity /= 2;
    } 

    if (a->capacity != old_capacity){
        a->data = (uint8_t *) realloc(a->data, a->capacity * element_size);
    }

    assert(a->data != NULL);
    a->_last = a->data + (a->size - 1) * element_size;
}

void _srph_array_init(_srph_base_array * a){
    assert(a != NULL);
    *a = {
        .size = 0,
        .capacity = 0,
        .data = NULL,
        ._last = NULL,
    };
}

bool _srph_array_is_empty(_srph_base_array * a){
    return a == NULL || a->data == NULL || a->size == 0;
}

void _srph_array_clear(_srph_base_array * a){
    assert(a != NULL);
    if (a->data != NULL){
        free(a->data);
        _srph_array_init(a);
    }
}

void _srph_array_push_back(_srph_base_array * a, size_t element_size){
    assert(a != NULL);
    a->size++;
    check_array(a, element_size);
}

void _srph_array_pop_back(_srph_base_array * a, size_t element_size){
    assert(a != NULL && !_srph_array_is_empty(a));
    a->size--;
    
    if (_srph_array_is_empty(a)){
        _srph_array_clear(a);
    } else {
        check_array(a, element_size);
    }
}
