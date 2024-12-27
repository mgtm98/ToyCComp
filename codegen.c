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
static void generate_stmt_var_decl(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_if(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_while(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_do_while(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_for(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_break(CodeGenerator_t *gen, ASTNode_t *root);
static void generate_stmt_assign(CodeGenerator_t *gen, ASTNode_t *root);

static Register generate_expr(CodeGenerator_t *gen, ASTNode_t *root);
static Register generate_comparison(CodeGenerator_t *gen, ASTNode_t *root);
static Register generate_arithmetic(CodeGenerator_t *gen, ASTNode_t *root);

// ============================================================
//                     Helper Functions
// ============================================================

/**
 * @brief Retrieves the nearest loop context in the abstract syntax tree (AST).
 *
 * Traverses upwards from the given AST node to find the closest ancestor node
 * representing a loop context.
 *
 * @param node A pointer to the starting AST node. If `NULL`, the function returns `NULL`.
 * @return ASTNode_t* A pointer to the nearest loop context node, or `NULL` if no such node exists.
 */
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

/**
 * @brief Generates assembly code for a given expression.
 *
 * This function generates code for evaluating an expression represented by
 * the AST (Abstract Syntax Tree) node. The expression can be one of the
 * following:
 * - A comparison expression (e.g., `==`, `<`, `>`).
 * - An additive expression (e.g., `+`, `-`).
 * - A variable reference.
 * - A literal value (e.g., a numeric constant).
 *
 * The function delegates the generation process to the appropriate handler
 * for comparison expressions (`generate_comparison`) as the entry point.
 * For details about supported expressions, refer to the BNF rules.
 *
 * @param gen Pointer to the code generator context.
 * @param root Pointer to the AST node representing the expression.
 * @return Register The register containing the result of the expression.
 */
static Register generate_expr(CodeGenerator_t *gen, ASTNode_t *root)
{
    return generate_comparison(gen, root);
}

/**
 * @brief Generates assembly code for a comparison expression.
 *
 * This function handles the generation of code for comparison expressions
 * in the Abstract Syntax Tree (AST). If the given `root` node does not
 * represent a comparison operation, the function delegates to
 * `generate_arithmetic` to handle the expression.
 *
 * Supported comparison operations:
 * - Equality (`==`)         - `AST_COMP_EQ`
 * - Inequality (`!=`)       - `AST_COMP_NE`
 * - Greater Than (`>`)      - `AST_COMP_GT`
 * - Greater or Equal (`>=`) - `AST_COMP_GE`
 * - Less Than (`<`)         - `AST_COMP_LT`
 * - Less or Equal (`<=`)    - `AST_COMP_LE`
 *
 * @param gen Pointer to the code generator context.
 * @param root Pointer to the AST node representing the comparison expression.
 * @return Register The register containing the result of the comparison.
 *
 * @note This function assumes that comparison operations result in a boolean
 *       value stored in a register (e.g., 0 for false, non-zero for true).
 * @note For details about arithmetic expressions, see `generate_arithmetic`.
 */
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

/**
 * @brief Generates assembly code for arithmetic expressions.
 *
 * This function generates code for evaluating arithmetic expressions represented
 * in the Abstract Syntax Tree (AST). It recursively evaluates the left and right
 * child nodes, depending on the operation type, and generates the corresponding
 * assembly instructions.
 *
 * Supported arithmetic operations:
 * - Addition (`+`)           - `AST_ADD`
 * - Subtraction (`-`)        - `AST_SUBTRACT`
 * - Multiplication (`*`)     - `AST_MULT`
 * - Division (`/`)           - `AST_DIV`
 *
 * Supported leaf nodes:
 * - Integer literal           - `AST_INT_LIT`
 * - Variable reference        - `AST_VAR`
 *
 * @param gen Pointer to the code generator context.
 * @param root Pointer to the AST node representing the arithmetic expression.
 * @return Register The register containing the result of the evaluated expression.
 *
 * @note This function assumes all variables are global and can be retrieved from
 *       the symbol table.
 */
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
        return asm_get_global_var(gen, symtab_get_symbol_name(root->value));

    default:
        debug_print(
            SEV_ERROR,
            "[CG] Unexpected type: %s in generate_arithmetic\n",
            NodeToString(*root));
        exit(1);
    }
}

/**
 * @brief Generates assembly code for a sequence of statements.
 *
 * This function processes a linked list of AST nodes, each representing a statement,
 * and generates the corresponding assembly code for each statement. It iterates
 * through the linked list using the `next` pointer in each node.
 *
 * @param gen Pointer to the code generator context.
 * @param root Pointer to the first AST node in the sequence of statements.
 *
 * @note The `next` field in each AST node is used to traverse the sequence.
 */
static void generate_statements(CodeGenerator_t *gen, ASTNode_t *root)
{
    while (root)
    {
        generate_statement(gen, root);
        root = root->next;
    }
}

/**
 * @brief Generates assembly code for a single statement.
 *
 * This function processes an AST node representing a statement and
 * delegates the code generation to the appropriate function based on the
 * type of the statement. The following types of statements are supported:
 *
 * - Print statement (`AST_PRINT`): Generates code to print a value.
 * - Variable declaration (`AST_VAR_DECL`): Handles code generation for variable declarations.
 * - Assignment statement (`AST_ASSIGN`): Generates code to assign a value to a variable.
 * - If statement (`AST_IF`): Handles conditional branching.
 *
 * @param gen Pointer to the code generator context.
 * @param root Pointer to the AST node representing the statement.
 *
 * @note The function logs an error and terminates the program if the statement
 *       type is not recognized.
 */
static void generate_statement(CodeGenerator_t *gen, ASTNode_t *root)
{
    switch (root->type)
    {
    case AST_PRINT:
        generate_stmt_print(gen, root);
        break;
    case AST_VAR_DECL:
        generate_stmt_var_decl(gen, root);
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

/**
 * @brief Generates assembly code for a print statement.
 *
 * This function generates code to print the result of an expression. It first
 * evaluates the expression specified in the `left` child of the AST node, then
 * emits the appropriate assembly instruction to print the value.
 *
 * @param gen Pointer to the code generator context.
 * @param root Pointer to the AST node representing the print statement.
 *
 * @note This function assumes that the `left` child of the `root` node contains
 *       the expression to be printed.
 */
static void generate_stmt_print(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register r = generate_expr(gen, root->left);
    asm_print(gen, r);
}

/**
 * @brief Generates assembly code for a variable declaration.
 *
 * This function handles the code generation for declaring a global variable.
 * It retrieves the variable's name from the symbol table and emits the
 * appropriate assembly instructions to allocate storage for the variable.
 *
 * @param gen Pointer to the code generator context.
 * @param root Pointer to the AST node representing the variable declaration.
 *
 * @note This function assumes that `root->value` contains the identifier
 *       (symbol table entry) for the variable being declared.
 */
static void generate_stmt_var_decl(CodeGenerator_t *gen, ASTNode_t *root)
{
    asm_add_global_var(gen, symtab_get_symbol_name(root->value));
}

/**
 * @brief Generates assembly code for an if statement.
 *
 * This function generates code for an if-else construct by:
 * - Evaluating the condition expression.
 * - Emitting a conditional jump to the `else` or `end` label based on the condition.
 * - Generating code for the true branch and optionally for the false (else) branch.
 * - Managing the control flow using labels for branching.
 *
 * @param gen Pointer to the code generator context.
 * @param root Pointer to the AST node representing the if statement.
 *
 * @note This function assumes:
 * - `root->left` contains the condition expression.
 * - `root->right->left` contains the true branch statements.
 * - `root->right->right` (optional) contains the false (else) branch statements.
 */
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

/**
 * @brief Generates assembly code for a `while` statement in the abstract syntax tree (AST).
 *
 * This function emits assembly code for a `while` loop, including condition evaluation,
 * loop body execution, and branching logic. It uses labels to manage the control flow
 * and ensures proper handling of loop termination.
 *
 * @param gen A pointer to the `CodeGenerator_t` structure that contains the output file stream.
 * @param root A pointer to the AST node representing the `while` statement.
 *             - The left child node represents the loop condition.
 *             - The right child node represents the loop body.
 */
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

/**
 * @brief Generates assembly code for a `do-while` statement in the abstract syntax tree (AST).
 *
 * This function emits assembly code to handle the control flow of a `do-while` loop,
 * ensuring the loop body is executed at least once before evaluating the loop condition.
 *
 * @param gen A pointer to the `CodeGenerator_t` structure that contains the output file stream.
 * @param root A pointer to the AST node representing the `do-while` statement.
 *             - The left child node represents the loop condition.
 *             - The right child node represents the loop body.
 *
 * @details
 * - Two unique labels are generated:
 *   - `start_label`: Marks the start of the loop.
 *   - `end_label`: Marks the point to exit the loop if the condition fails.
 * - The loop body is executed first, followed by the evaluation of the condition.
 * - If the condition evaluates to true, the code jumps back to the start of the loop.
 * - The end label is stored in the `value` field of the root node to support `break` statements.
 */
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

/**
 * @brief Generates assembly code for a `for` statement in the abstract syntax tree (AST).
 *
 * This function emits assembly code to handle the control flow of a `for` loop,
 * including initialization, condition evaluation, loop body execution, and update step.
 *
 * @param gen A pointer to the `CodeGenerator_t` structure that contains the output file stream.
 * @param root A pointer to the AST node representing the `for` statement.
 *             - The left child node contains:
 *               1. The initialization statement (`init`).
 *               2. The loop condition (`cond`), accessible via `init->next`.
 *               3. The update statement (`update`), accessible via `cond->next`.
 *             - The right child node contains the loop body as a block of statements.
 *
 * @details
 * - Two unique labels are generated:
 *   - `start_label`: Marks the start of the loop for condition evaluation.
 *   - `end_label`: Marks the exit point of the loop.
 * - The `for` loop components are processed in the following order:
 *   1. The initialization statement is executed.
 *   2. The condition is evaluated, and control exits the loop if the condition is false.
 *   3. The loop body is executed.
 *   4. The update statement is executed.
 *   5. Control jumps back to the condition check.
 * - The `end_label` is stored in the `value` field of the root node to support `break` statements.
 */
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

/**
 * @brief Generates assembly code for a `break` statement in the abstract syntax tree (AST).
 *
 * This function emits a jump instruction to exit the nearest enclosing loop.
 * It verifies that the `break` statement is used within a valid loop context.
 *
 * @param gen A pointer to the `CodeGenerator_t` structure that contains the output file stream.
 * @param root A pointer to the AST node representing the `break` statement.
 */
static void generate_stmt_break(CodeGenerator_t *gen, ASTNode_t *root)
{
    ASTNode_t *loop_node = get_loop_context(root);
    if (loop_node == NULL)
    {
        debug_print(SEV_ERROR, "[CG] break statement was called outside a loop context");
        exit(1);
    }
    asm_jmp(gen, (LabelId)loop_node->value);
}

/**
 * @brief Generates assembly code for an assignment statement.
 *
 * This function generates code to assign the result of an expression to a variable.
 * It evaluates the expression on the right-hand side, then emits the appropriate
 * assembly instruction to store the value in the specified variable.
 *
 * @param gen Pointer to the code generator context.
 * @param root Pointer to the AST node representing the assignment statement.
 *
 * @note This function assumes:
 * - `root->left` contains the variable being assigned.
 * - `root->right` contains the expression whose value is to be assigned.
 * - The variable is a global variable, and its name can be retrieved from the
 *   symbol table using `symtab_get_symbol_name`.
 */
static void
generate_stmt_assign(CodeGenerator_t *gen, ASTNode_t *root)
{
    Register i = generate_expr(gen, root->right);
    asm_set_global_var(gen, symtab_get_symbol_name(root->left->value), i);
}

CodeGenerator_t *codegen_init(char *path)
{
    CodeGenerator_t *gen = (CodeGenerator_t *)malloc(sizeof(CodeGenerator_t));
    gen->file = fopen(path, "w");
    return gen;
}

void codegen_start(CodeGenerator_t *gen, ASTNode_t *root)
{
    asm_init(gen);
    generate_statements(gen, root);
    asm_wrapup(gen);
}