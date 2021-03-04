#include "core/array.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

void srph_array_create(srph_array * a, uint32_t element_size){
    a->element_size = element_size;
    a->size = 0;
    a->capacity = 1;
    a->_data = malloc(element_size); 
}

void srph_array_destroy(srph_array * a){
    if (a != NULL && a->_data != NULL){
        free(a->_data);
    }
}

void * srph_array_first(srph_array * a){
    return a->size == 0 ? NULL : a->_data;
}

void * srph_array_last(srph_array * a){
    return a->size == 0 ? NULL : srph_array_at(a, a->size - 1);
}

void srph_array_push_back(srph_array * a, const void * data){
    if (a->size == a->capacity){
        a->capacity *= 2;
        a->_data = realloc(a->_data, a->capacity * a->element_size); 
    }

    memcpy(srph_array_at(a, a->size), data, a->element_size);
}

void srph_array_pop_back(srph_array * a, void * data){
    if (a->size == 0){
        return;
    }
    
    if (data != NULL){
        memcpy(data, srph_array_at(a, a->size - 1), a->element_size);
    }

    a->size--;

    if (a->size < a->capacity / 2 && a->capacity > 1){
        a->capacity /= 2;
        a->_data = realloc(a->_data, a->capacity * a->element_size);
    } 
}

void * srph_array_at(const srph_array * a, uint32_t i){
    if (a == NULL || i >= a->size){
        return NULL;
    }

    return ((uint8_t *) a->_data) + a->element_size * i;
}

bool srph_array_is_empty(const srph_array * a){
    return a->size == 0;
}
