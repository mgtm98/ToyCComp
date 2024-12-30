#ifndef _STMT_H_
#define _STMT_H_

#include "ast.h"
#include "scanner.h"

/**
 * @file stmt.h
 * @brief Header file for statement parsing functions.
 *
 * This file declares functions for parsing statements in the source code and
 * constructing the corresponding Abstract Syntax Tree (AST).
 *
 * @project ToyCComp
 * @author Mohamed Gamal
 * @date 2024-12-24
 */

/**
 * @brief Parses a block of statements enclosed by braces.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the block.
 */
ASTNode_t *stmt_block(Scanner_t *scanner);

#endif
