#ifndef GEN_H
#define GEN_H

#include <stdio.h>

#include "ast.h"
#include "sym.h"
#include "comp.h"
#include "asm.h"


int Compiler_Gen(Compiler, SymTable st, ASTnode n);

#endif