#include "datatype.h"
#include "debug.h"

#include <stdbool.h>

#define DATATYPE_VOID (&__supported_primative_types[0])
#define DATATYPE_CHAR (&__supported_primative_types[1])
#define DATATYPE_INT (&__supported_primative_types[2])

Datatype_t __supported_primative_types[] = {
    {"void", 0},
    {"char", 8},
    {"int", 32}};

Datatype_t *datatype_get_type(Scanner_t *scanner)
{
    Token_t tok;

    scanner_scan(scanner, &tok);
    switch (tok.type)
    {
    case TOK_CHAR:
        return &__supported_primative_types[DT_CHAR];
    case TOK_INT:
        return &__supported_primative_types[DT_INT];
    case TOK_VOID:
        return &__supported_primative_types[DT_VOID];
    default:
        if (tok.type != TOK_ID)
        {
            debug_print(
                SEV_ERROR,
                "Expected ID token or primative datatype tokeen, found %s", TokToString(tok));
            exit(1);
        }
        else
        {
            debug_print(
                SEV_ERROR,
                "Unrecognized datatype %s", tok.value.str_value);
            exit(1);
        }
    }
}

// TODO check for items other than type size later
Datatype_t *datatype_expr_type(Datatype_t *left, Datatype_t *right)
{
    // similar types
    if (left == right)
        return left;

    // void operations
    if (
        (left == DATATYPE_VOID && right != DATATYPE_VOID) ||
        (right == DATATYPE_VOID && left != DATATYPE_VOID))
    {
        debug_print(SEV_ERROR, "Can't use void in an expression");
        exit(1);
    }

    if (left->size > right->size)
        return left;
    else
        return right;
}

// TODO check for items other than type size later
void datatype_check_assign_expr_type(Datatype_t *left, Datatype_t *right)
{
    // void operations
    if (
        (left == DATATYPE_VOID && right != DATATYPE_VOID) ||
        (right == DATATYPE_VOID && left != DATATYPE_VOID))
    {
        debug_print(SEV_ERROR, "Can't use void in an expression");
        exit(1);
    }

    if (left->size < right->size)
    {
        debug_print(SEV_ERROR, "Can't assign %s to %s", right->name, left->name);
        exit(1);
    }
}

Datatype_t *datatype_get_primative_type(Datatype_Primative_e type)
{
    return &__supported_primative_types[type];
}