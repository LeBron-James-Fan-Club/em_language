#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdlib.h>
#include "sym.h"

struct context {
    SymTableEntry functionId;
};

typedef struct context *Context;

Context Context_New(void);

void Context_Free(Context this);

void Context_SetFunctionId(Context this, int id);
int Context_GetFunctionId(Context this);

#endif