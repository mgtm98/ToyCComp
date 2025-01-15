#include "datatype.h"
#include "debug.h"

#include <stdbool.h>
#include <string.h>

// Supported primative sizes are related to the arch of the machine
// TODO: Move this to code gen
Datatype_t __supported_primative_types[] = {
    {"void", 0, 0, 0, NULL},
    {"char", 8, 0, 0, NULL},
    {"int", 32, 0, 0, NULL},
    {"long", 64, 0, 0, NULL}};

static char *datatype_to_str(Datatype_t *type)
{
    char *p = malloc(sizeof(char) * (strlen(type->name) + type->pointer_level + 1));
    strcpy(p, type->name);
    memset(p + strlen(type->name), '*', type->pointer_level);
    p[strlen(type->name) + type->pointer_level] = '\0';
    return p;
}

Datatype_t *datatype_get_type(Scanner_t *scanner)
{
    Token_t tok;
    Datatype_t *out;

    scanner_scan(scanner, &tok);
    switch (tok.type)
    {
    case TOK_CHAR:
        out = &__supported_primative_types[DT_CHAR];
        break;
    case TOK_INT:
        out = &__supported_primative_types[DT_INT];
        break;
    case TOK_VOID:
        out = &__supported_primative_types[DT_VOID];
        break;
    case TOK_LONG:
        out = &__supported_primative_types[DT_LONG];
        break;
    default:
        if (tok.type != TOK_ID)
        {
            debug_print(
                SEV_ERROR,
                "[DATATYPE] Expected ID token or primative datatype token, found %s", TokToString(tok));
            exit(1);
        }
        else
        {
            debug_print(
                SEV_ERROR,
                "[DATATYPE] Unrecognized datatype %s", tok.value.str_value);
            exit(1);
        }
    }

    Datatype_t *t = out;
    while (1)
    {
        scanner_peek(scanner, &tok);
        if (tok.type != TOK_STAR)
            break;
        scanner_scan(scanner, &tok);
        t = datatype_get_pointer_of(out);
        if (out->base_type != NULL)
            free(out);
        out = t;
    }

    return t;
}

void check_pointer_levels(Datatype_t *left, Datatype_t *right)
{
    if (
        (left->pointer_level > 0 && right == datatype_get_primative_type(DT_LONG)) ||
        (right->pointer_level > 0 && left == datatype_get_primative_type(DT_LONG)))
        return;

    if (left->pointer_level != right->pointer_level)
    {
        char *t1, *t2;
        t1 = datatype_to_str(left);
        t2 = datatype_to_str(right);
        debug_print(
            SEV_ERROR,
            "[DATATYPE] Can't mix expressions of types %s and %s",
            t1,
            t2);
        free(t1);
        free(t2);
        exit(1);
    }

    if (left->pointer_level > 0 && right->pointer_level > 0 && left->base_type != right->base_type)
    {
        char *t1, *t2;
        t1 = datatype_to_str(left);
        t2 = datatype_to_str(right);
        debug_print(
            SEV_ERROR,
            "[DATATYPE] Can't assign type %s to %s",
            t1,
            t2);
        free(t1);
        free(t2);
        exit(1);
    }
}

// TODO check for items other than type size later
Datatype_t *datatype_expr_type(Datatype_t *left, Datatype_t *right)
{
    // similar types
    if (left == right)
        return left;

    // check_pointer_levels(left, right);

    // void operations
    if (
        (left == DATATYPE_VOID && right != DATATYPE_VOID) ||
        (right == DATATYPE_VOID && left != DATATYPE_VOID))
    {
        debug_print(SEV_ERROR, "[DATATYPE] Can't use void in an expression");
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
    check_pointer_levels(left, right);

    // void operations
    if (
        (left == DATATYPE_VOID && right != DATATYPE_VOID) ||
        (right == DATATYPE_VOID && left != DATATYPE_VOID))
    {
        debug_print(SEV_ERROR, "[DATATYPE] Can't use void in an expression");
        exit(1);
    }

    if (left->size < right->size)
    {
        debug_print(SEV_ERROR, "[DATATYPE] Can't assign %s to %s", right->name, left->name);
        exit(1);
    }
}

Datatype_t *datatype_get_primative_type(Datatype_Primative_e type)
{
    return &__supported_primative_types[type];
}

Datatype_t *datatype_get_pointer_of(Datatype_t *type)
{
    Datatype_t *out = malloc(sizeof(Datatype_t));
    out->name = type->name;
    out->size = 64; // pointer size is always 8 bytes
    out->pointer_level = type->pointer_level + 1;
    if (type->base_type == NULL)
        out->base_type = type;
    else
        out->base_type = type->base_type;
    return out;
}

Datatype_t *datatype_deref_pointer(Datatype_t *type, __uint8_t derefrence_level)
{
    Datatype_t *out = malloc(sizeof(Datatype_t));
    out->name = type->name;
    if (type->pointer_level == 0)
    {
        debug_print(SEV_ERROR, "[DATATYPE] Can't defrence %s", datatype_to_str(type));
        exit(1);
    }
    out->pointer_level = type->pointer_level - derefrence_level;
    if (out->pointer_level < 0)
    {
        debug_print(
            SEV_ERROR,
            "[DATATYPE] Can't defrence %s %d times",
            datatype_to_str(type),
            derefrence_level);
        exit(1);
    }

    if (out->pointer_level == 0)
    {
        out->size = type->base_type->size;
        out->base_type = NULL;
    }
    else
    {
        out->size = 64; // pointer size is always 8 bytes
        out->base_type = type->base_type;
    }
    return out;
}