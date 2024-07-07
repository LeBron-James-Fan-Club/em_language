#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#include "defs.h"
#include "asm.h"

bool type_compatible(int *left, int *right, bool onlyright);

#endif