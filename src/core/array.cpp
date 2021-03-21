#include "core/array.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void fix_ptrs(_srph_base_array * a){
    assert(a != NULL);

    if (a->size == 0){
        if (a->_base_ptr != NULL){
            free(a->_base_ptr);
            a->_base_ptr = NULL;
        }

        a->first = NULL;
        a->last = NULL;
    } else {
        if (a->_base_ptr == NULL){
            a->_base_ptr = (uint8_t *) malloc(a->size * a->element_size);
        }

        a->first = a->_base_ptr + a->_offset     * a->element_size;
        a->last  = a->first     + (a->size - 1) * a->element_size;
    }
}

void _srph_array_init(_srph_base_array * a, size_t element_size){
    assert(a != NULL);
    *a = {
        .size = 0,
        .capacity = 0,
        .element_size = element_size,
        .first = NULL,
        .last = NULL,
        ._offset = 0,
        ._base_ptr = NULL,
    };
}

bool _srph_array_is_empty(_srph_base_array * a){
    assert(a != NULL);
    return a->size == 0;
}

void _srph_array_clear(_srph_base_array * a){
    assert(a != NULL);
    free(a->_base_ptr);
    a->size = 0;
    a->capacity = 0;
    a->first = NULL;
    a->last = NULL;
    a->_offset = 0;
    a->_base_ptr = NULL;
}

void _srph_array_push_back(_srph_base_array * a){
    assert(a != NULL);
    
    if (a->size + a->_offset >= a->capacity){
        a->capacity = a->capacity == 0 ? 1 : a->capacity * 2;
        a->_base_ptr = (uint8_t *) realloc(a->_base_ptr, a->capacity * a->element_size);
    }
    
    a->size++;
    fix_ptrs(a);
}

void _srph_array_push_front(_srph_base_array * a){
    assert(a != NULL);
    
    if (a->_offset == 0){
        size_t new_capacity = a->capacity == 0 ? 1 : a->capacity * 2;
        size_t new_offset = new_capacity - a->capacity;
        uint8_t * new_data = (uint8_t *) malloc(new_capacity * a->element_size);

        if (a->_base_ptr != NULL){
            memcpy(
                new_data + new_offset * a->element_size, 
                a->_base_ptr, 
                a->size * a->element_size
            );
        }
        
        free(a->_base_ptr);
        a->_base_ptr = new_data;
        a->_offset = new_offset;
        a->capacity = new_capacity;     
    }

    a->_offset--;
    a->size++;
    fix_ptrs(a);
}

void _srph_array_pop_front(_srph_base_array * a){
    assert(a != NULL && !_srph_array_is_empty(a));
    
    if (a->_offset > a->capacity / 2){
        size_t new_capacity = a->capacity / 2;
        size_t new_offset = a->capacity - new_capacity;
        uint8_t * new_data = (uint8_t *) malloc(new_capacity * a->element_size);
        
        memcpy(
            new_data     + new_offset * a->element_size,
            a->_base_ptr + a->_offset * a->element_size,
            a->size * a->element_size
        );

        free(a->_base_ptr);
        a->_base_ptr = new_data;
        a->_offset = new_offset;
        a->capacity = new_capacity;
    }

    a->_offset++;
    a->size--;
    fix_ptrs(a);
}

void _srph_array_pop_back(_srph_base_array * a){
    assert(a != NULL && !_srph_array_is_empty(a));

    if (a->_offset + a->size < a->capacity / 2){
        a->capacity = a->capacity / 2;
        a->_base_ptr = (uint8_t *) realloc(a->_base_ptr, a->capacity * a->element_size);
    }
    
    a->size--;
    fix_ptrs(a);
}
