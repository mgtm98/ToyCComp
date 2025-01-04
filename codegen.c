/**
 * @file codegen.c
 * @brief Implementation of the code generation phase for the compiler.
 *
 * This file contains functions for generating assembly code from an Abstract
 * Syntax Tree (AST). It processes various types of statements and expressions,
 * as specified in the BNF grammar file, to produce the final output assembly code.
 *
 * @author Mohamed Gamal
 * @project ToyCComp
 * @date 2024-12-24
 *
 * @note This file is part of the ToyCComp project, which is a hobby compiler
 *       project designed for learning and experimenting with compiler design.
 */

#include "asm.h"
#include "ast.h"
#include "codegen.h"
#include "debug.h"
#include "symtab.h"

#include <stdlib.h>

//////////////////////////////
// All functions protptypes //
//////////////////////////////

static ASTNode_t *get_loop_context(ASTNode_t *root);

static void generate_statements(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_statement(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_print(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_if(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_while(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_do_while(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_for(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_break(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_assign(CodeGenerator_t *gen, ASTNode_t *root);

static Register generate_expr(CodeGenerator_t *gen, ASTNode_t *root);
static Register generate_comparison(CodeGenerator_t *gen, ASTNode_t *root);
static Register generate_arithmetic(CodeGenerator_t *gen, ASTNode_t *root);

static void generate_declerations(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_decleration(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_decl_func(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_decl_var(CodeGenerator_t *gen, ASTNode_t *root);
//////////////////////////////
//////////////////////////////

static ASTNode_t *get_loop_context(ASTNode_t *node)
{
    if (node == NULL)
        return NULL;

    node = node->parent;
    while (node != NULL)
    {
        if (ASTCheckLoopContext(node->type))
            return node;
        node = node->parent;
    }
    return NULL;
}

static Register generate_expr(CodeGenerator_t *gen, ASTNode_t *root)
{
    return generate_comparison(gen, root);
}

static Register generate_comparison(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register left, right;

    if (
        root->type != AST_COMP_EQ &&
        root->type != AST_COMP_NE &&
        root->type != AST_COMP_GT &&
        root->type != AST_COMP_GE &&
        root->type != AST_COMP_LT &&
        root->type != AST_COMP_LE)
    {
        return generate_arithmetic(gen, root);
    }

    left = generate_arithmetic(gen, root->left);
    right = generate_arithmetic(gen, root->right);

    switch (root->type)
    {
    case AST_COMP_EQ:
        return asm_comp_eq(gen, left, right);
    case AST_COMP_NE:
        return asm_comp_ne(gen, left, right);
    case AST_COMP_GT:
        return asm_comp_gt(gen, left, right);
    case AST_COMP_GE:
        return asm_comp_ge(gen, left, right);
    case AST_COMP_LT:
        return asm_comp_lt(gen, left, right);
    case AST_COMP_LE:
        return asm_comp_le(gen, left, right);

    default:
        debug_print(
            SEV_ERROR,
            "[CG] Unexpected type: %s in generate_comparison\n",
            NodeToString(*root));
        exit(1);
    }
}

static Register generate_arithmetic(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register left, right;

    if (root->left)
        left = generate_arithmetic(gen, root->left);

    if (root->right)
        right = generate_arithmetic(gen, root->right);

    switch (root->type)
    {
    case AST_ADD:
        return asm_add(gen, left, right);
    case AST_SUBTRACT:
        return asm_sub(gen, left, right);
    case AST_MULT:
        return asm_mul(gen, left, right);
    case AST_DIV:
        return asm_div(gen, left, right);
    case AST_INT_LIT:
        return asm_init_register(gen, root->value);
    case AST_VAR:
        return asm_get_global_var(gen, symtab_get_symbol(root->value)->sym_name);

    default:
        debug_print(
            SEV_ERROR,
            "[CG] Unexpected type: %s in generate_arithmetic\n",
            NodeToString(*root));
        exit(1);
    }
}

static void generate_statements(CodeGenerator_t *gen, ASTNode_t *root)
{
    while (root)
    {
        generate_statement(gen, root);
        root = root->next;
    }
}

static void generate_statement(CodeGenerator_t *gen, ASTNode_t *root)
{
    switch (root->type)
    {
    case AST_PRINT:
        generate_stmt_print(gen, root);
        break;
    case AST_VAR_DECL:
        generate_decl_var(gen, root);
        break;
    case AST_ASSIGN:
        generate_stmt_assign(gen, root);
        break;
    case AST_IF:
        generate_stmt_if(gen, root);
        break;
    case AST_WHILE:
        generate_stmt_while(gen, root);
        break;
    case AST_DO_WHILE:
        generate_stmt_do_while(gen, root);
        break;
    case AST_FOR:
        generate_stmt_for(gen, root);
        break;
    case AST_BREAK:
        generate_stmt_break(gen, root);
        break;
    case AST_EMPTY:
        break;
    default:
        debug_print(SEV_ERROR, "[CG] Unexpected node: %s", NodeToString(*root));
        exit(0);
    }
}

static void generate_stmt_print(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register r = generate_expr(gen, root->left);
    asm_print(gen, r);
}

static void generate_decl_var(CodeGenerator_t *gen, ASTNode_t *root)
{
    // TODO check if the variable has initial value
    asm_add_global_var(
        gen,
        symtab_get_symbol(root->value)->sym_name,
        (RegSize_e)root->expr_type->size);
    if (root->left)
    {
        Register value = generate_expr(gen, root->left);
        asm_set_global_var(gen, symtab_get_symbol(root->value)->sym_name, value);
    }
}

static void generate_stmt_if(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register comp;
    LabelId false_label = asm_generate_label();
    LabelId end_label = asm_generate_label();

    comp = generate_expr(gen, root->left);
    asm_jmp_ne(gen, comp, 1, false_label);
    generate_statements(gen, root->right->left);
    asm_jmp(gen, end_label);
    asm_lbl(gen, false_label);
    if (root->right->right)
    {
        generate_statements(gen, root->right->right);
        asm_jmp(gen, end_label);
    }
    asm_lbl(gen, end_label);
}

static void generate_stmt_while(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register comp;
    LabelId start_label = asm_generate_label();
    LabelId end_label = asm_generate_label();

    // Save the end label as the value of the node. This will be used
    // by the break statement
    root->value = end_label;

    asm_lbl(gen, start_label);
    comp = generate_expr(gen, root->left);
    asm_jmp_ne(gen, comp, 1, end_label);
    generate_statements(gen, root->right);
    asm_jmp(gen, start_label);
    asm_lbl(gen, end_label);
}

static void generate_stmt_do_while(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register comp;
    LabelId start_label = asm_generate_label();
    LabelId end_label = asm_generate_label();

    root->value = end_label;
    asm_lbl(gen, start_label);
    generate_statements(gen, root->right);
    comp = generate_expr(gen, root->left);
    asm_jmp_eq(gen, comp, 1, start_label);
    asm_lbl(gen, end_label);
}

static void generate_stmt_for(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register comp;
    ASTNode_t *init = root->left;
    ASTNode_t *cond = root->left->next;
    ASTNode_t *update = root->left->next->next;
    LabelId start_label = asm_generate_label();
    LabelId end_label = asm_generate_label();

    root->value = end_label;

    generate_statement(gen, init);
    asm_lbl(gen, start_label);
    comp = generate_expr(gen, cond);
    asm_jmp_ne(gen, comp, 1, end_label);
    generate_statements(gen, root->right);
    generate_statement(gen, update);
    asm_jmp(gen, start_label);
    asm_lbl(gen, end_label);
}

static void generate_stmt_break(CodeGenerator_t *gen, ASTNode_t *root)
{
    // TODO: Move this check early beforce codegen stage
    ASTNode_t *loop_node = get_loop_context(root);
    if (loop_node == NULL)
    {
        debug_print(SEV_ERROR, "[CG] break statement was called outside a loop context");
        exit(1);
    }
    asm_jmp(gen, (LabelId)loop_node->value);
}

static void generate_stmt_assign(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register i = generate_expr(gen, root->right);
    asm_set_global_var(gen, symtab_get_symbol(root->left->value)->sym_name, i);
}

static void generate_declerations(CodeGenerator_t *gen, ASTNode_t *root)
{
    while (root)
    {
        generate_decleration(gen, root);
        root = root->next;
    }
}

static void generate_decleration(CodeGenerator_t *gen, ASTNode_t *root)
{
    switch (root->type)
    {
    case AST_FUNC_DECL:
        generate_decl_func(gen, root);
        break;
    case AST_VAR_DECL:
        generate_decl_var(gen, root);
        break;
    default:
        debug_print(SEV_ERROR, "[CG] Unexpected declaration type found");
        exit(1);
    }
}

static void generate_decl_func(CodeGenerator_t *gen, ASTNode_t *root)
{
    asm_generate_function_prologue(gen, symtab_get_symbol(root->value)->sym_name);
    generate_statements(gen, root->left);
    asm_generate_function_epilogue(gen);
}

CodeGenerator_t *codegen_init(char *path)
{
    CodeGenerator_t *gen = (CodeGenerator_t *)malloc(sizeof(CodeGenerator_t));
    gen->file = fopen(path, "w");
    return gen;
}

void codegen_start(CodeGenerator_t *gen, ASTNode_t *root)
{
    generate_declerations(gen, root);
    asm_wrapup(gen);
}