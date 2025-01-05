#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#include "defs.h"
#include "asm.h"
#include "ast.h"

bool inttype(enum ASTPRIM type);
bool ptrtype(enum ASTPRIM type);
enum ASTPRIM pointer_to(enum ASTPRIM type);
enum ASTPRIM value_at(enum ASTPRIM type);
ASTnode modify_type(ASTnode tree, enum ASTPRIM rtype, SymTableEntry rctype,
                    enum ASTOP op);
int type_size(enum ASTPRIM type, SymTableEntry cType);

#endif