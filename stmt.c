/**
 * @file stmt.c
 * @brief Implementation of statement parsing functions for the compiler.
 *
 * This file contains functions for parsing statements in the source code and
 * constructing an Abstract Syntax Tree (AST) representation. It supports parsing
 * variable declarations, assignments, `if` statements, blocks, and `print` statements.
 *
 * @project ToyCComp
 * @date 2024-12-24
 */

#include "stmt.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"
#include "symtab.h"
#include "expr.h"

static ASTNode_t *stmt_statements(Scanner_t *scanner);
static ASTNode_t *stmt_statement(Scanner_t *scanner);
static ASTNode_t *stmt_print(Scanner_t *scanner);
static ASTNode_t *stmt_var_decl(Scanner_t *scanner);
static ASTNode_t *stmt_assign(Scanner_t *scanner);
static ASTNode_t *stmt_if(Scanner_t *scanner);
static ASTNode_t *stmt_while(Scanner_t *scanner);
static ASTNode_t *stmt_do_while(Scanner_t *scanner);
static ASTNode_t *stmt_for(Scanner_t *scanner);
static ASTNode_t *stmt_break(Scanner_t *scanner);

/**
 * @brief Parses a sequence of statements.
 *
 * Constructs an AST for a sequence of statements until the end
 * of the source or a closing brace is encountered.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the root AST node representing the sequence of statements.
 */
static ASTNode_t *stmt_statements(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *head = stmt_statement(scanner);
    ASTNode_t *root;
    ASTNode_t *current;

    root = head;

    while (true)
    {
        scanner_peek(scanner, &tok);
        if (tok.type == TOK_EOF || tok.type == TOK_RBRACE)
        {
            break;
        }
        current = stmt_statement(scanner);
        head->next = current;
        head = current;
    }

    return root;
}

/**
 * @brief Parses a single statement.
 *
 * Determines the type of statement based on the next token and
 * delegates parsing to the appropriate handler.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the parsed statement.
 */
static ASTNode_t *stmt_statement(Scanner_t *scanner)
{
    Token_t t;
    scanner_peek(scanner, &t);
    switch (t.type)
    {
    case TOK_ID:
        if (strcmp(t.value.str_value, "print") == 0)
        {
            return stmt_print(scanner);
        }
        else
        {
            return stmt_assign(scanner);
        }
    case TOK_INT:
        return stmt_var_decl(scanner);
    case TOK_IF:
        return stmt_if(scanner);
    case TOK_WHILE:
        return stmt_while(scanner);
    case TOK_DO:
        return stmt_do_while(scanner);
    case TOK_BREAK:
        return stmt_break(scanner);
    case TOK_FOR:
        return stmt_for(scanner);
    case TOK_SEMICOLON:
        scanner_match(scanner, TOK_SEMICOLON);
        return ast_create_leaf_node(
            AST_EMPTY,
            0);
    default:
        debug_print(SEV_ERROR, "[STMT] Unexpected token: %s in stmt_statement", TokToString(t));
        exit(0);
    }
}

/**
 * @brief Parses a `print` statement.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the `print` statement.
 */
static ASTNode_t *stmt_print(Scanner_t *scanner)
{
    scanner_match(scanner, TOK_ID);
    scanner_match(scanner, TOK_LPAREN);
    ASTNode_t *expr = expr_expression(scanner);
    scanner_match(scanner, TOK_RPAREN);
    scanner_match(scanner, TOK_SEMICOLON);
    return ast_create_node(AST_PRINT, expr, NULL, 0);
}

/**
 * @brief Parses a variable declaration statement.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the variable declaration.
 */
static ASTNode_t *stmt_var_decl(Scanner_t *scanner)
{
    Token_t tok;

    scanner_match(scanner, TOK_INT);
    scanner_scan(scanner, &tok);
    if (tok.type != TOK_ID)
    {
        debug_print(SEV_ERROR, "Expected a TOK_ID token, found %s", TokToString(tok));
        exit(0);
    }
    int symbol_index = symtab_add_global_symbol(tok.value.str_value);
    scanner_match(scanner, TOK_SEMICOLON);

    return ast_create_node(
        AST_VAR_DECL,
        ast_create_leaf_node(AST_DATATYPE, 0),
        ast_create_leaf_node(AST_VAR, 0),
        symbol_index);
}

/**
 * @brief Parses an assignment statement.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the assignment.
 */
static ASTNode_t *stmt_assign(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *var;
    ASTNode_t *expr;

    scanner_scan(scanner, &tok);
    if (tok.type != TOK_ID)
    {
        debug_print(SEV_ERROR, "Expected a TOK_ID token, found %s", TokToString(tok));
        exit(1);
    }
    int var_symbol_index = symtab_find_global_symbol(tok.value.str_value);
    if (var_symbol_index < 0)
    {
        debug_print(SEV_ERROR, "Variable %s is not defined before", tok.value.str_value);
        exit(1);
    }
    var = ast_create_leaf_node(
        AST_VAR,
        var_symbol_index);

    scanner_match(scanner, TOK_ASSIGN);
    expr = expr_expression(scanner);
    scanner_match(scanner, TOK_SEMICOLON);

    return ast_create_node(
        AST_ASSIGN,
        var,
        expr,
        0);
}

/**
 * @brief Parses an `if` statement.
 *
 * Handles `if` statements, including optional `else` blocks
 * and nested `if` statements.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the `if` statement.
 */
