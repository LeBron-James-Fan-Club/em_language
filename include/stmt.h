#ifndef STMT_H
#define STMT_H

#include "decl.h"
#include "defs.h"
#include "expr.h"
#include "gen.h"
#include "scan.h"
#include "sym.h"
#include "types.h"

ASTnode Compound_Statement(Compiler c, Scanner s, SymTable st, Token tok,
                           Context ctx, bool inSwitch);

#endif