#include "stmt.h"
#include "debug.h"
#include "string.h"
#include "stdlib.h"
#include "symtab.h"
#include "expr.h"

static bool in_loop = false;
extern int decl_current_func;

static ASTNode_t *stmt_statements(Scanner_t *scanner);
static ASTNode_t *stmt_statement(Scanner_t *scanner);
static ASTNode_t *stmt_print(Scanner_t *scanner);
static ASTNode_t *stmt_var_decl(Scanner_t *scanner);
static ASTNode_t *stmt_if(Scanner_t *scanner);
static ASTNode_t *stmt_while(Scanner_t *scanner);
static ASTNode_t *stmt_do_while(Scanner_t *scanner);
static ASTNode_t *stmt_for(Scanner_t *scanner);
static ASTNode_t *stmt_break(Scanner_t *scanner);
static ASTNode_t *stmt_return(Scanner_t *scanner);
static ASTNode_t *stmt_expression(Scanner_t *scanner);

static ASTNode_t *stmt_statements(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *root;
    ASTNode_t *head = stmt_statement(scanner);
    ASTNode_t *current;

    root = head;
    head = ast_flatten(head);

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

    // Func calls, assinement statements
    case TOK_STAR:
    case TOK_ID:
        return stmt_expression(scanner);
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
    case TOK_RETURN:
        return stmt_return(scanner);
    case TOK_SEMICOLON:
        scanner_match(scanner, TOK_SEMICOLON);
        return ast_create_leaf_node(
            AST_EMPTY,
            (ASTNodeValue)0);

    // local declarations using primatives
    default:
        return stmt_var_decl(scanner);
    }
}

static ASTNode_t *stmt_print(Scanner_t *scanner)
{
    scanner_match(scanner, TOK_ID);
    scanner_match(scanner, TOK_LPAREN);
    ASTNode_t *expr = expr_expression(scanner);
    scanner_match(scanner, TOK_RPAREN);
    scanner_match(scanner, TOK_SEMICOLON);
    return ast_create_node(AST_PRINT, expr, NULL, (ASTNodeValue)0);
}

static ASTNode_t *stmt_var_decl(Scanner_t *scanner)
{
    return decl_var(scanner);
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

    scanner_peek(scanner, &tok);
    if (tok.type != TOK_ELSE)
    {
        return ast_create_node(
            AST_IF,
            expr,
            ast_create_node(
                AST_GLUE,
                true_code,
                NULL,
                (ASTNodeValue)0),
            (ASTNodeValue)0);
    }
    scanner_match(scanner, TOK_ELSE);
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
            (ASTNodeValue)0),
        (ASTNodeValue)0);
}

static ASTNode_t *stmt_while(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *expr, *code;

    in_loop = true;
    scanner_match(scanner, TOK_WHILE);
    scanner_match(scanner, TOK_LPAREN);
    expr = expr_expression(scanner);
    scanner_match(scanner, TOK_RPAREN);
    code = stmt_block(scanner);
    in_loop = false;

    return ast_create_node(
        AST_WHILE,
        expr,
        code,
        (ASTNodeValue)0);
}

static ASTNode_t *stmt_do_while(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *expr, *code;

    in_loop = true;
    scanner_match(scanner, TOK_DO);
    code = stmt_block(scanner);
    scanner_match(scanner, TOK_WHILE);
    scanner_match(scanner, TOK_LPAREN);
    expr = expr_expression(scanner);
    scanner_match(scanner, TOK_RPAREN);
    scanner_match(scanner, TOK_SEMICOLON);
    in_loop = false;

    return ast_create_node(
        AST_DO_WHILE,
        expr,
        code,
        (ASTNodeValue)0);
}

static ASTNode_t *stmt_for(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *pre_post, *code;

    in_loop = true;
    scanner_match(scanner, TOK_FOR);
    scanner_match(scanner, TOK_LPAREN);
    pre_post = stmt_statement(scanner);
    pre_post->next = expr_expression(scanner);
    scanner_match(scanner, TOK_SEMICOLON);
    scanner_peek(scanner, &tok);
    if (tok.type == TOK_RPAREN)
    {
        scanner_scan(scanner, &tok);
        pre_post->next->next = ast_create_leaf_node(AST_EMPTY, (ASTNodeValue)0);
    }
    else
    {
        pre_post->next->next = expr_assignment(scanner);
        scanner_match(scanner, TOK_RPAREN);
    }
    code = stmt_block(scanner);
    in_loop = false;

    return ast_create_node(
        AST_FOR,
        pre_post,
        code,
        (ASTNodeValue)0);
}

static ASTNode_t *stmt_break(Scanner_t *scanner)
{
    if (!in_loop)
    {
        debug_print(SEV_ERROR, "[STMT] Can't call a break outside a loop context");
        exit(1);
    }
    scanner_match(scanner, TOK_BREAK);
    scanner_match(scanner, TOK_SEMICOLON);
    return ast_create_leaf_node(AST_BREAK, (ASTNodeValue)0);
}

static ASTNode_t *stmt_return(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *return_stmt;
    Symbol_t *func = symtab_get_symbol(decl_current_func);

    scanner_match(scanner, TOK_RETURN);
    scanner_peek(scanner, &tok);

    if (tok.type == TOK_SEMICOLON)
    {

        if (func->data_type != DATATYPE_VOID)
        {
            debug_print(SEV_ERROR, "[STMT] Must return a %s type", func->data_type->name);
            exit(1);
        }
        scanner_match(scanner, TOK_SEMICOLON);
        return_stmt = ast_create_leaf_node(AST_RETURN, (ASTNodeValue)decl_current_func);
        return_stmt->expr_type = DATATYPE_VOID;
        return return_stmt;
    }
    else
    {
        ASTNode_t *expr = expr_expression(scanner);
        scanner_match(scanner, TOK_SEMICOLON);
        datatype_check_assign_expr_type(func->data_type, expr->expr_type);
        return_stmt = ast_create_node(
            AST_RETURN,
            expr,
            NULL,
            (ASTNodeValue)decl_current_func);
        return_stmt->expr_type = expr->expr_type;
        return return_stmt;
    }
}

static ASTNode_t *stmt_expression(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *assignment_expr;

    assignment_expr = expr_expression(scanner);
    scanner_match(scanner, TOK_SEMICOLON);

    return assignment_expr;
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