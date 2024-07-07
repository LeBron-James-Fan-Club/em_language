#ifndef STMT_H
#define STMT_H

#include "scan.h"
#include "gen.h"
#include "sym.h"
#include "defs.h"
#include "decl.h"

ASTnode Compound_Statement(Scanner s, SymTable st, Token tok, Context ctx);

#endif