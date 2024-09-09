#include "context.h"

#include "sym.h"


Context Context_New(void) { return new context; }

void Context_Free(Context self) { free(self); }

void Context_SetFunctionId(Context self, SymTableEntry sym) { self->functionId = sym; }

SymTableEntry Context_GetFunctionId(Context self) { return self->functionId; }

void Context_IncLoopLevel(Context self) { self->loopLevel++; }

void Context_DecLoopLevel(Context self) { self->loopLevel--; }

int Context_GetLoopLevel(Context self) { return self->loopLevel; }

void Context_ResetLoopLevel(Context self) { self->loopLevel = 0; }

void Context_IncSwitchLevel(Context self) { self->switchLevel++; }

void Context_DecSwitchLevel(Context self) { self->switchLevel--; }

int Context_GetSwitchLevel(Context self) { return self->switchLevel; }

void Context_ResetSwitchLevel(Context self) { self->switchLevel = 0; }