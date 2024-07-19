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

enum ASTPRIM parse_type(Scanner s, Token tok);
void var_declare(Compiler c, Scanner s, SymTable st, Token tok,
                 enum ASTPRIM type, bool isLocal);
void global_declare(Compiler c, Scanner s, SymTable st, Token tok, Context ctx,
                    Flags f);
ASTnode function_declare(Compiler c, Scanner s, SymTable st, Token tok,
                         Context ctx, enum ASTPRIM type);

#endif