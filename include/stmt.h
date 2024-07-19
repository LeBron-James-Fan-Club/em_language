#ifndef STMT_H
#define STMT_H

#include "scan.h"
#include "gen.h"
#include "sym.h"
#include "defs.h"
#include "decl.h"
#include "expr.h"
#include "types.h"

ASTnode Compound_Statement(Compiler c, Scanner s, SymTable st, Token tok, Context ctx);

#endif