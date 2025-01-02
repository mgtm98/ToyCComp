#ifndef _STMT_H_
#define _STMT_H_

#include "ast.h"
#include "scanner.h"
#include "decl.h"

ASTNode_t *stmt_block(Scanner_t *scanner);

#endif
