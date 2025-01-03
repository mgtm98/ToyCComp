#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "stdio.h"
#include <stdbool.h>

#define MAX_PUTBACK_BUFFER_SIZE 255
typedef enum
{
    TOK_EMPTY, /** Empty token, default value. */
    TOK_EOF,   /** End of file. */

    TOK_PLUS,  /** '+' operator. */
    TOK_MINUS, /** '-' operator. */
    TOK_STAR,  /** '*' operator. */
    TOK_SLASH, /** '/' operator. */

    TOK_GT, /** '>' operator. */
    TOK_GE, /** '>=' operator. */
    TOK_LT, /** '<' operator. */
    TOK_LE, /** '<=' operator. */
    TOK_EQ, /** '==' operator. */
    TOK_NE, /** '!=' operator. */

    TOK_INTLIT, /** Integer literal. */
    TOK_ID,     /** Identifier. */

    TOK_INT,  /** 'int' keyword. */
    TOK_CHAR, /** 'int' keyword. */
    TOK_VOID, /** 'void' keyword. */
    TOK_LONG, /** 'void' keyword. */

    TOK_IF,    /** 'if' keyword. */
    TOK_ELSE,  /** 'else' keyword. */
    TOK_WHILE, /** 'while' keyword. */
    TOK_BREAK, /** 'break' keyword. */
    TOK_DO,    /** 'do' keyword. */
    TOK_FOR,   /** 'for  kryword. */

    TOK_SEMICOLON, /** ';' token. */
    TOK_COMMA,     /** ',' token. */
    TOK_LPAREN,    /** '(' token. */
    TOK_RPAREN,    /** ')' token. */
    TOK_LBRACE,    /** '{' token. */
    TOK_RBRACE,    /** '}' token. */
    TOK_ASSIGN     /** '=' assignment operator. */
} TokenType_e;

typedef struct
{
    TokenType_e type; /** Type of the token. */
    union
    {
        int int_value;   /** Integer value for numeric literals. */
        char *str_value; /** String value for identifiers or keywords. */
    } value;
    __uint32_t row; /** Line number where the token was found. */
    __uint32_t col; /** Column number where the token was found. */
} Token_t;

static char *__token_names[] = {
    "TOK_EMPTY",
    "TOK_EOF",
    "TOK_PLUS",
    "TOK_MINUS",
    "TOK_STAR",
    "TOK_SLASH",
    "TOK_GT",
    "TOK_GE",
    "TOK_LT",
    "TOK_LE",
    "TOK_EQ",
    "TOK_NE",
    "TOK_INTLIT",
    "TOK_ID",
    "TOK_INT",
    "TOK_CHAR",
    "TOK_VOID",
    "TOK_LONG",
    "TOK_IF",
    "TOK_ELSE",
    "TOK_WHILE",
    "TOK_BREAK",
    "TOK_DO",
    "TOK_FOR",
    "TOK_SEMICOLON",
    "TOK_LPAREN",
    "TOK_RPAREN",
    "TOK_LBRACE",
    "TOK_RBRACE",
    "TOK_ASSIGN"};

#define TokToString(tok) __token_names[(tok).type]          /** Convert token to string. */
#define TokTypeToString(tok_type) __token_names[(tok_type)] /** Convert token type to string. */

typedef __uint64_t CodeLocation;
#define PackLocation(row, col) ((CodeLocation)row << 32) | (__uint32_t)col
#define UnpackRow(loc) (int)(loc >> 32)
#define UnpackCol(loc) (int)(loc & 0xFFFFFFFF)

typedef struct
{
    FILE *file;
    Token_t putback_tok_buffer[MAX_PUTBACK_BUFFER_SIZE];
    int buffer_head;
    int buffer_tail;
    int buffer_size;
    char putback_char;
    __uint32_t current_line_number;
    __uint32_t current_col_number;
} Scanner_t;

Scanner_t *scanner_init(char *file_path);

void scanner_peek(Scanner_t *scanner, Token_t *tok);
bool scanner_scan(Scanner_t *scanner, Token_t *tok);
bool scanner_match(Scanner_t *scanner, TokenType_e what);
void scanner_putback(Scanner_t *scanner, Token_t *tok);
void scanner_copy_tok(Token_t *dest, Token_t *src);
TokenType_e scanner_cache_tok(Scanner_t *scanner);

#endif
