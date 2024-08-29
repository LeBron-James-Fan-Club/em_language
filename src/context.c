#include "context.h"

#include "sym.h"


Context Context_New(void) { return calloc(1, sizeof(struct context)); }

void Context_Free(Context this) { free(this); }

void Context_SetFunctionId(Context this, SymTableEntry sym) { this->functionId = sym; }

SymTableEntry Context_GetFunctionId(Context this) { return this->functionId; }
