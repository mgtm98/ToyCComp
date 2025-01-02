#include "symtab.h"
#include "debug.h"

#include <string.h>
#include <stdlib.h>

#define GLOBAL_SYMBOL_SIZE 255

static Symbol_t global_symbols[GLOBAL_SYMBOL_SIZE];
static int global_symbols_index = 0;

int symtab_add_global_symbol(char *symbol_name, SymbolType_e sym_type, Datatype_t *data_type)
{
    int sym_info = symtab_find_global_symbol(symbol_name);
    if (sym_info > 0)
        return sym_info;

    if (global_symbols_index == GLOBAL_SYMBOL_SIZE)
    {
        debug_print(SEV_ERROR, "Reached the maximum number of global symbols");
        exit(0);
    }

    global_symbols[global_symbols_index].sym_name = strdup(symbol_name);
    global_symbols[global_symbols_index].sym_type = sym_type;
    global_symbols[global_symbols_index].data_type = data_type;
    debug_print(SEV_DEBUG, "Added symbol %s in globals symbol table", symbol_name);
    return global_symbols_index++;
}

int symtab_find_global_symbol(char *symbol_name)
{
    for (int i = 0; i < global_symbols_index; i++)
    {
        if (strcmp(symbol_name, global_symbols[i].sym_name) == 0)
        {
            return i;
        }
    }

    return -1;
}

Symbol_t *symtab_get_symbol(int symbol_index)
{
    return &global_symbols[symbol_index];
}