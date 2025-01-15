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
static ASTNode_t *expr_lval(Scanner_t *scanner);
static ASTNode_t *expr_val(Scanner_t *scanner);
static ASTNode_t *expr_val_intlit(Scanner_t *scanner);
static ASTNode_t *expr_val_var(Scanner_t *scanner);
static ASTNode_t *expr_val_var_index(Scanner_t *scanner);
static ASTNode_t *expr_dref_ptr(Scanner_t *scanner);
static ASTNode_t *expr_address_of(Scanner_t *scanner);
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

static ASTNode_t *expr_lval(Scanner_t *scanner)
{
    Token_t token;
    ASTNode_t *expr;
    TokenType_e type;
    scanner_peek(scanner, &token);

    if (token.type == TOK_STAR)
        return expr_dref_ptr(scanner);
    else
    {
        while (scanner->buffer_size < 2)
        {
            type = scanner_cache_tok(scanner);
            if (type == TOK_EOF)
                break;
        }

        scanner_peek_at(scanner, &token, 1);

        if (token.type == TOK_LBRACKET)
        {
            return expr_val_var_index(scanner);
        }
        else
        {
            return expr_val_var(scanner);
        }
    }
    return expr;
}

static ASTNode_t *expr_val(Scanner_t *scanner)
{
    Token_t token;
    scanner_peek(scanner, &token);
    switch (token.type)
    {
    case TOK_INTLIT:
        return expr_val_intlit(scanner);
    // case TOK_ID:
    //     return expr_val_var(scanner);
    case TOK_LPAREN:
        return expr_val_expr(scanner);
    case TOK_AMPER:
        return expr_address_of(scanner);
    default:
        return expr_lval(scanner);
        // debug_print(SEV_ERROR, "[EXPR] Expected a number or id token, got %s", TokToString(token));
        // exit(1);
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
    scanner_peek(scanner, &token);

    int var_symbol_index = symtab_find_global_symbol(token.value.str_value);
    if (var_symbol_index < 0)
    {
        debug_print(SEV_ERROR, "[EXPR] %s is not defined before", token.value.str_value);
        exit(1);
    }

    if (symtab_get_symbol(var_symbol_index)->sym_type == SYMBOL_FUNC)
    {
        expr = expr_func_call(scanner);
    }
    else
    {
        scanner_match(scanner, TOK_ID);
        expr = ast_create_leaf_node(AST_VAR, var_symbol_index);
    }

    expr->expr_type = symtab_get_symbol(var_symbol_index)->data_type;
    return expr;
}

static ASTNode_t *expr_val_var_index(Scanner_t *scanner)
{
    Token_t token;
    ASTNode_t *expr, *var, *index;
    Datatype_t *dt;

    var = expr_val_var(scanner);
    scanner_match(scanner, TOK_LBRACKET);
    index = expr_comparison_expression(scanner);
    scanner_match(scanner, TOK_RBRACKET);

    dt = var->expr_type;
    var = ast_create_node(AST_ADDRESSOF, var, NULL, 0);
    var->expr_type = dt;

    expr = ast_create_node(
        AST_ADD,
        var,
        ast_create_node(
            AST_MULT,
            index,
            ast_create_leaf_node(AST_INT_LIT, dt->size / 8),
            0),
        0);
    expr->expr_type = datatype_expr_type(var->expr_type, index->expr_type);
    dt = datatype_deref_pointer(expr->expr_type, 1);
    expr = ast_create_node(
        AST_PTRDREF,
        expr,
        NULL,
        0);
    expr->expr_type = dt;
    return expr;
}

static ASTNode_t *expr_dref_ptr(Scanner_t *scanner)
{
    Token_t token;
    ASTNode_t *expr, *var;
    Datatype_t *type;
    __uint8_t dref_level = 0;

    scanner_peek(scanner, &token);
    while (token.type == TOK_STAR)
    {
        dref_level++;
        scanner_scan(scanner, &token);
        scanner_peek(scanner, &token);
    }
    var = expr_val(scanner);
    expr = var;
    for (int i = 0; i < dref_level; i++)
    {
        type = datatype_deref_pointer(expr->expr_type, 1);
        expr = ast_create_node(AST_PTRDREF, expr, NULL, 0);
        expr->expr_type = type;
    }
    return expr;
}

static ASTNode_t *expr_address_of(Scanner_t *scanner)
{
    ASTNode_t *out;
    ASTNode_t *var;

    scanner_match(scanner, TOK_AMPER);
    var = expr_val_var(scanner);
    out = ast_create_node(AST_ADDRESSOF, var, NULL, 0);
    out->expr_type = datatype_get_pointer_of(var->expr_type);
    return out;
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

    while (1)
    {
        scanner_peek(scanner, &token);
        if (token.type == TOK_STAR || token.type == TOK_SLASH)
        {
            scanner_scan(scanner, &token);
            type = get_node_type(token.type);
            right = expr_val(scanner);
            if (left->expr_type->pointer_level > 0 || right->expr_type->pointer_level > 0)
            {
                debug_print(SEV_ERROR, "[EXPR] Can't create a mult expr with pointers");
                exit(1);
            }
            expr_type = datatype_expr_type(left->expr_type, right->expr_type);
            left = ast_create_node(type, left, right, 0);
            left->expr_type = expr_type;
        }
        else
        {
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

    while (1)
    {
        scanner_peek(scanner, &token);
        if (token.type == TOK_PLUS || token.type == TOK_MINUS)
        {
            scanner_scan(scanner, &token);
            type = get_node_type(token.type);
            right = expr_multiplicative_expression(scanner);
            if (left->expr_type->pointer_level > 0 || right->expr_type->pointer_level > 0)
            {
                ASTNode_t *ptrdref, **other_expr;
                Datatype_t *expr_type;
                __uint8_t offset;
                if (left->expr_type->pointer_level > 0)
                {
                    ptrdref = left;
                    other_expr = &right;
                }
                else
                {
                    ptrdref = right;
                    other_expr = &left;
                }
                if (ptrdref->expr_type->pointer_level > 1)
                    offset = 8;
                else
                    offset = ptrdref->expr_type->base_type->size / 8;
                expr_type = (*other_expr)->expr_type;

                (*other_expr) = ast_create_node(
                    AST_OFFSET_SCALE,
                    *other_expr,
                    NULL,
                    offset);
                (*other_expr)->expr_type = expr_type;
            }
            expr_type = datatype_expr_type(left->expr_type, right->expr_type);
            left = ast_create_node(type, left, right, 0);
            left->expr_type = expr_type;
        }
        else
        {
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
    scanner_peek(scanner, &tok);
    if (
        tok.type == TOK_EQ ||
        tok.type == TOK_NE ||
        tok.type == TOK_GT ||
        tok.type == TOK_GE ||
        tok.type == TOK_LT ||
        tok.type == TOK_LE)
    {
        scanner_scan(scanner, &tok);
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
        return left;
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

    var = expr_lval(scanner);
    scanner_match(scanner, TOK_ASSIGN);
    val = expr_expression(scanner);

    expr = ast_create_node(
        AST_ASSIGN,
        var,
        val,
        0);
    datatype_check_assign_expr_type(var->expr_type, val->expr_type);
    expr->expr_type = var->expr_type;
    return expr;
}

ASTNode_t *expr_expression(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *expr;
    int i = 0;

    do
    {
        scanner_peek_at(scanner, &tok, i);
        i++;
    } while (tok.type != TOK_ASSIGN &&
             tok.type != TOK_SEMICOLON &&
             tok.type != TOK_EMPTY &&
             tok.type != TOK_RPAREN &&
             tok.type != TOK_EOF);

    if (tok.type == TOK_ASSIGN)
    {
        expr = expr_assignment(scanner);
    }
    else
    {
        expr = expr_comparison_expression(scanner);
    }

    assert(expr->expr_type != NULL);
    return expr;
}
