#ifndef DECL_H
#define DECL_H

#include "ast.h"
#include "defs.h"
#include "scan.h"
#include "stmt.h"
#include "sym.h"
#include "context.h"

enum ASTOP parse_type(Scanner s, Token tok);
void var_declare(Scanner s, SymTable st, Token tok);
ASTnode function_declare(Compiler c, Scanner s, SymTable st, Token tok, Context ctx) ;

#endif