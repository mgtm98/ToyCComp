#ifndef _DATATYPE_H_
#define _DATATYPE_H_

#include <stdlib.h>
#include <stdbool.h>

#include "scanner.h"

typedef struct
{
    char *name;
    __uint8_t size;
} Datatype_t;

typedef enum
{
    DT_VOID,
    DT_CHAR,
    DT_INT
} Datatype_Primative_e;

Datatype_t *datatype_get_type(Scanner_t *scanner);
Datatype_t *datatype_get_primative_type(Datatype_Primative_e type);
Datatype_t *datatype_expr_type(Datatype_t *left, Datatype_t *right);
void datatype_check_assign_expr_type(Datatype_t *left, Datatype_t *right);

#endif