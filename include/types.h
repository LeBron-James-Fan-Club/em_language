#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#include "defs.h"
#include "asm.h"

bool type_compatible(int *left, int *right, bool onlyright);

enum ASTPRIM pointer_to(enum ASTPRIM type);
enum ASTPRIM value_at(enum ASTPRIM type);

#endif