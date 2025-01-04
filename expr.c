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
#include "datatype.h"
#include "decl.h"
#include "llist_definitions.h"

#include <assert.h>
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
static ASTNode_t *expr_func_call(Scanner_t *scanner);

ASTNode_t *expr_assignment(Scanner_t *scanner);
ASTNode_t *expr_expression(Scanner_t *scanner);

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
        debug_print(SEV_ERROR, "[EXPR] Unexpected token type %d", type);
    }
}

static ASTNode_t *expr_val(Scanner_t *scanner)
{
    Token_t token;
    scanner_peek(scanner, &token);
    switch (token.type)
    {
    case TOK_INTLIT:
        return expr_val_intlit(scanner);
    case TOK_ID:
        return expr_val_var(scanner);
    case TOK_LPAREN:
        return expr_val_expr(scanner);
    default:
        debug_print(SEV_ERROR, "[EXPR] Expected a number or id token, got %s", TokToString(token));
        exit(1);
    }
}

// TODO: support signed numbers
static ASTNode_t *expr_val_intlit(Scanner_t *scanner)
{
    Token_t token;
    Datatype_t *datatype;
    ASTNode_t *node;
    scanner_scan(scanner, &token);

    if (0 <= token.value.int_value && token.value.int_value < 256)
        datatype = datatype_get_primative_type(DT_CHAR);
    else if (token.value.int_value >= 256)
        datatype = datatype_get_primative_type(DT_INT);
    else
    {
        debug_print(SEV_ERROR, "[EXPR] Signed numbers aren't supported yet!!");
        exit(1);
    }

    node = ast_create_leaf_node(AST_INT_LIT, token.value.int_value);
    node->expr_type = datatype;
    return node;
}

static ASTNode_t *expr_val_var(Scanner_t *scanner)
{
    Token_t token;
    ASTNode_t *expr;
    scanner_scan(scanner, &token);
    int var_symbol_index = symtab_find_global_symbol(token.value.str_value);
    if (var_symbol_index < 0)
    {
        debug_print(SEV_ERROR, "[EXPR] Variable %s is not defined before", token.value.str_value);
        exit(1);
    }

    expr = ast_create_leaf_node(AST_VAR, var_symbol_index);
    expr->expr_type = symtab_get_symbol(var_symbol_index)->data_type;
    return expr;
}

static ASTNode_t *expr_val_expr(Scanner_t *scanner)
{
    Token_t token;
    ASTNode_t *expr;
    scanner_match(scanner, TOK_LPAREN);
    expr = expr_expression(scanner);
    scanner_match(scanner, TOK_RPAREN);
    return expr;
}

static ASTNode_t *expr_multiplicative_expression(Scanner_t *scanner)
{
    Token_t token;
    ASTNode_t *left, *right, *temp;
    ASTNode_type_e type;
    Datatype_t *expr_type;

    left = expr_val(scanner);

    while (scanner_scan(scanner, &token))
    {
        if (token.type == TOK_STAR || token.type == TOK_SLASH)
        {
            type = get_node_type(token.type);
            right = expr_val(scanner);
            expr_type = datatype_expr_type(left->expr_type, right->expr_type);
            left = ast_create_node(type, left, right, 0);
            left->expr_type = expr_type;
        }
        else
        {
            scanner_putback(scanner, &token);
            return left;
        }
    }

    return left;
}

static ASTNode_t *expr_additive_expression(Scanner_t *scanner)
{
    Token_t token;
    ASTNode_t *left, *right;
    ASTNode_type_e type;
    Datatype_t *expr_type;

    left = expr_multiplicative_expression(scanner);

    while (scanner_scan(scanner, &token))
    {
        if (token.type == TOK_PLUS || token.type == TOK_MINUS)
        {
            type = get_node_type(token.type);
            right = expr_multiplicative_expression(scanner);
            expr_type = datatype_expr_type(left->expr_type, right->expr_type);
            left = ast_create_node(type, left, right, 0);
            left->expr_type = expr_type;
        }
        else
        {
            scanner_putback(scanner, &token);
            return left;
        }
    }

    return left;
}

