#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdlib.h>

typedef struct
{
    void *buffer;
    size_t capacity;
    size_t element_size;
} DArray_t;

void darray_init(DArray_t *arr, size_t cap, size_t element_size);
void *darray_get(DArray_t *arr, size_t index);

#endif