#include "decl.h"
#include "debug.h"
#include "symtab.h"
#include "stmt.h"

#include <stdlib.h>
#include <stdbool.h>

#define TOKEN_BUFFER_MAX_SIZE 256

static Token_t tok_buffer[TOKEN_BUFFER_MAX_SIZE];
static __uint8_t token_buffer_top = 0;
static __uint8_t token_buffer_last = 1;
static __uint8_t token_buffer_size = 0;

static void match_and_cache_token(Scanner_t *scanner, TokenType_e what);
static bool scan_token(Scanner_t *scanner, Token_t *tok);

static ASTNode_t *decl_function(Scanner_t *scanner);

// ============================================================
//                  Token Buffer Operations
// ============================================================

static void match_and_cache_token(Scanner_t *scanner, TokenType_e what)
{
    Token_t tok;
    if (token_buffer_size == TOKEN_BUFFER_MAX_SIZE)
    {
        debug_print(SEV_ERROR, "[DECL] Token buffer is full!!");
        exit(1);
    }
    scanner_scan(scanner, &tok);
    scanner_copy_tok(&tok_buffer[token_buffer_last - 1], &tok);
    token_buffer_last = (token_buffer_last + 1) % TOKEN_BUFFER_MAX_SIZE;
    token_buffer_size++;
}

static bool match_token(Scanner_t *scanner, TokenType_e what)
{
    if (token_buffer_size == 0)
    {
        return scanner_match(scanner, what);
    }
    if (tok_buffer[token_buffer_top].type != what)
    {
        debug_print(
            SEV_ERROR,
            "[DECL] Expected token %s, found token %s",
            TokTypeToString(what),
            TokToString(tok_buffer[token_buffer_top]));
        exit(1);
    }
    token_buffer_top = (token_buffer_top - 1) % TOKEN_BUFFER_MAX_SIZE;
    token_buffer_size--;
    return true;
}

static bool scan_token(Scanner_t *scanner, Token_t *tok)
{
    if (token_buffer_size == 0)
    {
        return scanner_scan(scanner, tok);
    }
    scanner_copy_tok(tok, &tok_buffer[token_buffer_top]);
    token_buffer_top = (token_buffer_top + 1) % TOKEN_BUFFER_MAX_SIZE;
    token_buffer_size--;
}

// ============================================================
//                  Declarations Parsing
// ============================================================

ASTNode_t *decl_declarations(Scanner_t *scanner)
{
    match_and_cache_token(scanner, TOK_VOID);
    match_and_cache_token(scanner, TOK_ID);
    match_and_cache_token(scanner, TOK_LBRACE);
    return decl_function(scanner);
}

static ASTNode_t *decl_function(Scanner_t *scanner)
{
    Token_t tok;
    ASTNode_t *stmts;
    int symbol_index;

    scan_token(scanner, &tok); // function type
    scan_token(scanner, &tok); // function name
    symbol_index = symtab_add_global_symbol(tok.value.str_value);
    match_token(scanner, TOK_LPAREN);
    match_token(scanner, TOK_RPAREN);
    stmts = stmt_block(scanner);
    return ast_create_node(
        AST_FUNC_DECL,
        stmts,
        NULL,
        symbol_index);
}