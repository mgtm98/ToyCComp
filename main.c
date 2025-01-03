#include "scanner.h"
#include "debug.h"
#include "ast.h"
#include "decl.h"
#include "codegen.h"
#include "symtab.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    init_debugging();
    symtab_init_global_symtab();
    Scanner_t *scanner = scanner_init("/mnt/d/ToyCComp/tests/scanner/test1");
    ASTNode_t *root = decl_declarations(scanner);
    if (root == NULL)
    {
        debug_print(SEV_ERROR, "Couldn't create root node");
    }
    ast_print(root);

    CodeGenerator_t *generator = codegen_init("out.s");
    codegen_start(generator, root);

    return 0;
}
