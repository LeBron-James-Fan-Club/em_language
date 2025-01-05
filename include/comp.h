#ifndef COMP_H
#define COMP_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "defs.h"

// Actually 10
#define MAX_REG 14

#define NO_REG -1

#define FIRST_PARAM_REG 10

#define TEMP_MAX_REG 10


struct compiler {
    bool regUsed[MAX_REG];

    bool styleRegUsed[MAX_REG];
    int styleSeek;

    bool sawSwitch;

    int paramRegCount;
    int spillReg;

    FILE *outfile;
    int label;
    

    int localOffset;
    int paramOffset;
};

// forward declaration
int PrimSize(enum ASTPRIM type);

typedef struct compiler *Compiler;

Compiler Compiler_New(char *outfile);
void Compiler_Free(Compiler);

void Compiler_ResetOffset(Compiler this);
int Compiler_GetLocalOffset(Compiler this, int size);
int Compiler_GetParamOffset(Compiler this, enum ASTPRIM type);

#endif