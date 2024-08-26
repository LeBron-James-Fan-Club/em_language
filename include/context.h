#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdlib.h>
#include "sym.h"

typedef struct context *Context;

struct context {
    SymTableEntry functionId;
};

Context Context_New(void);

void Context_Free(Context this);

void Context_SetFunctionId(Context this, int id);
SymTableEntry Context_GetFunctionId(Context this);

#endif