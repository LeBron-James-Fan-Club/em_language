#ifndef EXPR_H
#define EXPR_H

#include "ast.h"
#include "sym.h"
#include "defs.h"
#include "types.h"

#define MAX_STACK 500

ASTnode ASTnode_Order(Scanner s, SymTable st,  Token t);

void ASTnode_PrintTree(ASTnode n);
ASTnode ASTnode_FuncCall(Scanner s, SymTable st, Token tok);
ASTnode ASTnode_Prefix(Scanner s, SymTable st, Token tok);


#endif