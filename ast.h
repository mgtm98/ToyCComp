#ifndef _AST_H_
#define _AST_H_

#include "scanner.h"

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
    AST_PRINT,
    AST_ASSIGN,
    AST_VAR,
    AST_DATATYPE,

    AST_VAR_DECL,
    AST_FUNC_DECL,

    AST_IF,

    AST_WHILE,
    AST_DO_WHILE,
    AST_FOR,
    AST_BREAK
} ASTNode_type_e;

#define ASTCheckLoopContext(t) t >= AST_WHILE &&t <= AST_FOR

typedef struct ASTNode_t ASTNode_t;
struct ASTNode_t
{
    ASTNode_type_e type;
    ASTNode_t *next;
    ASTNode_t *left;
    ASTNode_t *right;
    ASTNode_t *parent;
    int value;
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
    "AST_PRINT",
    "AST_ASSIGN",
    "AST_VAR",
    "AST_DATATYPE",
    "AST_VAR_DECL",
    "AST_FUNC_DECL",
    "AST_IF",
    "AST_WHILE",
    "AST_DO_WHILE",
    "AST_FOR",
    "AST_BREAK"};
#define NodeToString(node) __ast_type_names[(node).type]

/**
 * @brief Creates a new abstract syntax tree (AST) node.
 *
 * Allocates and initializes a new AST node with the specified type, left and right children,
 * and value. Sets the parent pointers for the child nodes.
 *
 * @param type The type of the AST node, specified as an enumeration value of `ASTNode_type_e`.
 * @param left A pointer to the left child node or `NULL` if there is no left child.
 * @param right A pointer to the right child node or `NULL` if there is no right child.
 * @param value An integer value associated with the node.
 * @return A pointer to the newly created AST node.
 *
 * @note The function terminates if memory allocation fails. Ensure that all nodes
 *       are properly freed when no longer needed to avoid memory leaks.
 */
ASTNode_t *ast_create_node(ASTNode_type_e type, ASTNode_t *left, ASTNode_t *right, int value);

/**
 * @brief Creates a new leaf node for abstract syntax tree (AST).
 *
 * Allocates and initializes a new AST node with the specified type and value,
 * with no child nodes (left and right children are set to `NULL`).
 *
 * @param type The type of the AST node, specified as an enumeration value of `ASTNode_type_e`.
 * @param value An integer value associated with the node.
 * @return ASTNode_t* A pointer to the newly created leaf node.
 *
 * @note This function is a convenience wrapper around `ast_create_node` for creating
 *       nodes without child relationships.
 */
ASTNode_t *ast_create_leaf_node(ASTNode_type_e type, int value);

/**
 * @brief Finds the nearest ancestor node of a specified type in an abstract syntax tree (AST).
 *
 * This function traverses upwards from the given AST node to locate the
 * closest ancestor node that matches the specified type. It starts from the
 * parent of the provided node and continues traversing until either a matching
 * node is found or the root of the tree is reached.
 *
 * @param node A pointer to the starting AST node. If this node is `NULL`, the function
 *             returns `NULL` immediately.
 * @param type The type of the AST node to search for, specified as an enumeration value
 *             of `ASTNode_type_e`.
 *
 * @return A pointer to the nearest ancestor node of the specified type. If no
 *         matching ancestor is found, the function returns `NULL`.
 */
ASTNode_t *ast_get_parent_of_type(ASTNode_t *node, ASTNode_type_e type);

#endif