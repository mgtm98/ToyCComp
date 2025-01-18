#ifndef _AST_H_
#define _AST_H_

#include "scanner.h"
#include "datatype.h"

typedef enum
{
    AST_GLUE,
    AST_EMPTY,

    AST_ADD,
    AST_SUBTRACT,
    AST_MULT,
    AST_DIV,

    AST_COMP_GT,
    AST_COMP_GE,
    AST_COMP_LT,
    AST_COMP_LE,
    AST_COMP_EQ,
    AST_COMP_NE,

    AST_INT_LIT,
    AST_STR_LIT,

    AST_PRINT,
    AST_ASSIGN,
    AST_VAR,
    AST_DATATYPE,

    AST_ADDRESSOF,
    AST_PTRDREF,
    AST_OFFSET_SCALE,
    AST_ARRAY_INDEX,

    AST_VAR_DECL,
    AST_FUNC_DECL,

    AST_FUNC_CALL,
    AST_RETURN,

    AST_IF,

    AST_WHILE,
    AST_DO_WHILE,
    AST_FOR,
    AST_BREAK
} ASTNode_type_e;

#define ASTCheckLoopContext(t) t >= AST_WHILE &&t <= AST_FOR

typedef union ASTNodeValue
{
    int num;
    char *str;
} ASTNodeValue;

typedef struct ASTNode_t ASTNode_t;
struct ASTNode_t
{
    ASTNode_type_e type;
    ASTNode_t *next;
    ASTNode_t *left;
    ASTNode_t *right;
    ASTNode_t *parent;
    Datatype_t *expr_type;
    ASTNodeValue value;
};

static char *__ast_type_names[] = {
    "AST_GLUE",
    "AST_EMPTY",
    "AST_ADD",
    "AST_SUBTRACT",
    "AST_MULT",
    "AST_DIV",
    "AST_COMP_GT",
    "AST_COMP_GE",
    "AST_COMP_LT",
    "AST_COMP_LE",
    "AST_COMP_EQ",
    "AST_COMP_NE",
    "AST_INT_LIT",
    "AST_STR_LIT",
    "AST_PRINT",
    "AST_ASSIGN",
    "AST_VAR",
    "AST_DATATYPE",
    "AST_ADDRESSOF",
    "AST_PTRDREF",
    "AST_OFFSET_SCALE",
    "AST_ARRAY_INDEX",
    "AST_VAR_DECL",
    "AST_FUNC_DECL",
    "AST_FUNC_CALL",
    "AST_RETURN",
    "AST_IF",
    "AST_WHILE",
    "AST_DO_WHILE",
    "AST_FOR",
    "AST_BREAK"};
#define NodeToString(node) __ast_type_names[(node).type]

ASTNode_t *ast_create_node(ASTNode_type_e type, ASTNode_t *left, ASTNode_t *right, ASTNodeValue value);
ASTNode_t *ast_create_leaf_node(ASTNode_type_e type, ASTNodeValue value);
ASTNode_t *ast_get_parent_of_type(ASTNode_t *node, ASTNode_type_e type);
ASTNode_t *ast_flatten(ASTNode_t *node);
void ast_free(ASTNode_t *node);

#endif