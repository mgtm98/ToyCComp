#include "llist.h"

void LList_init(LList_t *list)
{
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}
inline LListItem_t *LList_head(LList_t *list) { return list->head; }
inline LListItem_t *LList_tail(LList_t *list) { return list->tail; }

void LList_append(LList_t *list, void *item)
{
    LListItem_t *litem = (LListItem_t *)malloc(sizeof(LListItem_t));
    litem->next = NULL;
    litem->prev = NULL;
    litem->obj = item;
    if (list->head == NULL)
        list->head = litem;
    if (list->tail)
    {
        list->tail->next = litem;
        litem->prev = list->tail;
    }
    list->tail = litem;
    list->size++;
}

void *LList_pop(LList_t *list)
{
    LListItem_t *tail = list->tail;
    void *out = tail->obj;

    if (list->tail == list->head)
    {
        list->tail = NULL;
        list->head = NULL;
    }
    else
    {
        list->tail = tail->prev;
        list->tail->next = NULL;
    }

    list->size--;
    free(tail);
    return out;
}

void *LList_get(LList_t *list, size_t index)
{
    if (index >= list->size)
        return NULL;

    LListItem_t *item = list->head;
    for (size_t i = 0; i < index; i++)
        item = item->next;
    return item->obj;
}