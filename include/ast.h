#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "scan.h"
#include "sym.h"
#include "types.h"

struct astnode {
    enum ASTOP op;
    enum ASTPRIM type;
    struct astnode *left;
    struct astnode *mid;
    struct astnode *right;
    union {
        int intvalue; // for INTLIT
        int id; // Var lookup
    } v;
};

typedef struct astnode *ASTnode;


ASTnode ASTnode_New(enum ASTOP op, enum ASTPRIM type, ASTnode left, ASTnode mid,
                    ASTnode right, int intvalue);
void ASTnode_Free(ASTnode this);
ASTnode ASTnode_NewLeaf(enum ASTOP op, enum ASTPRIM type, int intvalue);
ASTnode ASTnode_NewUnary(enum ASTOP op, enum ASTPRIM type, ASTnode left,
                         int intvalue);

ASTnode ASTnode_Order(Scanner s, SymTable st,  Token t);

void ASTnode_PrintTree(ASTnode n);
ASTnode ASTnode_FuncCall(Scanner s, SymTable st, Token tok);
ASTnode ASTnode_Prefix(Scanner s, SymTable st, Token tok);

#endif