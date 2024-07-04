#ifndef AST_H
#define AST_H

#include "tokens.h"
#include "scan.h"
#include "sym.h"

enum ASTOP {
    // 1:1 (almost) with tokens
    A_ADD = 1,
    A_SUBTRACT,
    A_MULTIPLY,
    A_DIVIDE,
    A_MODULO,
    
    A_EQ, A_NE,
    A_LT, A_GT,
    A_LE, A_GE,

    A_INTLIT,

    // After this we need checks
    A_IDENT,
    A_LVIDENT,
    A_ASSIGN,
    A_PRINT,
    A_INPUT,
    A_GLUE,
    A_IF,
    A_LABEL,
    A_GOTO,
    
    // for is also A_WHILE
    A_WHILE,
    A_FUNCTION 
    
};

struct astnode {
    enum ASTOP op;
    struct astnode *left;
    struct astnode *mid;
    struct astnode *right;
    union {
        int intvalue; // for INTLIT
        int id; // Var lookup
    } v;
};

typedef struct astnode *ASTnode;

ASTnode ASTnode_New(enum ASTOP op, ASTnode left, ASTnode mid, ASTnode right, int intvalue);
void ASTnode_Free(ASTnode this);
ASTnode ASTnode_NewLeaf(enum ASTOP op, int intvalue);
ASTnode ASTnode_NewUnary(enum ASTOP op, ASTnode left, int intvalue);

ASTnode ASTnode_Expr(Scanner s, Token t);
ASTnode ASTnode_Order(Scanner s, SymTable st,  Token t);

void ASTnode_PrintTree(ASTnode n);

#endif