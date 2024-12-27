#include "debug.h"
#include "ast.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

static bool enable_debug = false;
static bool enable_info = false;

void init_debugging(void)
{
    if (getenv("TOYC_DEBUG") != NULL)
        enable_debug = true;
    if (getenv("TOYC_INFO") != NULL)
        enable_info = true;
}

void debug_print(const Severity_e severity, const char *format, ...)
{

    if (severity == SEV_ERROR)
    {
        printf("[ERROR] ");
    }
    else if (severity == SEV_DEBUG)
    {
        if (!enable_debug)
            return;
        printf("[DEBUG] ");
    }
    else if (severity == SEV_INFO)
    {
        if (!enable_info)
            return;
        printf("[INFO] ");
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

static void print_branches(int depth, int is_last)
{
    for (int i = 0; i < depth; i++)
        printf("   ");
    printf(is_last ? "`-- " : "|-- ");
}

static void ast_print_recursive(ASTNode_t *node, int depth, int is_last)
{
    if (node == NULL)
        return;

    while (node)
    {
        print_branches(depth, is_last);

        printf("%s", NodeToString(*node));
        if (node->type == AST_INT_LIT)
            printf(": %d", node->value);
        printf("\n");

        if (node->left || node->right)
        {
            if (node->left)
                ast_print_recursive(node->left, depth + 1, node->right == NULL);
            if (node->right)
                ast_print_recursive(node->right, depth + 1, 1);
        }
        node = node->next;
    }
}

void ast_print(ASTNode_t *node)
{
    ast_print_recursive(node, 0, 1);
}