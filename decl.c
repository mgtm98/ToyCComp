#include "decl.h"
#include "debug.h"
#include "symtab.h"
#include "stmt.h"
#include "expr.h"
#include "datatype.h"

#include <stdlib.h>
#include <stdbool.h>

static void decl_id(Scanner_t *scanner, Token_t *tok);
static ASTNode_t *decl_function(Scanner_t *scanner);

static void decl_id(Scanner_t *scanner, Token_t *tok)
{
    scanner_scan(scanner, tok);
    if (tok->type != TOK_ID)
    {
        debug_print(
            SEV_ERROR,
            "[DECL] Expected token TOK_ID, found %s", TokToString(*tok));
        exit(1);
    }
}

ASTNode_t *decl_declarations(Scanner_t *scanner)
{
    ASTNode_t *root = NULL;
    ASTNode_t *head = NULL;
    ASTNode_t *current = NULL;
    TokenType_e type;

    bool need_to_flatten = false;

    while (1)
    {
        scanner_cache_tok(scanner);
        scanner_cache_tok(scanner);
        type = scanner_cache_tok(scanner);

        if (type == TOK_EOF)
            break;

        switch (type)
        {
        case TOK_LPAREN:
            current = decl_function(scanner);
            break;
        default:
            current = decl_var(scanner);
            need_to_flatten = true;
            break;
        }

        if (root == NULL)
            root = current;
        if (head != NULL)
            head->next = current;
        head = current;

        // Move the list tail to the last item in the var decl in case of any
        // This will handle multiple var declarations in the same line
        // int a, b, c;
        if (need_to_flatten)
        {
            head = ast_flatten(head);
            need_to_flatten = false;
        }
    }
    return root;
}

static ASTNode_t *decl_function(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *stmts;
    ASTNode_t *func;
    Datatype_t *return_type;
    int symbol_index;

    return_type = datatype_get_type(scanner);
    decl_id(scanner, &tok);
    scanner_match(scanner, TOK_LPAREN);
    scanner_match(scanner, TOK_RPAREN);
    stmts = stmt_block(scanner);

    symbol_index = symtab_add_global_symbol(
        tok.value.str_value,
        SYMBOL_FUNC,
        return_type);

    func = ast_create_node(
        AST_FUNC_DECL,
        stmts,
        NULL,
        symbol_index);
    func->expr_type = return_type;
    return func;
}

ASTNode_t *decl_var(Scanner_t *scanner)
{
    Datatype_t *var_type;
    ASTNode_t *var_list_head = NULL;
    ASTNode_t *var_list_tail = NULL;
    ASTNode_t *current_var;

    Token_t tok;
    int symbol_index;

    var_type = datatype_get_type(scanner);

    do
    {
        decl_id(scanner, &tok);

        symbol_index = symtab_add_global_symbol(
            tok.value.str_value,
            SYMBOL_VAR,
            var_type);

        current_var = ast_create_leaf_node(AST_VAR_DECL, symbol_index);
        current_var->expr_type = var_type;

        if (var_list_head == NULL)
            var_list_head = current_var;

        if (var_list_tail != NULL)
            var_list_tail->next = current_var;

        var_list_tail = current_var;

        scanner_scan(scanner, &tok);
        if (tok.type == TOK_ASSIGN)
        {
            current_var->left = expr_expression(scanner);
            datatype_check_assign_expr_type(
                symtab_get_symbol(symbol_index)->data_type,
                current_var->left->expr_type);
            scanner_scan(scanner, &tok);
        }

    } while (tok.type == TOK_COMMA);
    scanner_putback(scanner, &tok);
    scanner_match(scanner, TOK_SEMICOLON);

    return var_list_head;
}