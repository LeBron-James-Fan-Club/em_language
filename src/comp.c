#include "comp.h"

Compiler Compiler_New(char *outfile) {
    Compiler c = calloc(1, sizeof(struct compiler));
    if (c == NULL) {
        fprintf(stderr, "Error: Unable to initialise compiler\n");
        exit(-1);
    }

    c->outfile = fopen(outfile, "w");
    if (c->outfile == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", outfile);
        exit(-1);
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
    printf("the local offset is %d\n", this->localOffset);
    this->localOffset += (PrimSize(type) > 4) ? PrimSize(type) : 4;
    printf("after the local offset is %d\n", this->localOffset);
    return this->localOffset;
}

int Compiler_GetParamOffset(Compiler this, enum ASTPRIM type) {
    printf("the param offset is %d\n", this->paramOffset);
    this->paramOffset += (PrimSize(type) > 4) ? PrimSize(type) : 4;
    printf("after the param offset is %d\n", this->paramOffset);
    return this->paramOffset;
}