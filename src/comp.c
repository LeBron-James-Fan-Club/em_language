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

void Compiler_ResetLocals(Compiler this) { this->localOffset = 0; }

int Compiler_GetLocalOffset(Compiler this, enum ASTPRIM type) {
    this->localOffset += (PrimSize(type) > 4) ? PrimSize(type) : 4;
    return this->localOffset;
}