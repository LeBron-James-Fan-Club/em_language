#ifndef EXPR_H
#define EXPR_H

#include "ast.h"
#include "defs.h"
#include "sym.h"
#include "types.h"

#define MAX_STACK 500

ASTnode ASTnode_Order(Scanner s, SymTable st, Token t, Context ctx);
ASTnode expression_list(Scanner s, SymTable st, Token tok, Context ctx,
                        enum OPCODES endToken);
                               
#endif