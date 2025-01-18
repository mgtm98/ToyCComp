#include "symtab.h"
#include "debug.h"
#include "darray.h"
#include "llist_definitions.h"

#include <string.h>
#include <stdlib.h>

#define GLOBAL_SYMBOL_SIZE 255

#define GlobalSymTab(index) (*(Symbol_t **)darray_get(&global_symbols, index))

static DArray_t global_symbols;
static int global_symbols_index = 0;

int symtab_add_global_symbol(char *symbol_name, SymbolType_e sym_type, Datatype_t *data_type)
{
    int sym_info = symtab_find_global_symbol(symbol_name);
    if (sym_info > -1)
    {
        debug_print(SEV_ERROR, "[SYMTAB] Redefining symbol %s", symbol_name);
        exit(1);
    }

    if (sym_type == SYMBOL_VAR)
    {
        GlobalSymTab(global_symbols_index) = malloc(sizeof(Symbol_t));
        GlobalSymTab(global_symbols_index)->sym_name = strdup(symbol_name);
        GlobalSymTab(global_symbols_index)->sym_type = sym_type;
        GlobalSymTab(global_symbols_index)->data_type = data_type;
    }
    else if (sym_type == SYMBOL_FUNC)
    {
        GlobalSymTab(global_symbols_index) = malloc(sizeof(SymbolFunc_t));
        ((SymbolFunc_t *)GlobalSymTab(global_symbols_index))->sym_name = strdup(symbol_name);
        ((SymbolFunc_t *)GlobalSymTab(global_symbols_index))->sym_type = sym_type;
        ((SymbolFunc_t *)GlobalSymTab(global_symbols_index))->data_type = data_type;
        LList_init(&(((SymbolFunc_t *)GlobalSymTab(global_symbols_index))->args));
    }
    else
    {
        debug_print(SEV_ERROR, "[SYMTAB] Unsupported symbol type");
        exit(1);
    }

    debug_print(SEV_DEBUG, "Added symbol %s in globals symbol table", symbol_name);
    return global_symbols_index++;
}

int symtab_find_global_symbol(char *symbol_name)
{
    for (int i = 0; i < global_symbols_index; i++)
    {
        if (
            strcmp(symbol_name, GlobalSymTab(i)->sym_name) == 0)
        {
            return i;
        }
    }

    return -1;
}

Symbol_t *symtab_get_symbol(int symbol_index)
{
    return GlobalSymTab(symbol_index);
}

void symtab_init_global_symtab()
{
    darray_init(&global_symbols, GLOBAL_SYMBOL_SIZE, sizeof(Symbol_t *));

    int lib_print = symtab_add_global_symbol("print", SYMBOL_FUNC, DATATYPE_VOID);
    SymbolFuncArg_t *argument = (SymbolFuncArg_t *)malloc(sizeof(SymbolFuncArg_t));
    argument->arg_name = "x";
    argument->arg_type = DATATYPE_LONG;
    LList_SymbolFuncArg_append(&((SymbolFunc_t *)symtab_get_symbol(lib_print))->args, argument);

    lib_print = symtab_add_global_symbol("print_char", SYMBOL_FUNC, DATATYPE_VOID);
    argument = (SymbolFuncArg_t *)malloc(sizeof(SymbolFuncArg_t));
    argument->arg_name = "x";
    argument->arg_type = DATATYPE_CHAR;
    LList_SymbolFuncArg_append(&((SymbolFunc_t *)symtab_get_symbol(lib_print))->args, argument);

    lib_print = symtab_add_global_symbol("print_str", SYMBOL_FUNC, DATATYPE_VOID);
    argument = (SymbolFuncArg_t *)malloc(sizeof(SymbolFuncArg_t));
    argument->arg_name = "x";
    argument->arg_type = datatype_get_pointer_of(DATATYPE_CHAR);
    LList_SymbolFuncArg_append(&((SymbolFunc_t *)symtab_get_symbol(lib_print))->args, argument);

    lib_print = symtab_add_global_symbol("print_ln", SYMBOL_FUNC, DATATYPE_VOID);
    argument = (SymbolFuncArg_t *)malloc(sizeof(SymbolFuncArg_t));
    argument->arg_name = "x";
    argument->arg_type = datatype_get_pointer_of(DATATYPE_CHAR);
    LList_SymbolFuncArg_append(&((SymbolFunc_t *)symtab_get_symbol(lib_print))->args, argument);
}