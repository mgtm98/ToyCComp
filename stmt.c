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

static ASTNode_t *stmt_print(Scanner_t *scanner)
{
    scanner_match(scanner, TOK_ID);
    scanner_match(scanner, TOK_LPAREN);
    ASTNode_t *expr = expr_expression(scanner);
    scanner_match(scanner, TOK_RPAREN);
    scanner_match(scanner, TOK_SEMICOLON);
    return ast_create_node(AST_PRINT, expr, NULL, 0);
}

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

static ASTNode_t *stmt_break(Scanner_t *scanner)
{
    scanner_match(scanner, TOK_BREAK);
    scanner_match(scanner, TOK_SEMICOLON);
    return ast_create_leaf_node(AST_BREAK, 0);
}

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