#ifndef _LIST_H_
#define _LIST_H_

#include <stdlib.h>

typedef struct LListItem_t LListItem_t;

struct LListItem_t
{
    void *obj;
    LListItem_t *next;
    LListItem_t *prev;
};

typedef struct
{
    LListItem_t *head;
    LListItem_t *tail;
    size_t size;
} LList_t;

void LList_init(LList_t *list);
LListItem_t *LList_head(LList_t *list);
LListItem_t *LList_tail(LList_t *list);
void LList_append(LList_t *list, void *item);
void *LList_pop(LList_t *list);
void *LList_get(LList_t *list, size_t index);

#define LLIST_DEFINE_FUNCTIONS(type, type_name)                \
    void LList_##type_name##_init(LList_t *list)               \
    {                                                          \
        LList_init(list);                                      \
    }                                                          \
    type *LList_##type_name##_head(LList_t *list)              \
    {                                                          \
        return list->head ? (type *)list->head->obj : NULL;    \
    }                                                          \
    type *LList_##type_name##_tail(LList_t *list)              \
    {                                                          \
        return list->tail ? (type *)list->tail->obj : NULL;    \
    }                                                          \
    void LList_##type_name##_append(LList_t *list, type *item) \
    {                                                          \
        LList_append(list, (void *)item);                      \
    }                                                          \
    type *LList_##type_name##_get(LList_t *list, size_t index) \
    {                                                          \
        return (type *)LList_get(list, index);                 \
    }                                                          \
    type *LList_##type_name##_pop(LList_t *list)               \
    {                                                          \
        return (type *)LList_pop(list);                        \
    }

#define LLIST_DEFINE_PROTOTYPES(type, type_name)                \
    void LList_##type_name##_init(LList_t *list);               \
    type *LList_##type_name##_head(LList_t *list);              \
    type *LList_##type_name##_tail(LList_t *list);              \
    void LList_##type_name##_append(LList_t *list, type *item); \
    type *LList_##type_name##_get(LList_t *list, size_t index); \
    type *LList_##type_name##_pop(LList_t *list);

#endif