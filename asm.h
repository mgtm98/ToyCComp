#ifndef _ASM_H_
#define _ASM_H_

#include <stdio.h>

#include "codegen.h"

typedef __int8_t Register;
typedef __uint32_t LabelId;

// ============================================================
//                     Module initialization
// ============================================================

/**
 * @brief Initializes the assembly file for code generation.
 *
 * Writes the initial setup for the assembly file, including the `.text` section
 * and the `main` label, which serves as the entry point for
 * the program.
 *
 * @param gen Pointer to the code generator context.
 */
void asm_init(CodeGenerator_t *gen);

/**
 * @brief Finalizes the assembly file for code generation.
 *
 * Writes the closing assembly instructions, including the system call
 * to terminate the program, and generates the global, data, and `.bss` sections as needed.
 *
 * @param gen Pointer to the code generator context.
 */
void asm_wrapup(CodeGenerator_t *gen);

// ============================================================
//                     Register Allocation
// ============================================================

/**
 * @brief Allocates a register and initializes it with a given value.
 *
 * Allocates a free register and writes an assembly instruction to move the
 * specified value into the allocated register.
 *
 * Example of generated assembly:
 * ```
 * mov r12, 10
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param value The integer value to initialize the register with.
 * @return The allocated register.
 *
 * @warning If no registers are available, the program will terminate with an error.
 */
Register asm_init_register(CodeGenerator_t *gen, int value);

// ============================================================
//                     Math Operations
// ============================================================

/**
 * @brief Performs addition and stores the result in the first register.
 *
 * Emits an assembly instruction to add the value of the second register to the first
 * register. The second register is freed after the operation.
 *
 * Example of generated assembly:
 * ```
 * add r12, r13
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The register to hold the result of the addition.
 * @param r2 The register containing the value to add.
 * @return The first register (`r1`) containing the result.
 *
 * @warning If `r2` is already free, the program will terminate with an error.
 */
Register asm_add(CodeGenerator_t *gen, Register r1, Register r2);

/**
 * @brief Performs subtraction and stores the result in the first register.
 *
 * Emits an assembly instruction to subtract the value of the second register from
 * the first register. The second register is freed after the operation.
 *
 * Example of generated assembly:
 * ```
 * sub r12, r13  ; Subtracts the value in r13 from r12 and stores the result in r12
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The register to hold the result of the subtraction.
 * @param r2 The register containing the value to subtract.
 * @return The first register (`r1`) containing the result.
 *
 * @warning If `r2` is already free, the program will terminate with an error.
 */
Register asm_sub(CodeGenerator_t *gen, Register r1, Register r2);

/**
 * @brief Performs multiplication and stores the result in the first register.
 *
 * Emits an assembly instruction to multiply the values in the two registers and
 * stores the result in the first register. The second register is freed after the operation.
 *
 * Example of generated assembly:
 * ```
 * imul r12, r13  ; Multiplies the values in r12 and r13, stores the result in r12
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The register to hold the result of the multiplication.
 * @param r2 The register containing the value to multiply.
 * @return The first register (`r1`) containing the result.
 *
 * @warning If `r2` is already free, the program will terminate with an error.
 */
Register asm_mul(CodeGenerator_t *gen, Register r1, Register r2);

/**
 * @brief Performs integer division and stores the result in the first register.
 *
 * Emits assembly instructions to divide the value in the first register by the
 * value in the second register. The quotient is stored in the first register, and
 * the second register is freed after the operation.
 *
 * Example of generated assembly:
 * ```
 * mov rax, r12  ; Move the dividend into rax
 * cqo           ; Sign-extend rax into rdx:rax
 * idiv r13      ; Divide rdx:rax by r13
 * mov r12, rax  ; Move the quotient back into r12
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The register containing the dividend and to hold the quotient.
 * @param r2 The register containing the divisor.
 * @return The first register (`r1`) containing the quotient.
 *
 * @warning Division by zero is not checked and will cause a runtime error.
 * @warning If `r2` is already free, the program will terminate with an error.
 */
Register asm_div(CodeGenerator_t *gen, Register r1, Register r2);

// ============================================================
//                     Comparison Operations
// ============================================================

