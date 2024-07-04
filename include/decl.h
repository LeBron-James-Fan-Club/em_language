#ifndef DECL_H
#define DECL_H

#include "tokens.h"
#include "scan.h"
#include "sym.h"

void var_declare(Scanner s, SymTable st, Token tok);

#endif