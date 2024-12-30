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
} bss_symbol;

static int free_reg[GLOBAL_REG_COUNT] = {1, 1, 1, 1};
static char *reg_list[GLOBAL_REG_COUNT] = {"r12", "r13", "r14", "r15"};
static char *breg_list[GLOBAL_REG_COUNT] = {"r12b", "r13b", "r14b", "r15b"};

static bss_symbol bss_symbols[MAX_BSS_SYMBOLS];
static int bss_symbol_count = 0;

static bool print_used = false;
static LabelId label_count = 0;

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
    fprintf(gen->file, "\n");
    fprintf(gen->file, "\n");

    // Generate the `.bss` section for uninitialized variables
    if (bss_symbol_count)
    {
        fprintf(gen->file, "section .bss\n");
        for (int i = 0; i < bss_symbol_count; i++)
        {
            fprintf(gen->file, "\t%s resq 1\n", bss_symbols[i].symbol_name);
        }
    }
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

void asm_add_global_var(CodeGenerator_t *gen, char *var_name)
{
    for (int i = 0; i < bss_symbol_count; i++)
    {
        if (strcmp(bss_symbols[i].symbol_name, var_name) == 0)
        {
            debug_print(SEV_ERROR, "[ASM] Redefinition of an existing bss symbol %s", var_name);
            exit(0);
        }
    }
    debug_print(SEV_DEBUG, "Adding symbol %s in bss section", var_name);
    bss_symbols[bss_symbol_count]
        .symbol_name = strdup(var_name);
    bss_symbol_count++;
}

void asm_set_global_var(CodeGenerator_t *gen, char *var_name, Register r)
{
    fprintf(gen->file, "\tmov [%s], %s\n", var_name, reg_list[r]);
    free_register(r);
}

Register asm_get_global_var(CodeGenerator_t *gen, char *var_name)
{
    Register r = allocate_register();
    fprintf(gen->file, "\tmov %s, [%s]\n", reg_list[r], var_name);
    return r;
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
    fputs("\tmov eax, 0\n", gen->file);
    fputs("\tpop rbp\n", gen->file);
    fputs("\tret\n", gen->file);
}

void asm_print(CodeGenerator_t *gen, Register r)
{
    fprintf(gen->file, "\n\tmov rsi, %s\n", reg_list[r]);
    fprintf(gen->file, "\tlea rdi, [rel format]\n");
    fprintf(gen->file, "\txor rax, rax\n");
    fprintf(gen->file, "\tcall printf\n\n");
    free_register(r);
    print_used = true;
}
