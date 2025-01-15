#include "asm.h"
#include "debug.h"
#include "codegen.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define GLOBAL_REG_COUNT 4
#define MAX_BSS_SYMBOLS 1000

typedef struct
{
    char *symbol_name;
    RegSize_e size;
    int number_of_items;
} bss_symbol;

static int free_reg[GLOBAL_REG_COUNT] = {1, 1, 1, 1};
static char *reg_list[] = {"r12", "r13", "r14", "r15", "rax"};
static char *dreg_list[] = {"r12d", "r13d", "r14d", "r15d", "eax"};
static char *wreg_list[] = {"r12w", "r13w", "r14w", "r15w", "ax"};
static char *breg_list[] = {"r12b", "r13b", "r14b", "r15b", "al"};

static bss_symbol bss_symbols[MAX_BSS_SYMBOLS];
static int bss_symbol_count = 0;

static bool print_used = false;
static LabelId label_count = 0;

Register asm_RAX = (Register)4;
Register asm_NoReg = (Register)-1;

static bss_symbol *get_bss_symbol(char *name)
{
    for (int i = 0; i < bss_symbol_count; i++)
    {
        if (strcmp(bss_symbols[i].symbol_name, name) == 0)
            return &bss_symbols[i];
    }
    return NULL;
}

void asm_wrapup(CodeGenerator_t *gen)
{
    fputs("\n", gen->file);

    // Add runtime support for `printf` if used
    if (print_used)
    {
        fprintf(gen->file, "extern printf\n");
        fprintf(gen->file, "\n");
        fprintf(gen->file, "section .data\n");
        fprintf(gen->file, "\tformat db \"%%d\", 10, 0\n");
    }
    fprintf(gen->file, "extern print\n");
    fprintf(gen->file, "\n");
    fprintf(gen->file, "\n");

    // Generate the `.bss` section for uninitialized variables
    if (bss_symbol_count)
    {
        fprintf(gen->file, "section .bss\n");
        for (int i = 0; i < bss_symbol_count; i++)
        {
            if (bss_symbols[i].size == SIZE_8bit)
                fprintf(gen->file, "\t%s resb %d\n", bss_symbols[i].symbol_name, bss_symbols[i].number_of_items);
            else if (bss_symbols[i].size == SIZE_16bit)
                fprintf(gen->file, "\t%s resw %d\n", bss_symbols[i].symbol_name, bss_symbols[i].number_of_items);
            else if (bss_symbols[i].size == SIZE_32bit)
                fprintf(gen->file, "\t%s resd %d\n", bss_symbols[i].symbol_name, bss_symbols[i].number_of_items);
            else if (bss_symbols[i].size == SIZE_64bit)
                fprintf(gen->file, "\t%s resq %d\n", bss_symbols[i].symbol_name, bss_symbols[i].number_of_items);
        }
    }
    fprintf(gen->file, "section .note.GNU-stack noalloc noexec nowrite progbits");
}

static Register allocate_register(void)
{
    for (Register i = 0; i < GLOBAL_REG_COUNT; i++)
    {
        if (free_reg[i])
        {
            debug_print(SEV_DEBUG, "[ASM] Allocating register %s", reg_list[i]);
            free_reg[i] = 0;
            return i;
        }
    }
    debug_print(SEV_ERROR, "[ASM] Out of registers!\n");
    exit(1);
}

static void free_register(Register r)
{
    if (r >= GLOBAL_REG_COUNT)
    {
        debug_print(SEV_ERROR, "[ASM] Can't free special register %s", reg_list[r]);
        exit(1);
    }

    if (free_reg[r] != 0)
    {
        debug_print(SEV_ERROR, "[ASM] Error trying to free register %d\n", r);
        exit(1);
    }
    free_reg[r] = 1;
}

Register asm_init_register(CodeGenerator_t *gen, int value)
{
    Register r = allocate_register();
    fprintf(gen->file, "\tmov %s, %d\n", reg_list[r], value);
    return r;
}

void asm_set_register(CodeGenerator_t *gen, Register dest, Register src, RegSize_e size, bool free_src)
{
    if (size == SIZE_64bit)
        fprintf(gen->file, "\tmov %s, %s\n", reg_list[dest], reg_list[src]);
    else if (size == SIZE_32bit)
        fprintf(gen->file, "\tmov %s, %s\n", dreg_list[dest], dreg_list[src]);
    else if (size == SIZE_16bit)
        fprintf(gen->file, "\tmov %s, %s\n", wreg_list[dest], wreg_list[src]);
    else if (size == SIZE_8bit)
        fprintf(gen->file, "\tmov %s, %s\n", breg_list[dest], breg_list[src]);

    if (free_src)
        free_register(src);
}

Register asm_add(CodeGenerator_t *gen, Register r1, Register r2)
{
    fprintf(gen->file, "\tadd %s, %s\n", reg_list[r1], reg_list[r2]);
    free_register(r2);
    return r1;
}

