#ifndef _DEBUG_H_
#define _DEBUG_H_

typedef enum
{
    SEV_DEBUG,
    SEV_ERROR,
    SEV_INFO
} Severity_e;

typedef struct ASTNode_t ASTNode_t;

void init_debugging(void);
void debug_print(const Severity_e severity, const char *format, ...);
void ast_print(ASTNode_t *node);

#endif // _DEBUG_H_