/**
 * @brief Emits assembly code for equality comparison (`==`).
 *
 * Example of generated assembly:
 * ```
 * cmp r12, r13  ; Compare the values in r12 and r13
 * sete r12b     ; Set the result of equality comparison (1 if equal, 0 otherwise)
 * movzx r12, r12b  ; Zero-extend the result into the full register
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The first register and the destination for the comparison result.
 * @param r2 The second register for comparison.
 * @return The first register (`r1`) containing the result (1 if equal, 0 otherwise).
 */
Register asm_comp_eq(CodeGenerator_t *gen, Register r1, Register r2);

/**
 * @brief Emits assembly code for inequality comparison (`!=`).
 *
 * Example of generated assembly:
 * ```
 * cmp r12, r13  ; Compare the values in r12 and r13
 * setne r12b    ; Set the result of inequality comparison (1 if not equal, 0 otherwise)
 * movzx r12, r12b  ; Zero-extend the result into the full register
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The first register and the destination for the comparison result.
 * @param r2 The second register for comparison.
 * @return The first register (`r1`) containing the result (1 if not equal, 0 otherwise).
 */
Register asm_comp_ne(CodeGenerator_t *gen, Register r1, Register r2);

/**
 * @brief Emits assembly code for greater-than comparison (`>`).
 *
 * Example of generated assembly:
 * ```
 * cmp r12, r13  ; Compare the values in r12 and r13
 * setg r12b     ; Set the result of greater-than comparison (1 if r12 > r13, 0 otherwise)
 * movzx r12, r12b  ; Zero-extend the result into the full register
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The first register and the destination for the comparison result.
 * @param r2 The second register for comparison.
 * @return The first register (`r1`) containing the result (1 if greater, 0 otherwise).
 */
Register asm_comp_gt(CodeGenerator_t *gen, Register r1, Register r2);

/**
 * @brief Emits assembly code for greater-than-or-equal comparison (`>=`).
 *
 * Example of generated assembly:
 * ```
 * cmp r12, r13  ; Compare the values in r12 and r13
 * setge r12b    ; Set the result of greater-than-or-equal comparison (1 if r12 >= r13, 0 otherwise)
 * movzx r12, r12b  ; Zero-extend the result into the full register
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The first register and the destination for the comparison result.
 * @param r2 The second register for comparison.
 * @return The first register (`r1`) containing the result (1 if greater or equal, 0 otherwise).
 */
Register asm_comp_ge(CodeGenerator_t *gen, Register r1, Register r2);

/**
 * @brief Emits assembly code for less-than comparison (`<`).
 *
 * Example of generated assembly:
 * ```
 * cmp r12, r13  ; Compare the values in r12 and r13
 * setl r12b     ; Set the result of less-than comparison (1 if r12 < r13, 0 otherwise)
 * movzx r12, r12b  ; Zero-extend the result into the full register
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The first register and the destination for the comparison result.
 * @param r2 The second register for comparison.
 * @return The first register (`r1`) containing the result (1 if less, 0 otherwise).
 */
Register asm_comp_lt(CodeGenerator_t *gen, Register r1, Register r2);

/**
 * @brief Emits assembly code for less-than-or-equal comparison (`<=`).
 *
 * Example of generated assembly:
 * ```
 * cmp r12, r13  ; Compare the values in r12 and r13
 * setle r12b    ; Set the result of less-than-or-equal comparison (1 if r12 <= r13, 0 otherwise)
 * movzx r12, r12b  ; Zero-extend the result into the full register
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The first register and the destination for the comparison result.
 * @param r2 The second register for comparison.
 * @return The first register (`r1`) containing the result (1 if less or equal, 0 otherwise).
 */
Register asm_comp_le(CodeGenerator_t *gen, Register r1, Register r2);

// ============================================================
//                     Label Management
// ============================================================

/**
 * @brief Generates a unique label ID for use in assembly code.
 *
 * This function increments an internal counter to produce a unique
 * label ID each time it is called. The returned label ID can be
 * used to define and reference labels in the generated assembly code.
 *
 * @return LabelId A unique identifier for an assembly label.
 */
LabelId asm_generate_label();

/**
 * @brief Outputs an assembly label to the specified code generator file.
 *
 * This function writes the definition of a label to the file associated
 * with the given code generator. The label is formatted as `__label__<id>:`,
 * where `<id>` is the unique label ID generated by `asm_generate_label`.
 *
 * @param gen A pointer to the CodeGenerator_t structure that contains
 *            the output file stream.
 * @param lbl_id The unique label ID to define in the assembly code.
 */
void asm_lbl(CodeGenerator_t *gen, LabelId lbl);

