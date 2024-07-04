#ifndef DECL_H
#define DECL_H

#include "tokens.h"
#include "scan.h"
#include "sym.h"
#include "ast.h"
#include "stmt.h"

void var_declare(Scanner s, SymTable st, Token tok);
ASTnode function_declare(Scanner s, SymTable st, Token tok);

#endif