Register asm_sub(CodeGenerator_t *gen, Register r1, Register r2)
{
    fprintf(gen->file, "\tsub %s, %s\n", reg_list[r1], reg_list[r2]);
    free_register(r2);
    return r1;
}

Register asm_mul(CodeGenerator_t *gen, Register r1, Register r2)
{
    fprintf(gen->file, "\timul %s, %s\n", reg_list[r1], reg_list[r2]);
    free_register(r2);
    return r1;
}

Register asm_div(CodeGenerator_t *gen, Register r1, Register r2)
{
    fprintf(gen->file, "\tmov rax, %s\n", reg_list[r1]);
    fprintf(gen->file, "\tcqo\n");
    fprintf(gen->file, "\tidiv %s\n", reg_list[r2]);
    fprintf(gen->file, "\tmov %s, rax\n", reg_list[r1]);
    free_register(r2);
    return r1;
}

static Register asm_comp(CodeGenerator_t *gen, Register r1, Register r2, char *func)
{
    fprintf(gen->file, "\tcmp %s, %s\n", reg_list[r1], reg_list[r2]);
    fprintf(gen->file, "\t%s %s\n", func, breg_list[r1]);
    fprintf(gen->file, "\tmovzx %s, %s \n", reg_list[r1], breg_list[r1]);
    free_register(r2);
    return r1;
}

Register asm_comp_eq(CodeGenerator_t *gen, Register r1, Register r2)
{
    return asm_comp(gen, r1, r2, "sete");
}

Register asm_comp_ne(CodeGenerator_t *gen, Register r1, Register r2)
{
    return asm_comp(gen, r1, r2, "setne");
}

Register asm_comp_gt(CodeGenerator_t *gen, Register r1, Register r2)
{
    return asm_comp(gen, r1, r2, "setg");
}

Register asm_comp_ge(CodeGenerator_t *gen, Register r1, Register r2)
{
    return asm_comp(gen, r1, r2, "setge");
}

Register asm_comp_lt(CodeGenerator_t *gen, Register r1, Register r2)
{
    return asm_comp(gen, r1, r2, "setl");
}

Register asm_comp_le(CodeGenerator_t *gen, Register r1, Register r2)
{
    return asm_comp(gen, r1, r2, "setle");
}

static void asm_jmp_with_cond(CodeGenerator_t *gen, Register r1, int comp_val, char *func, unsigned int label_number)
{
    fprintf(gen->file, "\tcmp %s, %d\n", reg_list[r1], comp_val);
    fprintf(gen->file, "\t%s __label__%d\n", func, label_number);
    free_register(r1);
}

void asm_jmp(CodeGenerator_t *gen, LabelId lbl)
{
    fprintf(gen->file, "\tjmp __label__%d\n", lbl);
}

void asm_jmp_eq(CodeGenerator_t *gen, Register r1, int comp_val, LabelId label_number)
{
    return asm_jmp_with_cond(gen, r1, comp_val, "je", label_number);
}

void asm_jmp_ne(CodeGenerator_t *gen, Register r1, int comp_val, LabelId label_number)
{
    return asm_jmp_with_cond(gen, r1, comp_val, "jne", label_number);
}

void asm_add_global_var(CodeGenerator_t *gen, char *var_name, RegSize_e size, size_t number_of_elements)
{
    if (number_of_elements == 0)
        number_of_elements = 1;
    if (get_bss_symbol(var_name) != NULL)
    {
        debug_print(SEV_ERROR, "[ASM] Redefinition of an existing bss symbol %s", var_name);
        exit(0);
    }
    debug_print(SEV_DEBUG, "Adding symbol %s in bss section", var_name);
    bss_symbols[bss_symbol_count].symbol_name = strdup(var_name);
    bss_symbols[bss_symbol_count].size = size;
    bss_symbols[bss_symbol_count].number_of_items = number_of_elements;
    bss_symbol_count++;
}

void asm_set_global_var(CodeGenerator_t *gen, char *var_name, Register r)
{
    bss_symbol *symbol = get_bss_symbol(var_name);
    if (symbol == NULL)
    {
        debug_print(SEV_ERROR, "[ASM] Symbol is not defined before!!");
        exit(1);
    }
    if (symbol->size == SIZE_64bit)
        fprintf(gen->file, "\tmov [%s], %s\n", var_name, reg_list[r]);
    else if (symbol->size == SIZE_32bit)
        fprintf(gen->file, "\tmov [%s], %s\n", var_name, dreg_list[r]);
    else if (symbol->size == SIZE_16bit)
        fprintf(gen->file, "\tmov [%s], %s\n", var_name, wreg_list[r]);
    else if (symbol->size == SIZE_8bit)
        fprintf(gen->file, "\tmov [%s], %s\n", var_name, breg_list[r]);
    free_register(r);
}

