#ifndef SERAPHIM_ARRAY_H
#define SERAPHIM_ARRAY_H

#include <stddef.h>
#include <stdint.h>

#define array_t(T)                                                               \
    struct {                                                                        \
        size_t size;                                                                \
        size_t capacity;                                                            \
        size_t element_size;                                                        \
        union {                                                                     \
            T *data;                                                                \
            T *first;                                                               \
            uint8_t *raw_data;                                                      \
        };                                                                          \
        T *last;                                                                    \
        uint32_t offset;                                                            \
        uint8_t *base_ptr;                                                          \
    }
typedef array_t(uint8_t) array_base_t;

#define ARRAY_CAST(x) ((array_base_t *)x)

#define array_create(x) array_base_create(ARRAY_CAST(x), sizeof((x)->data[0]))
void array_base_create(array_base_t *a, size_t element_size);

#define array_clear(x) array_base_clear(ARRAY_CAST(x))
void array_base_clear(array_base_t *a);

#define array_is_empty(x) array_base_is_empty(ARRAY_CAST(x))
bool array_base_is_empty(array_base_t *a);

#define array_push_front(x) array_base_push_front(ARRAY_CAST(x))
void array_base_push_front(array_base_t *a);

#define array_push_back(x) array_base_push_back(ARRAY_CAST(x))
void array_base_push_back(array_base_t *a);

#define array_pop_front(x) array_base_pop_front(ARRAY_CAST(x))
void array_base_pop_front(array_base_t *a);

#define array_pop_back(x) array_base_pop_back(ARRAY_CAST(x))
void array_base_pop_back(array_base_t *a);

#define array_sort(x, f) array_base_sort(ARRAY_CAST(x), (f))
void array_base_sort(array_base_t *a,
                      int (*comparator)(const void *, const void *));

#endif
