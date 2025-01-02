#ifndef _DECL_H_
#define _DECL_H_

#include "ast.h"
#include "scanner.h"

ASTNode_t *decl_declarations(Scanner_t *scanner);
ASTNode_t *decl_var(Scanner_t *scanner);

#endif
