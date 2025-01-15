#include "decl.h"
#include "debug.h"
#include "symtab.h"
#include "stmt.h"
#include "expr.h"
#include "datatype.h"
#include "llist_definitions.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static void decl_id(Scanner_t *scanner, Token_t *tok);
static ASTNode_t *decl_function(Scanner_t *scanner);

static void *args_decl(Scanner_t *scanner, LList_t *args_list);

int decl_current_func = DECL_NO_FUNC;

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

    if (scanner == NULL)
        return NULL;

    while (1)
    {
        while (1)
        {
            type = scanner_cache_tok(scanner);
            if (
                type == TOK_SEMICOLON ||
                type == TOK_EMPTY ||
                type == TOK_LPAREN ||
                type == TOK_EOF ||
                type == TOK_COMMA ||
                type == TOK_ASSIGN)
                break;
        }

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

// TODO: Make sure there is no duplicate argument name
// TODO: Create symbol table for each function
// TODO: Create a visualization code for SymbolTables
static ASTNode_t *decl_function(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *stmts;
    ASTNode_t *func;
    Datatype_t *return_type;
    int symbol_index;

    return_type = datatype_get_type(scanner);
    decl_id(scanner, &tok);

    symbol_index = symtab_add_global_symbol(
        tok.value.str_value,
        SYMBOL_FUNC,
        return_type);
    decl_current_func = symbol_index;

    scanner_match(scanner, TOK_LPAREN);
    args_decl(scanner, &((SymbolFunc_t *)symtab_get_symbol(symbol_index))->args);
    scanner_match(scanner, TOK_RPAREN);

    stmts = stmt_block(scanner);
    decl_current_func = DECL_NO_FUNC;

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

    while (1)
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

        scanner_peek(scanner, &tok);
        if (tok.type == TOK_ASSIGN)
        {
            scanner_scan(scanner, &tok);
            current_var->left = expr_expression(scanner);
            datatype_check_assign_expr_type(
                symtab_get_symbol(symbol_index)->data_type,
                current_var->left->expr_type);
            scanner_peek(scanner, &tok);
        }
        else if (tok.type == TOK_LBRACKET)
        {
            scanner_scan(scanner, &tok);
            scanner_scan(scanner, &tok);
            if (tok.type == TOK_INTLIT)
            {
                Datatype_t *array_dt = datatype_get_pointer_of(var_type);
                array_dt->array_size = tok.value.int_value;
                current_var->expr_type = array_dt;
                symtab_get_symbol(symbol_index)->data_type = array_dt;
            }
            else
            {
                debug_print(
                    SEV_ERROR,
                    "[DATATYPE] Expected an integer literal, found %s",
                    TokToString(tok));
                exit(1);
            }
            scanner_match(scanner, TOK_RBRACKET);
        }

        if (tok.type == TOK_COMMA)
            scanner_scan(scanner, &tok);
        else
            break;
    }
    scanner_match(scanner, TOK_SEMICOLON);

    return var_list_head;
}

ASTNode_t *args(Scanner_t *scanner)
{
    ASTNode_t *args = NULL;
    ASTNode_t *args_head = NULL;
    Token_t tok;
    scanner_peek(scanner, &tok);
    if (tok.type == TOK_RPAREN)
        return NULL;

    do
    {
        if (args == NULL)
            args = expr_expression(scanner);
        else
        {
            args->next = expr_expression(scanner);
            args = args->next;
        }
        if (args_head == NULL)
            args_head = args;

        scanner_peek(scanner, &tok);
        if (tok.type != TOK_COMMA)
            break;
        scanner_scan(scanner, &tok);
    } while (1);
    return args_head;
}

static void *args_decl(Scanner_t *scanner, LList_t *args_list)
{
    Token_t tok;
    Datatype_t *type;

    while (true)
    {
        scanner_peek(scanner, &tok);
        if (tok.type == TOK_RPAREN)
            break;

        type = datatype_get_type(scanner);
        scanner_scan(scanner, &tok);
        if (tok.type != TOK_ID)
        {
            debug_print(SEV_ERROR, "[DECL] Expected an identifier, found %s", TokToString(tok));
        }
        SymbolFuncArg_t *argument = (SymbolFuncArg_t *)malloc(sizeof(SymbolFuncArg_t));
        argument->arg_name = strdup(tok.value.str_value);
        argument->arg_type = type;
        LList_SymbolFuncArg_append(args_list, argument);

        scanner_peek(scanner, &tok);
        if (tok.type == TOK_COMMA)
            scanner_scan(scanner, &tok);
    }
}

int args_count(ASTNode_t *args)
{
    int i = 0;
    while (args)
    {
        i++;
        args = args->next;
    }
    return i;
}