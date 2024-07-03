#ifndef STMT_H
#define STMT_H

#include "scan.h"
#include "gen.h"
#include "sym.h"
#include "tokens.h"

ASTnode Compound_Statement(Scanner s, SymTable st, Token tok);

#endif