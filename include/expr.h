#ifndef EXPR_H
#define EXPR_H

#include "ast.h"
#include "sym.h"
#include "defs.h"
#include "types.h"

#define MAX_STACK 500

ASTnode ASTnode_Order(Scanner s, SymTable st,  Token t);

void ASTnode_PrintTree(ASTnode n);


#endif