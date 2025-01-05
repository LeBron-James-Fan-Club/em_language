#ifndef EXPR_H
#define EXPR_H

#include "ast.h"
#include "defs.h"
#include "sym.h"
#include "types.h"

#define MAX_STACK 500

// forward declaration
int parse_stars(Scanner s, Token tok, enum ASTPRIM type);
enum ASTPRIM parse_cast(Compiler c, Scanner s, SymTable st, Token tok,
                        Context ctx, SymTableEntry *cType);

ASTnode ASTnode_Order(Compiler c, Scanner s, SymTable st, Token t, Context ctx,
                      int prePreced);
ASTnode expression_list(Compiler c, Scanner s, SymTable st, Token tok, Context ctx,
                        enum OPCODES endToken);

#endif