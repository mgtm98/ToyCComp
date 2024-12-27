#include "ast.h"
#include "debug.h"

#include <stdlib.h>

ASTNode_t *ast_create_node(ASTNode_type_e type, ASTNode_t *left, ASTNode_t *right, int value)
{
    ASTNode_t *node = (ASTNode_t *)malloc(sizeof(ASTNode_t));
    if (node == NULL)
    {
        debug_print(SEV_ERROR, "Can't create an AST node");
        exit(1);
    }

    node->type = type;
    node->left = left;
    node->right = right;
    node->value = value;
    node->next = NULL;

    while (left)
    {
        left->parent = node;
        left = left->next;
    }

    while (right)
    {
        right->parent = node;
        right = right->next;
    }

    debug_print(SEV_DEBUG, "Creating %s with value=%d", NodeToString(*node), value);

    return node;
}

ASTNode_t *ast_create_leaf_node(ASTNode_type_e type, int value)
{
    return ast_create_node(type, NULL, NULL, value);
}

ASTNode_t *ast_get_parent_of_type(ASTNode_t *node, ASTNode_type_e type)
{
    if (node == NULL)
        return NULL;

    printf("node type is %s\n", NodeToString(*node));

    node = node->parent;
    printf("parent type is %s\n", NodeToString(*node));

    while (node != NULL)
    {
        printf("parent type is %s\n", NodeToString(*node));
        if (node->type == type)
            return node;
        node = node->parent;
    }
    return NULL;
}