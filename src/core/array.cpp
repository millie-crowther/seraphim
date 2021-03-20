#include "core/array.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void realloc_array(_srph_base_array * a, uint32_t new_capacity, uint32_t element_size){
    a->capacity = new_capacity;
    a->data = (uint8_t *) realloc(a->data, a->capacity * element_size); 
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

void _srph_array_clear(_srph_base_array * a){
    if (a != NULL){
        if (a->data != NULL){
            free(a->data);
        }
    
        _srph_array_init(a);    
    }
}

void _srph_array_push_back(_srph_base_array * a, uint32_t element_size){
    assert(a != NULL);    

    while (a->size >= a->capacity){
        uint32_t new_capacity = a->capacity == 0 ? 1 : a->capacity * 2;
        realloc_array(a, new_capacity, element_size);
    }

    a->size++;
    
    if (a->size == 1){
        a->_last = a->data;
    } else {
        a->_last += element_size;
    }
}

void _srph_array_pop_back(_srph_base_array * a, uint32_t element_size){
    assert(a != NULL && !_srph_array_is_empty(a));
    
    if (a->size == 1){
        _srph_array_clear(a);
    } else if (a->size < a->capacity / 2){
        realloc_array(a, a->capacity / 2, element_size);
    } 
    
    a->size--;
    a->_last -= element_size;
}

bool _srph_array_is_empty(const _srph_base_array * a){
    return a == NULL || a->data == NULL || a->size == 0;
}
