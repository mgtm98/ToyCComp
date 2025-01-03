#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "datatype.h"
typedef enum
{
    SYMBOL_VAR,
    SYMBOL_FUNC
} SymbolType_e;

typedef struct
{
    char *sym_name;
    SymbolType_e sym_type;
    Datatype_t *data_type;

} Symbol_t;

void symtab_init_global_symtab();
int symtab_add_global_symbol(char *symbol_name, SymbolType_e sym_type, Datatype_t *data_type);
int symtab_find_global_symbol(char *symbol_name);
Symbol_t *symtab_get_symbol(int symbol_index);

#endif