#include "context.h"

Context Context_New(void) { return calloc(1, sizeof(struct context)); }

void Context_Free(Context this) { free(this); }

void Context_SetFunctionId(Context this, int id) { this->functionId = id; }

int Context_GetFunctionId(Context this) { return this->functionId; }
