/**
 * @file scanner.c
 * @brief Implementation of the lexical analysis (scanner) phase for the compiler.
 *
 * This file contains functions for scanning the input source code and generating
 * tokens. It processes the source file character by character, identifying keywords,
 * literals, operators, and other syntactic elements based on the language's grammar.
 *
 * Key functionalities:
 * - Matching tokens with specific types (`scanner_match`).
 * - Peeking at the next token without consuming it (`scanner_peek`).
 * - Putting back a token into the stream for reprocessing (`scanner_putback`).
 * - Tokenizing operators, keywords, literals, and identifiers.
 *
 * @author Mohamed Gamal
 * @project ToyCComp
 * @date 2024-12-24
 *
 */

#include "scanner.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <string.h>

/**
 * @brief Checks the position of a character in a string.
 *
 * @param s Pointer to the string.
 * @param c The character to locate.
 * @return The position of the character in the string, or -1 if not found.
 */
static int chrpos(char *s, int c)
{
    char *p;
    p = strchr(s, c);
    return (p ? p - s : -1);
}

/**
 * @brief Checks if a file exists.
 *
 * @param filename Pointer to the file path.
 * @return `true` if the file exists, `false` otherwise.
 */
bool file_exists(const char *filename)
{
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

/**
 * @brief Skips whitespace characters in the source code.
 *
 * This function advances the scanner's position, skipping over whitespace and
 * updating the line and column numbers as necessary.
 *
 * @param scanner Pointer to the scanner context.
 */
static void skip_ws(Scanner_t *scanner)
{
    char out;
    do
    {
        if (scanner->putback_char != 0)
        {
            out = scanner->putback_char;
            scanner->putback_char = 0;
        }
        else
        {
            out = fgetc(scanner->file);
            scanner->current_col_number++;
        }
        if (out == '\n')
        {
            scanner->current_line_number++;
            scanner->current_col_number = 1;
        }
    } while (isspace(out));
    scanner->putback_char = out;
}

/**
 * @brief Reads the next character from the source code.
 *
 * @param scanner Pointer to the scanner context.
 * @return The next character from the input stream.
 */
static char next(Scanner_t *scanner)
{
    char out;

    if (scanner->putback_char)
    {
        out = scanner->putback_char;
        scanner->putback_char = 0;
        return out;
    }
    out = fgetc(scanner->file);
    scanner->current_col_number++;

    return out;
}

/**
 * @brief Scans an integer literal from the source code.
 *
 * @param scanner Pointer to the scanner context.
 * @return The integer value parsed from the input stream.
 */
static int scan_number(Scanner_t *scanner)
{
    int t;
    char c;
    int out = 0;

    while (true)
    {
        c = next(scanner);
        t = chrpos("0123456789", c);
        if (t < 0)
        {
            scanner->putback_char = c;
            break;
        }
        else
        {
            out = out * 10 + t;
        }
    }

    return out;
}

/**
 * @brief Scans an identifier or keyword from the source code.
 *
 * @param scanner Pointer to the scanner context.
 * @return Pointer to the dynamically allocated string representing the identifier.
 */
static char *scan_id(Scanner_t *scanner)
{
    char c;
    int i = 0;
    char buffer[255];

    while (true)
    {
        c = next(scanner);
        if (isalpha(c) || c == '_')
        {
            buffer[i] = c;
            i++;
        }
        else
        {
            scanner->putback_char = c;
            buffer[i] = '\0';
            break;
        }
    }
    return strdup(buffer);
}

/**
 * @brief Checks if an identifier matches a reserved keyword.
 *
 * @param id Pointer to the identifier string.
 * @return The token type corresponding to the keyword or `TOK_ID` if not a keyword.
 */
static TokenType_e check_keyword(char *id)
{
    static const struct
    {
        const char *keyword;
        TokenType_e token;
    } keyword_map[] = {
        {"break", TOK_BREAK},
        {"else", TOK_ELSE},
        {"if", TOK_IF},
        {"int", TOK_INT},
        {"while", TOK_WHILE},
        {"do", TOK_DO},
        {"for", TOK_FOR}};

    size_t id_length = strlen(id);

    for (size_t i = 0; i < sizeof(keyword_map) / sizeof(keyword_map[0]); i++)
    {
        if (strncmp(id, keyword_map[i].keyword, id_length) == 0 &&
            id_length == strlen(keyword_map[i].keyword))
        {
            return keyword_map[i].token;
        }
    }

    return TOK_ID;
}

/**
 * @brief Initializes the scanner.
 *
 * This function allocates and initializes a `Scanner_t` object, opening the
 * specified file for reading.
 *
 * @param file_path Pointer to the string representing the file path.
 * @return Pointer to the initialized scanner context, or `NULL` if the file does not exist.
 */
Scanner_t *scanner_init(char *file_path)
{
    Scanner_t *scanner = (Scanner_t *)calloc(1, sizeof(Scanner_t));
    scanner->current_line_number = 1;
    scanner->current_col_number = 1;
    if (file_exists(file_path))
    {
        scanner->file = fopen(file_path, "r");
        return scanner;
    }
    else
    {
        debug_print(SEV_ERROR, "File %s is not found", file_path);
        return NULL;
    }
}

/**
 * @brief Scans the next token from the source code.
 *
 * This function identifies and classifies the next token in the source stream,
 * handling keywords, literals, operators, and other syntactic elements.
 *
 * @param scanner Pointer to the scanner context.
 * @param tok Pointer to the token object to store the scanned token.
 * @return `true` if a token is successfully scanned, `false` if end of file (EOF) is reached.
 */
bool scanner_scan(Scanner_t *scanner, Token_t *tok)
{
    if (scanner->putback_tok.type != TOK_EMPTY)
    {
        tok->type = scanner->putback_tok.type;
        tok->value = scanner->putback_tok.value;
        scanner->putback_tok.type = TOK_EMPTY;
        debug_print(SEV_DEBUG, "Token %s", TokToString(*tok));
        return true;
    }

    skip_ws(scanner);
    char t = next(scanner);
    switch (t)
    {
    case EOF:
        tok->type = TOK_EOF;
        return false;
    case '+':
        tok->type = TOK_PLUS;
        break;
    case '-':
        tok->type = TOK_MINUS;
        break;
    case '*':
        tok->type = TOK_STAR;
        break;
    case '/':
        tok->type = TOK_SLASH;
        break;
    case ';':
        tok->type = TOK_SEMICOLON;
        break;
    case '(':
        tok->type = TOK_LPAREN;
        break;
    case ')':
        tok->type = TOK_RPAREN;
        break;
    case '{':
        tok->type = TOK_LBRACE;
        break;
    case '}':
        tok->type = TOK_RBRACE;
        break;
    case '>':
        t = next(scanner);
        if (t == '=')
            tok->type = TOK_GE;
        else
        {
            scanner->putback_char = t;
            tok->type = TOK_GT;
        }
        break;
    case '<':
        t = next(scanner);
        if (t == '=')
            tok->type = TOK_LE;
        else
        {
            scanner->putback_char = t;
            tok->type = TOK_LT;
        }
        break;
    case '=':
        t = next(scanner);
        if (t == '=')
            tok->type = TOK_EQ;
        else
        {
            scanner->putback_char = t;
            tok->type = TOK_ASSIGN;
        }
        break;
    case '!':
        t = next(scanner);
        if (t == '=')
            tok->type = TOK_NE;
        else
        {
            debug_print(SEV_ERROR, "[SCANNER] expected '=' but found '%c'", t);
            exit(1);
        }
        break;
    default:
        if (isdigit(t))
        {
            scanner->putback_char = t;
            tok->type = TOK_INTLIT;
            tok->value.int_value = scan_number(scanner);
        }
        else if (isalpha(t) || t == '_')
        {
            scanner->putback_char = t;
            tok->value.str_value = scan_id(scanner);
            tok->type = check_keyword(tok->value.str_value);
        }
        else
        {
            debug_print(SEV_ERROR, "Unkown token %c \n", t);
        }
        break;
    }
    debug_print(SEV_DEBUG, "Token %s", TokToString(*tok));
    return true;
}

/**
 * @brief Matches the next token against a specific type.
 *
 * This function scans the next token and checks if it matches the expected type.
 * If the types match, the function returns `true`. Otherwise, an error is logged
 * and the program terminates.
 *
 * @param scanner Pointer to the scanner context.
 * @param what The expected token type to match.
 * @return `true` if the token matches the expected type; otherwise, the program exits.
 */
bool scanner_match(Scanner_t *scanner, TokenType_e what)
{
    Token_t t;
    scanner_scan(scanner, &t);
    debug_print(
        SEV_DEBUG,
        "line %d: Matching type: %s, found type: %s",
        scanner->current_line_number,
        TokTypeToString(what),
        TokToString(t));
    if (t.type == what)
        return true;
    debug_print(
        SEV_ERROR,
        "line %d: Expected token: %s, found token: %s",
        scanner->current_line_number,
        TokTypeToString(what),
        TokToString(t));
    exit(1);
    return false;
}

/**
 * @brief Puts a token back into the stream for reprocessing.
 *
 * This function stores the specified token in the scanner's internal buffer,
 * making it the next token to be processed.
 *
 * @param scanner Pointer to the scanner context.
 * @param tok Pointer to the token to be put back.
 */
void scanner_putback(Scanner_t *scanner, Token_t *tok)
{
    scanner->putback_tok.type = tok->type;
    scanner->putback_tok.value = tok->value;
    debug_print(SEV_DEBUG, "putback Token %s", TokToString(*tok));
}

/**
 * @brief Peeks at the next token without consuming it.
 *
 * This function scans the next token and immediately puts it back into the
 * stream, allowing non-destructive inspection of the token.
 *
 * @param scanner Pointer to the scanner context.
 * @param tok Pointer to the token object where the peeked token will be stored.
 */
void scanner_peek(Scanner_t *scanner, Token_t *tok)
{
    scanner_scan(scanner, tok);
    scanner_putback(scanner, tok);
}
