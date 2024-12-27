#ifndef _SYMTAB_H_
#define _SYMTAB_H_

int symtab_add_global_symbol(char *symbol_name);
int symtab_find_global_symbol(char *symbol_name);
char *symtab_get_symbol_name(int symbol_index);

#endif