// ============================================================
//                     Jumb Oprtations
// ============================================================

/**
 * @brief Emits an unconditional jump assembly instruction.
 *
 * Emits an assembly instruction to jump to the specified label unconditionally.
 *
 * Example of generated assembly:
 * ```
 * jmp __label__5  ; Unconditionally jump to label 5
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param lbl The label ID to jump to.
 */
void asm_jmp(CodeGenerator_t *gen, LabelId lbl);

/**
 * @brief Emits a conditional jump if the values are equal.
 *
 * Compares the value in a register with a specified constant and emits a conditional
 * jump instruction to the specified label if the values are equal.
 *
 * Example of generated assembly:
 * ```
 * cmp r12, 0   ; Compare the value in r12 with 0
 * je __label__5 ; Jump to label 5 if the comparison result is equal
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The register containing the value to compare.
 * @param comp_val The constant value to compare against.
 * @param label_number The label ID to jump to if the condition is met.
 */
void asm_jmp_eq(CodeGenerator_t *gen, Register r, int val, LabelId lbl);

/**
 * @brief Emits a conditional jump if the values are not equal.
 *
 * Compares the value in a register with a specified constant and emits a conditional
 * jump instruction to the specified label if the values are not equal.
 *
 * Example of generated assembly:
 * ```
 * cmp r12, 0   ; Compare the value in r12 with 0
 * jne __label__5 ; Jump to label 5 if the comparison result is not equal
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param r1 The register containing the value to compare.
 * @param comp_val The constant value to compare against.
 * @param label_number The label ID to jump to if the condition is met.
 */
void asm_jmp_ne(CodeGenerator_t *gen, Register r, int val, LabelId lbl);
// void asm_jmp_gt(CodeGenerator_t *gen, Register r1, Register r2, LabelId lbl);
// void asm_jmp_ge(CodeGenerator_t *gen, Register r1, Register r2, LabelId lbl);
// void asm_jmp_lt(CodeGenerator_t *gen, Register r1, Register r2, LabelId lbl);
// void asm_jmp_le(CodeGenerator_t *gen, Register r1, Register r2, LabelId lbl);

// ============================================================
//                     Global Variables
// ============================================================

/**
 * @brief Adds a global variable to the `.bss` section.
 *
 * Checks if a variable with the same name already exists in the `.bss` section. If not,
 * adds the variable and reserves space for it. Logs an error and terminates if the
 * variable is redefined.
 *
 * Example of generated assembly:
 * ```
 * section .bss
 * var_name resq 1  ; Reserve space for one quadword
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param var_name The name of the global variable to add.
 *
 * @warning If the variable is already defined, the program terminates with an error.
 */
void asm_add_global_var(CodeGenerator_t *gen, char *var_name);

/**
 * @brief Sets the value of a global variable in memory.
 *
 * Moves the value from a register into the memory location associated with the specified
 * global variable. Frees the register after use.
 *
 * Example of generated assembly:
 * ```
 * mov [var_name], r12  ; Store the value in r12 into the variable var_name
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param var_name The name of the global variable.
 * @param r The register containing the value to store.
 */
void asm_set_global_var(CodeGenerator_t *gen, char *var_name, Register r);

/**
 * @brief Retrieves the value of a global variable into a register.
 *
 * Allocates a register and loads the value of the specified global variable into it.
 *
 * Example of generated assembly:
 * ```
 * mov r12, [var_name]  ; Load the value of var_name into r12
 * ```
 *
 * @param gen Pointer to the code generator context.
 * @param var_name The name of the global variable.
 * @return The register containing the value of the global variable.
 */
Register asm_get_global_var(CodeGenerator_t *gen, char *var_name);

// ============================================================
//                     Runtime Library
// ============================================================

/**
 * @brief Emits assembly instructions to print the value of a register.
 *
 * This function generates assembly code to print the value stored in a given register.
 * It follows the ABI (Application Binary Interface) conventions for calling the `printf`
 * function, including setting up the necessary arguments and ensuring compliance with
 * calling conventions (e.g., register usage and alignment requirements).
 *
 * @param gen A pointer to the CodeGenerator_t structure that contains the output file stream.
 * @param r The register whose value is to be printed.
 *
 * @see For more details on ABI conventions, refer to:
 *   https://en.wikipedia.org/wiki/Application_binary_interfac
 */
void asm_print(CodeGenerator_t *gen, Register r);

#endif