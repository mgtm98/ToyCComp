#ifndef _EXPR_H_
#define _EXPR_H_

#include "scanner.h"
#include "ast.h"
#include "debug.h"

ASTNode_t *expr_expression(Scanner_t *scanner);
ASTNode_t *expr_assignment(Scanner_t *scanner);

#endif
