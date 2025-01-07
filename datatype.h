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
    DT_INT,
    DT_LONG
} Datatype_Primative_e;

extern Datatype_t __supported_primative_types[];

#define DATATYPE_VOID (&__supported_primative_types[0])
#define DATATYPE_CHAR (&__supported_primative_types[1])
#define DATATYPE_INT (&__supported_primative_types[2])
#define DATATYPE_LONG (&__supported_primative_types[3])

Datatype_t *datatype_get_type(Scanner_t *scanner);
Datatype_t *datatype_get_primative_type(Datatype_Primative_e type);
Datatype_t *datatype_expr_type(Datatype_t *left, Datatype_t *right);
void datatype_check_assign_expr_type(Datatype_t *left, Datatype_t *right);

#endif