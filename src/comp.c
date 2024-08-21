#include "comp.h"
#include "misc.h"

Compiler Compiler_New(char *outfile) {
    Compiler c = calloc(1, sizeof(struct compiler));
    if (c == NULL) {
        fatal("InternalError: Unable to allocate memory for compiler");
    }

    c->outfile = fopen(outfile, "w");
    if (c->outfile == NULL) {
        fatala("OSError: Unable to open file %s", outfile);
    }

    return c;
}

void Compiler_Free(Compiler this) {
    fclose(this->outfile);
    free(this);
}

void Compiler_ResetOffset(Compiler this) {
    // 8 offset for $ra and $fp
    this->localOffset = 8;
    // I forgot
    this->paramOffset = 0;
}

int Compiler_GetLocalOffset(Compiler this, enum ASTPRIM type) {
    debug("the local offset is %d", this->localOffset);
    this->localOffset += (PrimSize(type) > 4) ? PrimSize(type) : 4;
    debug("after the local offset is %d", this->localOffset);
    return this->localOffset;
}

int Compiler_GetParamOffset(Compiler this, enum ASTPRIM type) {
    debug("the param offset is %d", this->paramOffset);
    this->paramOffset += (PrimSize(type) > 4) ? PrimSize(type) : 4;
    debug("after the param offset is %d", this->paramOffset);
    return this->paramOffset;
}