Register asm_get_global_var(CodeGenerator_t *gen, char *var_name)
{
    Register r = allocate_register();
    bss_symbol *symbol = get_bss_symbol(var_name);
    if (symbol == NULL)
    {
        debug_print(SEV_ERROR, "[ASM] Symbol is not defined before!!");
        exit(1);
    }
    // TODO: Check we need this xor command ??!!
    fprintf(gen->file, "\txor %s, %s\n", reg_list[r], reg_list[r]);

    if (symbol->size == SIZE_64bit)
        fprintf(gen->file, "\tmov %s, [%s]\n", reg_list[r], var_name);
    else if (symbol->size == SIZE_32bit)
        fprintf(gen->file, "\tmov %s, [%s]\n", dreg_list[r], var_name);
    else if (symbol->size == SIZE_16bit)
        fprintf(gen->file, "\tmov %s, [%s]\n", wreg_list[r], var_name);
    else if (symbol->size == SIZE_8bit)
        fprintf(gen->file, "\tmov %s, [%s]\n", breg_list[r], var_name);
    return r;
}

Register asm_address_of(CodeGenerator_t *gen, char *var_name)
{
    Register out = allocate_register();
    fprintf(gen->file, "\tlea %s, [%s]\n", reg_list[out], var_name);
    return out;
}

Register asm_load_mem(CodeGenerator_t *gen, Register addr, RegSize_e size)
{
    Register out = allocate_register();
    switch (size)
    {
    case SIZE_8bit:
        fprintf(gen->file, "\tmov %s, byte [%s]\n", breg_list[out], reg_list[addr]);
        break;
    case SIZE_16bit:
        fprintf(gen->file, "\tmov %s, word [%s]\n", wreg_list[out], reg_list[addr]);
        break;
    case SIZE_32bit:
        fprintf(gen->file, "\tmov %s, dword [%s]\n", dreg_list[out], reg_list[addr]);
        break;
    case SIZE_64bit:
        fprintf(gen->file, "\tmov %s, qword [%s]\n", reg_list[out], reg_list[addr]);
        break;
    }
    free_register(addr);
    return out;
}

void asm_store_mem(CodeGenerator_t *gen, Register addr, Register val, RegSize_e size)
{
    switch (size)
    {
    case SIZE_8bit:
        fprintf(gen->file, "\tmov byte [%s], %s\n", reg_list[addr], breg_list[val]);
        break;
    case SIZE_16bit:
        fprintf(gen->file, "\tmov word [%s], %s\n", reg_list[addr], wreg_list[val]);
        break;
    case SIZE_32bit:
        fprintf(gen->file, "\tmov dword [%s], %s\n", reg_list[addr], dreg_list[val]);
        break;
    case SIZE_64bit:
        fprintf(gen->file, "\tmov qword [%s], %s\n", reg_list[addr], reg_list[val]);
        break;
    }
    free_register(val);
    free_register(addr);
}

LabelId asm_generate_label()
{
    return label_count++;
}

void asm_lbl(CodeGenerator_t *gen, LabelId lbl_id)
{
    fprintf(gen->file, "__label__%d:\n", lbl_id);
}

void asm_generate_function_prologue(CodeGenerator_t *gen, char *func_name)
{
    fputs("section\t.text\n", gen->file);
    fprintf(gen->file, "global\t%s\n", func_name);
    fprintf(gen->file, "%s:\n", func_name);
    fputs("\tpush rbp\n", gen->file);
    fputs("\tmov rbp, rsp\n", gen->file);
}

void asm_generate_function_epilogue(CodeGenerator_t *gen)
{
    fputs("\tpop rbp\n", gen->file);
    fputs("\tret\n\n", gen->file);
}

void asm_generate_func_return(CodeGenerator_t *gen, Register r, RegSize_e size)
{
    if (size == SIZE_64bit)
        fprintf(gen->file, "\tmov %s, %s\n", reg_list[asm_RAX], reg_list[r]);
    else if (size == SIZE_32bit)
        fprintf(gen->file, "\tmov %s, %s\n", dreg_list[asm_RAX], dreg_list[r]);
    else if (size == SIZE_16bit)
        fprintf(gen->file, "\tmov %s, %s\n", wreg_list[asm_RAX], wreg_list[r]);
    else if (size == SIZE_8bit)
        fprintf(gen->file, "\tmov %s, %s\n", breg_list[asm_RAX], breg_list[r]);

    // TODO: uncomment this when handling register saving when doing function calls
    // free_register(r);
}

Register asm_generate_func_call(CodeGenerator_t *gen, char *func_name, Register arg1, bool need_return)
{
    // TODO: Check the size of the argument and use the correct reg for it
    Register out = allocate_register();
    if (arg1 != asm_NoReg)
        fprintf(gen->file, "\tmov rdi, %s\n", reg_list[arg1]);

    fprintf(gen->file, "\tcall %s\n", func_name);
    fprintf(gen->file, "\tmov %s,  rax\n", reg_list[out]);

    if (arg1 != asm_NoReg)
        free_register(arg1);

    if (need_return)
        return (out);
    else
    {
        free_register(out);
        return asm_NoReg;
    }
}
