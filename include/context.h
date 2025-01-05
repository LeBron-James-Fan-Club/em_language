#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdlib.h>
#include "sym.h"

typedef struct context *Context;

struct context {
    SymTableEntry functionId;
    int loopLevel;
    int switchLevel;
};

Context Context_New(void);

void Context_Free(Context this);

void Context_SetFunctionId(Context this, SymTableEntry sym);
SymTableEntry Context_GetFunctionId(Context this);

void Context_IncLoopLevel(Context this);
void Context_DecLoopLevel(Context this);
int Context_GetLoopLevel(Context this);
void Context_ResetLoopLevel(Context this);

void Context_IncSwitchLevel(Context this);
void Context_DecSwitchLevel(Context this);
int Context_GetSwitchLevel(Context this);
void Context_ResetSwitchLevel(Context this);

#endif