static ASTNode_t *stmt_if(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *expr;
    ASTNode_t *true_code;
    ASTNode_t *false_code;

    scanner_match(scanner, TOK_IF);
    scanner_match(scanner, TOK_LPAREN);
    expr = expr_expression(scanner);
    scanner_match(scanner, TOK_RPAREN);
    true_code = stmt_block(scanner);

    scanner_scan(scanner, &tok);
    if (tok.type != TOK_ELSE)
    {
        scanner_putback(scanner, &tok);
        return ast_create_node(
            AST_IF,
            expr,
            ast_create_node(
                AST_GLUE,
                true_code,
                NULL,
                0),
            0);
    }
    scanner_peek(scanner, &tok);
    if (tok.type == TOK_IF)
    {
        false_code = stmt_if(scanner);
    }
    else
    {
        false_code = stmt_block(scanner);
    }

    return ast_create_node(
        AST_IF,
        expr,
        ast_create_node(
            AST_GLUE,
            true_code,
            false_code,
            0),
        0);
}

/**
 * @brief Parses a `while` statement and constructs its abstract syntax tree (AST) representation.
 *
 * Processes tokens from the scanner to parse a `while` statement,
 * including its condition and body, and generates an AST node representing the `while` construct.
 *
 * @param scanner A pointer to the `Scanner_t` structure used for tokenizing the input source code.
 * @return ASTNode_t* A pointer to the AST node representing the `while` statement.
 *                    - The left child node contains the loop condition.
 *                    - The right child node contains the loop body.
 *
 * @details
 * - The function expects the `while` statement to follow this syntax:
 *   `while (<expression>) <block>`
 * - An AST node of type `AST_WHILE` is created with the condition as the left child
 *   and the loop body as the right child.
 */
static ASTNode_t *stmt_while(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *expr, *code;

    scanner_match(scanner, TOK_WHILE);
    scanner_match(scanner, TOK_LPAREN);
    expr = expr_expression(scanner);
    scanner_match(scanner, TOK_RPAREN);
    code = stmt_block(scanner);

    return ast_create_node(
        AST_WHILE,
        expr,
        code,
        0);
}

/**
 * @brief Parses a `do-while` statement and constructs its abstract syntax tree (AST) representation.
 *
 * Processes tokens from the scanner to parse a `do-while` statement,
 * including its body and condition, and generates an AST node representing the `do-while` construct.
 *
 * @param scanner A pointer to the `Scanner_t` structure used for tokenizing the input source code.
 * @return ASTNode_t* A pointer to the AST node representing the `do-while` statement.
 *                    - The right child node contains the loop body.
 *                    - The left child node contains the loop condition.
 *
 * @details
 * - The function expects the `do-while` statement to follow this syntax:
 *   `do { <block> } while (<expression>);`
 * - An AST node of type `AST_DO_WHILE` is created with the condition as the left child
 *   and the loop body as the right child.
 */
static ASTNode_t *stmt_do_while(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *expr, *code;

    scanner_match(scanner, TOK_DO);
    code = stmt_block(scanner);
    scanner_match(scanner, TOK_WHILE);
    scanner_match(scanner, TOK_LPAREN);
    expr = expr_expression(scanner);
    scanner_match(scanner, TOK_RPAREN);
    scanner_match(scanner, TOK_SEMICOLON);

    return ast_create_node(
        AST_DO_WHILE,
        expr,
        code,
        0);
}

/**
 * @brief Parses a `for` statement and constructs its abstract syntax tree (AST) representation.
 *
 * This function processes tokens from the scanner to parse a `for` loop, including its
 * initialization, condition, update expressions, and body, and generates an AST node
 * representing the `for` construct.
 *
 * @param scanner A pointer to the `Scanner_t` structure used for tokenizing the input source code.
 * @return ASTNode_t* A pointer to the AST node representing the `for` statement.
 *                    - The left child node (`pre_post`) contains:
 *                      1. The initialization statement or expression.
 *                      2. The condition expression (linked via `next`).
 *                      3. The update expression (linked via `next->next`).
 *                    - The right child node contains the loop body as a block.
 *
 * @details
 * - The function expects the `for` statement to follow this syntax:
 *   `for (<init>; <condition>; <update>) <block>`
 */
static ASTNode_t *stmt_for(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *pre_post, *code;

    scanner_match(scanner, TOK_FOR);
    scanner_match(scanner, TOK_LPAREN);
    pre_post = stmt_statement(scanner);
    pre_post->next = expr_expression(scanner);
    scanner_match(scanner, TOK_SEMICOLON);
    pre_post->next->next = stmt_statement(scanner);
    scanner_match(scanner, TOK_RPAREN);
    code = stmt_block(scanner);

    return ast_create_node(
        AST_FOR,
        pre_post,
        code,
        0);
}

/**
 * @brief Parses a `break` statement and constructs its abstract syntax tree (AST) representation.
 *
 * Processes tokens from the scanner to parse a `break` statement and generates
 * a leaf AST node representing the `break` construct.
 *
 * @param scanner A pointer to the `Scanner_t` structure used for tokenizing the input source code.
 * @return ASTNode_t* A pointer to the AST leaf node representing the `break` statement.
 *                    - The node is of type `AST_BREAK`.
 *                    - The `value` field of the node is set to 0.
 */
static ASTNode_t *stmt_break(Scanner_t *scanner)
{
    scanner_match(scanner, TOK_BREAK);
    scanner_match(scanner, TOK_SEMICOLON);
    return ast_create_leaf_node(AST_BREAK, 0);
}

/**
 * @brief Parses a block of statements enclosed by braces.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the AST node representing the block.
 */
ASTNode_t *stmt_block(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *out;

    scanner_peek(scanner, &tok);
    if (tok.type == TOK_LBRACE)
    {
        scanner_match(scanner, TOK_LBRACE);
        out = stmt_statements(scanner);
        scanner_match(scanner, TOK_RBRACE);
    }
    else
    {
        out = stmt_statement(scanner);
    }
    return out;
}