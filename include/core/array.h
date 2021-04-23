#ifndef SERAPHIM_ARRAY_H
#define SERAPHIM_ARRAY_H

#include <stddef.h>
#include <stdint.h>

#define srph_array(T) struct { \
    size_t size;  \
    size_t capacity; \
    size_t element_size; \
    union { T * data; T * first; uint8_t * raw_data; }; \
    T * last; \
    uint32_t offset; \
    uint8_t * base_ptr; \
}
typedef srph_array(uint8_t) _srph_base_array;

#define SRPH_ARRAY_CAST(x) ((_srph_base_array *) x)

#define srph_array_init(x) _srph_array_init(SRPH_ARRAY_CAST(x), sizeof((x)->data[0]))
void _srph_array_init(_srph_base_array * a, size_t element_size);

#define srph_array_clear(x) _srph_array_clear(SRPH_ARRAY_CAST(x))
void _srph_array_clear(_srph_base_array * a);

#define srph_array_is_empty(x) _srph_array_is_empty(SRPH_ARRAY_CAST(x))
bool _srph_array_is_empty(_srph_base_array * a);

#define srph_array_push_front(x) _srph_array_push_front(SRPH_ARRAY_CAST(x))
void _srph_array_push_front(_srph_base_array * a);

#define srph_array_push_back(x) _srph_array_push_back(SRPH_ARRAY_CAST(x))
void _srph_array_push_back(_srph_base_array * a);

#define srph_array_pop_front(x) _srph_array_pop_front(SRPH_ARRAY_CAST(x))
void _srph_array_pop_front(_srph_base_array * a);

#define srph_array_pop_back(x) _srph_array_pop_back(SRPH_ARRAY_CAST(x))
void _srph_array_pop_back(_srph_base_array * a);

#define srph_array_sort(x, f) _srph_array_sort(SRPH_ARRAY_CAST(x), (f))
void _srph_array_sort(_srph_base_array * a, int (*comparator)(const void *, const void *));

#endif
