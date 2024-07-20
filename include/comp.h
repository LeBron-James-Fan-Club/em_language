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

#define FREE_REG 4


struct compiler {
    bool regUsed[MAX_REG];
    FILE *outfile;
    int label;

    int localOffset;
    int stackOffset;
};

typedef struct compiler *Compiler;

Compiler Compiler_New(char *outfile);
void Compiler_Free(Compiler);

void Compiler_ResetLocals(Compiler this);
int Compiler_GetLocalOffset(Compiler this, enum ASTPRIM type);

#endif