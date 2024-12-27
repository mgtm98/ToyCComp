#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "stdio.h"
#include <stdbool.h>

/**
 * @file scanner.h
 * @brief Header file for the lexical analysis (scanner) phase of the compiler.
 *
 * This file provides the interface for tokenizing source code into meaningful
 * units (tokens) for parsing. It defines token types, scanner context, and
 * utility functions to support lexical analysis.
 *
 * @project ToyCComp
 * @author Mohamed Gamal
 * @date 2024-12-24
 *
 */

/** @enum TokenType_e
 *  @brief Enum for token types.
 */
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
    TOK_INT,    /** 'int' keyword. */

    TOK_IF,    /** 'if' keyword. */
    TOK_ELSE,  /** 'else' keyword. */
    TOK_WHILE, /** 'while' keyword. */
    TOK_BREAK, /** 'break' keyword. */
    TOK_DO,    /** 'do' keyword. */
    TOK_FOR,   /** 'for  kryword. */

    TOK_SEMICOLON, /** ';' token. */
    TOK_LPAREN,    /** '(' token. */
    TOK_RPAREN,    /** ')' token. */
    TOK_LBRACE,    /** '{' token. */
    TOK_RBRACE,    /** '}' token. */
    TOK_ASSIGN     /** '=' assignment operator. */
} TokenType_e;

/**
 * @brief Structure representing a token.
 */
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

/**
 * @brief Array mapping token types to their string representations.
 */
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

/**
 * @brief Encoded representation of a code location (row and column).
 */
typedef __uint64_t CodeLocation;

/**
 * @brief Packs row and column into a single 64-bit value.
 * @param row Line number.
 * @param col Column number.
 * @return Encoded location.
 */
#define PackLocation(row, col) ((CodeLocation)row << 32) | (__uint32_t)col

/**
 * @brief Extracts the row (line number) from an encoded location.
 * @param loc Encoded location.
 * @return Line number.
 */
#define UnpackRow(loc) (int)(loc >> 32)

/**
 * @brief Extracts the column number from an encoded location.
 * @param loc Encoded location.
 * @return Column number.
 */
#define UnpackCol(loc) (int)(loc & 0xFFFFFFFF)

/**
 * @brief Structure representing the scanner context.
 */
typedef struct
{
    FILE *file;                     /** File pointer to the source code file. */
    Token_t putback_tok;            /** Token buffer for putback operations. */
    char putback_char;              /** Character buffer for putback operations. */
    __uint32_t current_line_number; /** Current line number in the source. */
    __uint32_t current_col_number;  /** Current column number in the source. */
} Scanner_t;

/**
 * @brief Initializes the scanner.
 * @param file_path Path to the source file.
 * @return Pointer to the initialized scanner context.
 */
Scanner_t *scanner_init(char *file_path);

/**
 * @brief Scans the next token from the source code.
 * @param scanner Pointer to the scanner context.
 * @param tok Pointer to the token to store the result.
 * @return `true` if a token is scanned, `false` if EOF is reached.
 */
bool scanner_scan(Scanner_t *scanner, Token_t *tok);

/**
 * @brief Peeks at the next token without consuming it.
 * @param scanner Pointer to the scanner context.
 * @param tok Pointer to the token to store the peeked result.
 */
void scanner_peek(Scanner_t *scanner, Token_t *tok);

/**
 * @brief Matches the next token against a specified type.
 * @param scanner Pointer to the scanner context.
 * @param what The expected token type.
 * @return `true` if the token matches, otherwise the program exits with an error.
 */
bool scanner_match(Scanner_t *scanner, TokenType_e what);

/**
 * @brief Puts a token back into the stream for reprocessing.
 * @param scanner Pointer to the scanner context.
 * @param tok Pointer to the token to be put back.
 */
void scanner_putback(Scanner_t *scanner, Token_t *tok);

#endif
