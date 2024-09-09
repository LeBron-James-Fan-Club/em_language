#include "comp.h"
#include "misc.h"

Compiler Compiler_New(char *outfile) {
    Compiler c = new compiler;

    c->outfile = fopen(outfile, "w");
    if (c->outfile == NULL) {
        fatala("OSError: Unable to open file %s", outfile);
    }

    return c;
}

void Compiler_Free(Compiler self) {
    fclose(self->outfile);
    free(self);
}

void Compiler_ResetOffset(Compiler self) {

    self->localOffset = 0;
    // 4 because its like accessing a[0] instead of a[1]
    self->paramOffset = 4;
    self->paramRegCount = 0;
}

int Compiler_GetLocalOffset(Compiler self, enum ASTPRIM type) {
    debug("the local offset is %d", self->localOffset);
    self->localOffset += (PrimSize(type) > 4) ? PrimSize(type) : 4;
    debug("after the local offset is %d", self->localOffset);
    return self->localOffset;
}

int Compiler_GetParamOffset(Compiler self, enum ASTPRIM type) {
    debug("the param offset is %d", self->paramOffset);
    self->paramOffset += (PrimSize(type) > 4) ? PrimSize(type) : 4;
    debug("after the param offset is %d", self->paramOffset);
    return self->paramOffset;
}