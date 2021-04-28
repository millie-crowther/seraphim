#include "core/array.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static void fix_ptrs(_srph_base_array * a)
{
	if (a->size == 0) {
		free(a->base_ptr);
		a->base_ptr = NULL;
		a->first = NULL;
		a->last = NULL;
	} else {
		if (a->base_ptr == NULL) {
			a->base_ptr =
			    (uint8_t *) malloc(a->size * a->element_size);
		}

		a->first = a->base_ptr + a->offset * a->element_size;
		a->last = a->first + (a->size - 1) * a->element_size;
	}
}

void _srph_array_init(_srph_base_array * a, size_t element_size)
{
	a->element_size = element_size;
	a->base_ptr = NULL;
	srph_array_clear(a);
}

bool _srph_array_is_empty(_srph_base_array * a)
{
	return a->size == 0;
}

void _srph_array_clear(_srph_base_array * a)
{
	a->size = 0;
	a->capacity = 0;
	a->offset = 0;
	fix_ptrs(a);
}

void _srph_array_push_back(_srph_base_array * a)
{
	if (a->size + a->offset >= a->capacity) {
		a->capacity = a->capacity == 0 ? 1 : a->capacity * 2;
		a->base_ptr =
		    (uint8_t *) realloc(a->base_ptr,
					a->capacity * a->element_size);
	}

	a->size++;
	fix_ptrs(a);
}

void _srph_array_push_front(_srph_base_array * a)
{
	if (a->offset == 0) {
		size_t new_capacity = a->capacity == 0 ? 1 : a->capacity * 2;
		size_t new_offset = new_capacity - a->capacity;
		uint8_t *new_data =
		    (uint8_t *) malloc(new_capacity * a->element_size);

		if (a->base_ptr != NULL) {
			memcpy(new_data + new_offset * a->element_size,
			       a->base_ptr, a->size * a->element_size);
		}

		free(a->base_ptr);
		a->base_ptr = new_data;
		a->offset = new_offset;
		a->capacity = new_capacity;
	}

	a->offset--;
	a->size++;
	fix_ptrs(a);
}

void _srph_array_pop_front(_srph_base_array * a)
{
	assert(!_srph_array_is_empty(a));

	if (a->offset > a->capacity / 2) {
		size_t new_capacity = a->capacity / 2;
		size_t new_offset = a->capacity - new_capacity;
		uint8_t *new_data =
		    (uint8_t *) malloc(new_capacity * a->element_size);

		memcpy(new_data + new_offset * a->element_size,
		       a->base_ptr + a->offset * a->element_size,
		       a->size * a->element_size);

		free(a->base_ptr);
		a->base_ptr = new_data;
		a->offset = new_offset;
		a->capacity = new_capacity;
	}

	a->offset++;
	a->size--;
	fix_ptrs(a);
}

void _srph_array_pop_back(_srph_base_array * a)
{
	assert(!_srph_array_is_empty(a));

	if (a->offset + a->size < a->capacity / 2) {
		a->capacity = a->capacity / 2;
		a->base_ptr =
		    (uint8_t *) realloc(a->base_ptr,
					a->capacity * a->element_size);
	}

	a->size--;
	fix_ptrs(a);
}

void _srph_array_sort(_srph_base_array * a,
		      int (*comparator)(const void *, const void *))
{
	if(a->size <= 1) {
		return;
	}

	qsort(a->base_ptr, a->size, a->element_size, comparator);
}
