#ifndef SERAPHIM_ARRAY_H
#define SERAPHIM_ARRAY_H

#include <stdint.h>

#define srph_array(T) struct { uint32_t size; uint32_t capacity; T * data; T * _last; }
typedef srph_array(uint8_t) _srph_base_array;

#define SERAPHIM_ARRAY_BASE_CAST(x) ((_srph_base_array *) x)
#define SERAPHIM_ARRAY_ELEMENT_SIZE(x) sizeof((x)->data[0])

#define srph_array_first(x) ((x)->data)
#define srph_array_last(x) ((x)->_last)

#define srph_array_init(x) _srph_array_init(SERAPHIM_ARRAY_BASE_CAST(x))
void _srph_array_init(_srph_base_array * a);

#define srph_array_clear(x) _srph_array_clear(SERAPHIM_ARRAY_BASE_CAST(x))
void _srph_array_clear(_srph_base_array * a);

#define srph_array_pop_back(x) \
    _srph_array_pop_back(SERAPHIM_ARRAY_BASE_CAST(x), SERAPHIM_ARRAY_ELEMENT_SIZE(x))
void _srph_array_pop_back(_srph_base_array * a, uint32_t element_size);

#define srph_array_push_back(x) \
    _srph_array_push_back(SERAPHIM_ARRAY_BASE_CAST(x), SERAPHIM_ARRAY_ELEMENT_SIZE(x))
void _srph_array_push_back(_srph_base_array * a, uint32_t element_size);

#define srph_array_is_empty(x) _srph_array_is_empty(SERAPHIM_ARRAY_BASE_CAST(x))
bool _srph_array_is_empty(const _srph_base_array * a);

#endif
