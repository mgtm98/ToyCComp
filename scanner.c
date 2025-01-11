#include "scanner.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <string.h>

static void scanner_putback(Scanner_t *scanner, Token_t *tok);

static int chrpos(char *s, int c)
{
    char *p;
    p = strchr(s, c);
    return (p ? p - s : -1);
}

static bool file_exists(const char *filename)
{
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

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

static TokenType_e check_keyword(char *id)
{
    static const struct
    {
        const char *keyword;
        TokenType_e token;
    } keyword_map[] = {
        {"break", TOK_BREAK},
        {"char", TOK_CHAR},
        {"do", TOK_DO},
        {"else", TOK_ELSE},
        {"for", TOK_FOR},
        {"if", TOK_IF},
        {"int", TOK_INT},
        {"long", TOK_LONG},
        {"return", TOK_RETURN},
        {"void", TOK_VOID},
        {"while", TOK_WHILE}};

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

Scanner_t *scanner_init(char *file_path)
{
    Scanner_t *scanner = (Scanner_t *)calloc(1, sizeof(Scanner_t));
    scanner->current_line_number = 1;
    scanner->current_col_number = 1;
    scanner->buffer_head = 0;
    scanner->buffer_tail = 1;
    scanner->buffer_size = 0;

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

static bool __scanner_scan(Scanner_t *scanner, Token_t *tok, bool ignore_cache)
{
    if (!ignore_cache && scanner->buffer_size > 0)
    {
        scanner_copy_tok(tok, &scanner->putback_tok_buffer[scanner->buffer_head]);
        scanner->buffer_head = (scanner->buffer_head + 1) % MAX_PUTBACK_BUFFER_SIZE;
        scanner->buffer_size--;
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
    case ',':
        tok->type = TOK_COMMA;
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
    case '&':
        tok->type = TOK_AMPER;
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

bool scanner_scan(Scanner_t *scanner, Token_t *tok)
{
    return __scanner_scan(scanner, tok, false);
}

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

static void scanner_putback(Scanner_t *scanner, Token_t *tok)
{
    scanner_copy_tok(&scanner->putback_tok_buffer[scanner->buffer_tail - 1], tok);
    scanner->buffer_tail = (scanner->buffer_tail + 1) % MAX_PUTBACK_BUFFER_SIZE;
    scanner->buffer_size++;
}

void scanner_peek(Scanner_t *scanner, Token_t *tok)
{
    if (scanner->buffer_size == 0)
    {
        scanner_scan(scanner, tok);
        scanner_putback(scanner, tok);
    }
    else
    {
        scanner_copy_tok(tok, &scanner->putback_tok_buffer[scanner->buffer_head]);
    }
}

void scanner_copy_tok(Token_t *dest, Token_t *src)
{
    dest->type = src->type;
    dest->value = src->value;
}

TokenType_e scanner_cache_tok(Scanner_t *scanner)
{
    Token_t t;
    __scanner_scan(scanner, &t, true);
    scanner_putback(scanner, &t);
    return t.type;
}