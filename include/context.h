#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdlib.h>
#include "sym.h"

typedef struct context *Context;

struct context {
    SymTableEntry functionId;
    int loopLevel;
};

Context Context_New(void);

void Context_Free(Context this);

void Context_SetFunctionId(Context this, SymTableEntry sym);
SymTableEntry Context_GetFunctionId(Context this);

void Context_IncLoopLevel(Context this);
void Context_DecLoopLevel(Context this);
int Context_GetLoopLevel(Context this);
void Context_ResetLoopLevel(Context this);

#endif