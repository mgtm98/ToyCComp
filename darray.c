#include "darray.h"

#include <stdio.h>
#include <string.h>

void darray_init(DArray_t *arr, size_t cap, size_t element_size)
{
    arr->element_size = element_size;
    arr->capacity = cap;
    arr->buffer = calloc(cap, element_size);
}

void *darray_get(DArray_t *arr, size_t index)
{
    if (index >= arr->capacity)
    {
        arr->capacity *= 2;
        arr->buffer = realloc(arr->buffer, arr->capacity * arr->element_size);
    }

    return (char *)arr->buffer + index * arr->element_size;
}
