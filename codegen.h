#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "ast.h"

#include <stdio.h>

/**
 * @brief Code generator context.
 *
 * The `CodeGenerator_t` structure holds the context for the code generation
 * process, including the output file where the generated assembly code is written.
 */
typedef struct
{
    FILE *file; /**< Pointer to the output file for generated assembly code. */
} CodeGenerator_t;

/**
 * @brief Initializes the code generator.
 *
 * Allocates and initializes a `CodeGenerator_t` object for generating assembly code.
 * Opens the specified file for writing the output.
 *
 * @param path Pointer to the string representing the file path where the generated
 *        assembly code will be written.
 * @return Pointer to the initialized `CodeGenerator_t` object.
 */
CodeGenerator_t *codegen_init(char *path);

/**
 * @brief Starts the code generation process.
 *
 * This function serves as the entry point for generating assembly code. It initializes
 * the code generation process, generates the assembly code for the given Abstract Syntax
 * Tree (AST), and finalizes the output.
 *
 * @param gen Pointer to the code generator context.
 * @param root Pointer to the root node of the AST representing the program.
 */
void codegen_start(CodeGenerator_t *gen, ASTNode_t *root);

#endif // _CODEGEN_H_
