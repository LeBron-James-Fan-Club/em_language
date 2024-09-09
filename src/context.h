#ifndef CONTEXT_H
#define CONTEXT_H

#include "sym.h"

typedef struct context *Context;

struct context {
    SymTableEntry functionId{};
    int loopLevel{};
    int switchLevel{};
};

void Context_SetFunctionId(Context self, SymTableEntry sym);
SymTableEntry Context_GetFunctionId(Context self);

void Context_IncLoopLevel(Context self);
void Context_DecLoopLevel(Context self);
int Context_GetLoopLevel(Context self);
void Context_ResetLoopLevel(Context self);

void Context_IncSwitchLevel(Context self);
void Context_DecSwitchLevel(Context self);
int Context_GetSwitchLevel(Context self);
void Context_ResetSwitchLevel(Context self);

#endif