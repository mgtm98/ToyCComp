/**
 * @file expr.c
 * @brief Implementation of expression parsing functions for the compiler.
 *
 * This file contains functions for parsing expressions in the source code
 * and constructing the Abstract Syntax Tree (AST) representation. It follows
 * the BNF grammar defined for the ToyCComp language.
 *
 * @project ToyCComp
 * @author Mohamed Gamal
 * @date 2024-12-24
 */

#include "scanner.h"
#include "ast.h"
#include "debug.h"
#include "symtab.h"

#include <stdbool.h>
#include <stdlib.h>

static ASTNode_type_e get_node_type(TokenType_e type);

static ASTNode_t *expr_comparison_expression(Scanner_t *scanner);
static ASTNode_t *expr_additive_expression(Scanner_t *scanner);
static ASTNode_t *expr_multiplicative_expression(Scanner_t *scanner);
static ASTNode_t *expr_val(Scanner_t *scanner);
static ASTNode_t *expr_val_intlit(Scanner_t *scanner);
static ASTNode_t *expr_val_var(Scanner_t *scanner);
static ASTNode_t *expr_val_expr(Scanner_t *scanner);
ASTNode_t *expr_expression(Scanner_t *scanner);

/**
 * @brief Maps a token type to its corresponding AST node type.
 *
 * This function converts token types representing operators or comparison
 * symbols into their equivalent Abstract Syntax Tree (AST) node types.
 *
 * ### Supported Mappings:
 * - Arithmetic operators:
 *   - `TOK_PLUS` -> `AST_ADD`
 *   - `TOK_MINUS` -> `AST_SUBTRACT`
 *   - `TOK_STAR` -> `AST_MULT`
 *   - `TOK_SLASH` -> `AST_DIV`
 * - Comparison operators:
 *   - `TOK_GT` -> `AST_COMP_GT`
 *   - `TOK_GE` -> `AST_COMP_GE`
 *   - `TOK_LT` -> `AST_COMP_LT`
 *   - `TOK_LE` -> `AST_COMP_LE`
 *   - `TOK_EQ` -> `AST_COMP_EQ`
 *   - `TOK_NE` -> `AST_COMP_NE`
 *
 * @param type Token type to map.
 * @return Corresponding AST node type.
 *
 * @note If an unsupported token type is provided, the function logs an error
 *       and does not return a value.
 */
static ASTNode_type_e get_node_type(TokenType_e type)
{
    switch (type)
    {
    case TOK_PLUS:
        return AST_ADD;
    case TOK_MINUS:
        return AST_SUBTRACT;
    case TOK_STAR:
        return AST_MULT;
    case TOK_SLASH:
        return AST_DIV;
    case TOK_GT:
        return AST_COMP_GT;
    case TOK_GE:
        return AST_COMP_GE;
    case TOK_LT:
        return AST_COMP_LT;
    case TOK_LE:
        return AST_COMP_LE;
    case TOK_EQ:
        return AST_COMP_EQ;
    case TOK_NE:
        return AST_COMP_NE;
    default:
        debug_print(SEV_ERROR, "Unexpected token type %d", type);
    }
}

/**
 * @brief Parses a value (BNF: val).
 *
 * Handles numbers, identifiers, or parenthesized expressions.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the value.
 */
static ASTNode_t *expr_val(Scanner_t *scanner)
{
    Token_t token;
    scanner_scan(scanner, &token);
    scanner_putback(scanner, &token);
    switch (token.type)
    {
    case TOK_INTLIT:
        return expr_val_intlit(scanner);
    case TOK_ID:
        return expr_val_var(scanner);
    case TOK_LPAREN:
        return expr_val_expr(scanner);
    default:
        debug_print(SEV_ERROR, "Expected a number or id token, got %s", TokToString(token));
        exit(1);
    }
}

/**
 * @brief Parses an integer literal (BNF: number).
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the integer literal.
 */
static ASTNode_t *expr_val_intlit(Scanner_t *scanner)
{
    Token_t token;
    scanner_scan(scanner, &token);
    return ast_create_leaf_node(
        AST_INT_LIT,
        token.value.int_value);
}

/**
 * @brief Parses an identifier (BNF: identifier).
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the variable.
 */
static ASTNode_t *expr_val_var(Scanner_t *scanner)
{
    Token_t token;
    scanner_scan(scanner, &token);
    int var_symbol_index = symtab_find_global_symbol(token.value.str_value);
    if (var_symbol_index < 0)
    {
        debug_print(SEV_ERROR, "Variable %s is not defined before", token.value.str_value);
        exit(1);
    }
    return ast_create_leaf_node(
        AST_VAR,
        var_symbol_index);
}

/**
 * @brief Parses a parenthesized expression (BNF: '(' expression ')').
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the expression.
 */
static ASTNode_t *expr_val_expr(Scanner_t *scanner)
{
    Token_t token;
    ASTNode_t *expr;
    scanner_match(scanner, TOK_LPAREN);
    expr = expr_expression(scanner);
    scanner_match(scanner, TOK_RPAREN);
    return expr;
}

/**
 * @brief Parses a multiplicative expression (BNF: multiplicative_expression).
 *
 * Handles values combined with '*' or '/' operators.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the multiplicative expression.
 */
static ASTNode_t *expr_multiplicative_expression(Scanner_t *scanner)
{
    Token_t token;
    ASTNode_t *left, *right, *temp;
    ASTNode_type_e type;

    left = expr_val(scanner);

    while (scanner_scan(scanner, &token))
    {
        if (token.type == TOK_STAR || token.type == TOK_SLASH)
        {
            type = get_node_type(token.type);
            right = expr_val(scanner);
            left = ast_create_node(type, left, right, 0);
        }
        else
        {
            scanner_putback(scanner, &token);
            return left;
        }
    }

    return left;
}

/**
 * @brief Parses an additive expression (BNF: additive_expression).
 *
 * Handles multiplicative expressions combined with '+' or '-' operators.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the additive expression.
 */
static ASTNode_t *expr_additive_expression(Scanner_t *scanner)
{
    Token_t token;
    ASTNode_t *left, *right;
    ASTNode_type_e type;

    left = expr_multiplicative_expression(scanner);

    while (scanner_scan(scanner, &token))
    {
        if (token.type == TOK_PLUS || token.type == TOK_MINUS)
        {
            type = get_node_type(token.type);
            right = expr_multiplicative_expression(scanner);
            left = ast_create_node(type, left, right, 0);
        }
        else
        {
            scanner_putback(scanner, &token);
            return left;
        }
    }

    return left;
}

/**
 * @brief Parses a comparison expression (BNF: comparison_expression).
 *
 * Handles additive expressions combined with comparison operators.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the comparison expression.
 */
static ASTNode_t *expr_comparison_expression(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *left;
    ASTNode_t *right;
    left = expr_additive_expression(scanner);
    scanner_scan(scanner, &tok);
    if (
        tok.type == TOK_EQ ||
        tok.type == TOK_NE ||
        tok.type == TOK_GT ||
        tok.type == TOK_GE ||
        tok.type == TOK_LT ||
        tok.type == TOK_LE)
    {
        right = expr_additive_expression(scanner);
        return ast_create_node(
            get_node_type(tok.type),
            left,
            right,
            0);
    }
    else
    {
        scanner_putback(scanner, &tok);
        return left;
    }
}

/**
 * @brief Parses an expression (BNF: expression).
 *
 * This function serves as the entry point for parsing expressions, starting with
 * comparison expressions.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the parsed expression.
 */
ASTNode_t *expr_expression(Scanner_t *scanner)
{
    return expr_comparison_expression(scanner);
}
