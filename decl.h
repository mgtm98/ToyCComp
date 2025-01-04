#ifndef _DECL_H_
#define _DECL_H_

#include "ast.h"
#include "scanner.h"

ASTNode_t *decl_declarations(Scanner_t *scanner);
ASTNode_t *decl_var(Scanner_t *scanner);

// TODO: Move this to utils/common
ASTNode_t *args(Scanner_t *scanner);
int args_count(ASTNode_t *args);

#endif
