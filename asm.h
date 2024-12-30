#ifndef _ASM_H_
#define _ASM_H_

#include <stdio.h>

#include "codegen.h"

typedef __int8_t Register;
typedef __uint32_t LabelId;

void asm_wrapup(CodeGenerator_t *gen);

Register asm_init_register(CodeGenerator_t *gen, int value);

Register asm_add(CodeGenerator_t *gen, Register r1, Register r2);
Register asm_sub(CodeGenerator_t *gen, Register r1, Register r2);
Register asm_mul(CodeGenerator_t *gen, Register r1, Register r2);
Register asm_div(CodeGenerator_t *gen, Register r1, Register r2);

Register asm_comp_eq(CodeGenerator_t *gen, Register r1, Register r2);
Register asm_comp_ne(CodeGenerator_t *gen, Register r1, Register r2);
Register asm_comp_gt(CodeGenerator_t *gen, Register r1, Register r2);
Register asm_comp_ge(CodeGenerator_t *gen, Register r1, Register r2);
Register asm_comp_lt(CodeGenerator_t *gen, Register r1, Register r2);
Register asm_comp_le(CodeGenerator_t *gen, Register r1, Register r2);

LabelId asm_generate_label();
void asm_lbl(CodeGenerator_t *gen, LabelId lbl);

void asm_jmp(CodeGenerator_t *gen, LabelId lbl);
void asm_jmp_eq(CodeGenerator_t *gen, Register r, int val, LabelId lbl);
void asm_jmp_ne(CodeGenerator_t *gen, Register r, int val, LabelId lbl);
// void asm_jmp_gt(CodeGenerator_t *gen, Register r1, Register r2, LabelId lbl);
// void asm_jmp_ge(CodeGenerator_t *gen, Register r1, Register r2, LabelId lbl);
// void asm_jmp_lt(CodeGenerator_t *gen, Register r1, Register r2, LabelId lbl);
// void asm_jmp_le(CodeGenerator_t *gen, Register r1, Register r2, LabelId lbl);

void asm_add_global_var(CodeGenerator_t *gen, char *var_name);
void asm_set_global_var(CodeGenerator_t *gen, char *var_name, Register r);
Register asm_get_global_var(CodeGenerator_t *gen, char *var_name);

void asm_generate_function_prologue(CodeGenerator_t *gen, char *func_name);
void asm_generate_function_epilogue(CodeGenerator_t *gen);

void asm_print(CodeGenerator_t *gen, Register r);

#endif