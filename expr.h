#ifndef _EXPR_H_
#define _EXPR_H_

#include "scanner.h"
#include "ast.h"
#include "debug.h"

/**
 * @file expr.h
 * @brief Header file for expression parsing functions.
 *
 * This file declares functions for parsing expressions in the source code
 * and constructing the corresponding Abstract Syntax Tree (AST).
 *
 * @project ToyCComp
 * @author Mohamed Gamal
 * @date 2024-12-24
 */

/**
 * @brief Parses an expression (BNF: expression).
 *
 * Entry point for parsing expressions in the source code.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the parsed expression.
 */
ASTNode_t *expr_expression(Scanner_t *scanner);

#endif
