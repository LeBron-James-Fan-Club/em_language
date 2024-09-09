#ifndef GEN_H
#define GEN_H

#include "ast.h"
#include "sym.h"
#include "comp.h"
#include "asm.h"

int Compiler_Gen(Compiler self, SymTable st, Context ctx, ASTnode n);

#endif