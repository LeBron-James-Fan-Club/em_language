#ifndef DECL_H
#define DECL_H

#include "ast.h"
#include "context.h"
#include "defs.h"
#include "flags.h"
#include "gen.h"
#include "scan.h"
#include "stmt.h"
#include "sym.h"

enum ASTPRIM parse_type(Scanner s, SymTable st, Token tok, SymTableEntry *ctype);

void var_declare(Scanner s, SymTable st, Token tok, enum ASTPRIM type,
                 SymTableEntry cType, enum STORECLASS store);
void global_declare(Compiler c, Scanner s, SymTable st, Token tok, Context ctx);
ASTnode function_declare(Compiler c, Scanner s, SymTable st, Token tok,
                         Context ctx, enum ASTPRIM type);

#endif