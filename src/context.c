#include "context.h"

#include "sym.h"


Context Context_New(void) { return calloc(1, sizeof(struct context)); }

void Context_Free(Context this) { free(this); }

void Context_SetFunctionId(Context this, SymTableEntry sym) { this->functionId = sym; }

SymTableEntry Context_GetFunctionId(Context this) { return this->functionId; }

void Context_IncLoopLevel(Context this) { this->loopLevel++; }

void Context_DecLoopLevel(Context this) { this->loopLevel--; }

int Context_GetLoopLevel(Context this) { return this->loopLevel; }

void Context_ResetLoopLevel(Context this) { this->loopLevel = 0; }

void Context_IncSwitchLevel(Context this) { this->switchLevel++; }

void Context_DecSwitchLevel(Context this) { this->switchLevel--; }

int Context_GetSwitchLevel(Context this) { return this->switchLevel; }

void Context_ResetSwitchLevel(Context this) { this->switchLevel = 0; }