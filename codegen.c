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

#include <math.h>
#include <stdlib.h>

//////////////////////////////
// All functions protptypes //
//////////////////////////////

static ASTNode_t *get_loop_context(ASTNode_t *root);

static void generate_statements(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_statement(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_if(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_while(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_do_while(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_for(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_break(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_assign(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_return(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_fcall(CodeGenerator_t *gen, ASTNode_t *root);

static Register generate_expr(CodeGenerator_t *gen, ASTNode_t *root);
static Register generate_expr_comparison(CodeGenerator_t *gen, ASTNode_t *root);
static Register generate_expr_arithmetic(CodeGenerator_t *gen, ASTNode_t *root);
static Register generate_expr_fcall(CodeGenerator_t *gen, ASTNode_t *root);
static Register generate_expr_addressof(CodeGenerator_t *gen, ASTNode_t *root);
static Register generate_expr_ptrdref(CodeGenerator_t *gen, ASTNode_t *root);
static Register generate_expr_arr_index(CodeGenerator_t *gen, ASTNode_t *root);

static void generate_declerations(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_decleration(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_decl_func(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_decl_var(CodeGenerator_t *gen, ASTNode_t *root);
//////////////////////////////
//////////////////////////////

static bool return_called_flag = false;

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
    switch (root->type)
    {
    case AST_FUNC_CALL:
        return generate_expr_fcall(gen, root);
    case AST_ADDRESSOF:
        return generate_expr_addressof(gen, root);
    default:
        return generate_expr_comparison(gen, root);
    }
}

static Register generate_expr_comparison(CodeGenerator_t *gen, ASTNode_t *root)
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
        return generate_expr_arithmetic(gen, root);
    }

    left = generate_expr(gen, root->left);
    right = generate_expr(gen, root->right);

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
            "[CG] Unexpected type: %s in generate_expr_comparison\n",
            NodeToString(*root));
        exit(1);
    }
}

static Register generate_expr_arithmetic(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register left, right;

    if (
        root->left &&
        root->type != AST_FUNC_CALL &&
        root->type != AST_PTRDREF &&
        root->type != AST_ARRAY_INDEX)
        left = generate_expr(gen, root->left);

    if (root->right && root->type != AST_ARRAY_INDEX)
        right = generate_expr(gen, root->right);

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
        return asm_init_register(gen, root->value.num);
    case AST_STR_LIT:
        return asm_address_of(gen, asm_generate_string_lit(gen, root->value.str));

    case AST_VAR:
        return asm_get_global_var(gen, symtab_get_symbol(root->value.num)->sym_name);
    case AST_OFFSET_SCALE:
        Register offset = asm_init_register(gen, root->value.num);
        return asm_mul(gen, left, offset);
    case AST_PTRDREF:
        if (root->expr_type->pointer_level > 0)
            return generate_expr_ptrdref(gen, root);
        else
            return asm_load_mem(
                gen,
                generate_expr_ptrdref(gen, root),
                root->expr_type->size);
    case AST_ARRAY_INDEX:
        return asm_load_mem(
            gen,
            generate_expr_arr_index(gen, root),
            root->expr_type->size);

    default:
        debug_print(
            SEV_ERROR,
            "[CG] Unexpected type: %s in generate_expr_arithmetic\n",
            NodeToString(*root));
        exit(1);
    }
}

static Register generate_expr_fcall(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register expr = asm_NoReg;
    if (root->left)
        expr = generate_expr(gen, root->left);

    return asm_generate_func_call(
        gen,
        symtab_get_symbol(root->value.num)->sym_name,
        expr,
        true);
}

static Register generate_expr_addressof(CodeGenerator_t *gen, ASTNode_t *root)
{
    return asm_address_of(gen, symtab_get_symbol(root->left->value.num)->sym_name);
}

static Register generate_expr_ptrdref(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register expr = generate_expr(gen, root->left);
    if (root->expr_type->pointer_level == 0)
        return expr;
    else
        return asm_load_mem(gen, expr, root->expr_type->size);
}

static Register generate_expr_arr_index(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register index, base_address;

    index = generate_expr(gen, root->right);
    asm_sll(gen, index, (int)log2(root->value.num / 8));
    base_address = asm_address_of(gen, symtab_get_symbol(root->left->value.num)->sym_name);
    return asm_add(gen, base_address, index);
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
    case AST_RETURN:
        generate_stmt_return(gen, root);
        break;
    case AST_FUNC_CALL:
        generate_stmt_fcall(gen, root);
        break;
    default:
        debug_print(SEV_ERROR, "[CG] Unexpected node: %s", NodeToString(*root));
        exit(0);
    }
}

static void generate_decl_var(CodeGenerator_t *gen, ASTNode_t *root)
{
    // TODO check if the variable has initial value
    Symbol_t *symbol = symtab_get_symbol(root->value.num);
    asm_add_global_var(
        gen,
        symbol->sym_name,
        (RegSize_e)root->expr_type->size,
        symbol->data_type->array_size);
    if (root->left)
    {
        if (root->left->type == AST_INT_LIT)
        {
            asm_set_global_var_initial_val(
                gen,
                symtab_get_symbol(root->value.num)->sym_name,
                (ASMSymbolValue)root->left->value.num,
                ASM_SYMBOL_INT);
        }
        else if (root->left->type == AST_STR_LIT)
        {
            asm_set_global_var_initial_val(
                gen,
                symtab_get_symbol(root->value.num)->sym_name,
                (ASMSymbolValue)root->left->value.str,
                ASM_SYMBOL_STR);
        }
        else
        {
            Register value = generate_expr(gen, root->left);
            asm_set_global_var(gen, symtab_get_symbol(root->value.num)->sym_name, value);
        }
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
    root->value.num = end_label;

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

    root->value.num = end_label;
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

    root->value.num = end_label;

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
    asm_jmp(gen, (LabelId)loop_node->value.num);
}

static void generate_stmt_assign(CodeGenerator_t *gen, ASTNode_t *root)
{
    Symbol_t *symbol;
    Register i = generate_expr(gen, root->right);

    switch (root->left->type)
    {
    case AST_VAR:
        symbol = symtab_get_symbol(root->left->value.num);
        asm_set_global_var(gen, symbol->sym_name, i);
        break;

    case AST_PTRDREF:
        Register expr1 = generate_expr_ptrdref(gen, root->left);
        asm_store_mem(gen, expr1, i, root->left->expr_type->size);
        break;

    case AST_ARRAY_INDEX:
        Register expr2 = generate_expr_arr_index(gen, root->left);
        asm_store_mem(gen, expr2, i, root->left->expr_type->size);
        break;

    default:
        debug_print(SEV_ERROR, "[CG] Unsupported lvalue type %s", NodeToString(*(root->left)));
        exit(1);
        break;
    }
}

static void generate_stmt_return(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register i = generate_expr(gen, root->left);
    asm_generate_func_return(gen, i, symtab_get_symbol(root->value.num)->data_type->size);
    return_called_flag = true;
}

static void generate_stmt_fcall(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register expr = asm_NoReg;
    if (root->left)
        expr = generate_expr(gen, root->left);

    asm_generate_func_call(
        gen,
        symtab_get_symbol(root->value.num)->sym_name,
        expr,
        false);
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
    return_called_flag = false;
    asm_generate_function_prologue(gen, symtab_get_symbol(root->value.num)->sym_name);
    generate_statements(gen, root->left);
    if (!return_called_flag)
    {
        Register i = asm_init_register(gen, 0);
        asm_generate_func_return(gen, i, SIZE_8bit);
    }
    asm_generate_function_epilogue(gen);
}

CodeGenerator_t *codegen_init(char *path)
{
    CodeGenerator_t *gen = (CodeGenerator_t *)malloc(sizeof(CodeGenerator_t));
    gen->file = fopen(path, "w");
    // gen->file = stdout;
    return gen;
}

void codegen_start(CodeGenerator_t *gen, ASTNode_t *root)
{
    generate_declerations(gen, root);
    asm_wrapup(gen);
}