static ASTNode_t *expr_comparison_expression(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *left;
    ASTNode_t *right;
    ASTNode_t *out;
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
        out = ast_create_node(
            get_node_type(tok.type),
            left,
            right,
            0);
        out->expr_type = datatype_get_primative_type(DT_CHAR);
        return out;
    }
    else
    {
        scanner_putback(scanner, &tok);
        return left;
    }
}

static ASTNode_t *expr_func_call(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *func_args = NULL;
    ASTNode_t *func_call;
    int symbol_index;

    scanner_scan(scanner, &tok);
    symbol_index = symtab_find_global_symbol(tok.value.str_value);
    if (symbol_index == -1)
    {
        debug_print(
            SEV_ERROR,
            "[EXPR] Calling function %s before definition",
            tok.value.str_value);
        exit(1);
    }

    Symbol_t *func_symbol = symtab_get_symbol(symbol_index);
    if (func_symbol->sym_type != SYMBOL_FUNC)
    {
        debug_print(
            SEV_ERROR,
            "[EXPR] %s is defined as a variable not a function",
            func_symbol->sym_name);
        exit(1);
    }

    scanner_match(scanner, TOK_LPAREN);
    func_args = args(scanner);
    scanner_match(scanner, TOK_RPAREN);

    if (args_count(func_args) != ((SymbolFunc_t *)func_symbol)->args.size)
    {
        debug_print(
            SEV_ERROR,
            "[EXPR] Expected number of args for %s is %d, found %d",
            func_symbol->sym_name,
            ((SymbolFunc_t *)func_symbol)->args.size,
            args_count(func_args));
        exit(1);
    }

    int counter = 0;
    for (ASTNode_t *actual_arg = func_args; actual_arg; actual_arg = actual_arg->next)
    {
        SymbolFuncArg_t *formal_arg = LList_SymbolFuncArg_get(&((SymbolFunc_t *)func_symbol)->args, counter);
        datatype_check_assign_expr_type(formal_arg->arg_type, actual_arg->expr_type);
        counter++;
    }

    func_call = ast_create_node(
        AST_FUNC_CALL,
        func_args,
        NULL,
        symbol_index);
    func_call->expr_type = symtab_get_symbol(symbol_index)->data_type;
    return func_call;
}

ASTNode_t *expr_assignment(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *var;
    ASTNode_t *val;
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
    if (symtab_get_symbol(var_symbol_index)->sym_type != SYMBOL_VAR)
    {
        debug_print(SEV_ERROR, "%s is defined as a function!!", tok.value.str_value);
        exit(1);
    }
    var = ast_create_leaf_node(
        AST_VAR,
        var_symbol_index);
    var->expr_type = symtab_get_symbol(var_symbol_index)->data_type;

    scanner_match(scanner, TOK_ASSIGN);
    val = expr_expression(scanner);

    expr = ast_create_node(
        AST_ASSIGN,
        var,
        val,
        0);
    expr->expr_type = var->expr_type;

    datatype_check_assign_expr_type(symtab_get_symbol(var_symbol_index)->data_type, expr->expr_type);
    return expr;
}

ASTNode_t *expr_expression(Scanner_t *scanner)
{
    TokenType_e type;
    ASTNode_t *expr;

    // we need to check for ID ASSIGN expr
    // ID token is already cached in stmt_statement,
    // we need to cache only one token to know its' type.
    type = scanner_cache_tok(scanner);

    if (type == TOK_ASSIGN)
    {
        expr = expr_assignment(scanner);
    }
    else if (type == TOK_LPAREN)
    {
        expr = expr_func_call(scanner);
    }
    else
    {
        expr = expr_comparison_expression(scanner);
    }

    assert(expr->expr_type != NULL);
    return expr;
}
