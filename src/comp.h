#ifndef COMP_H
#define COMP_H

#include <stdio.h>

#include "defs.h"

// Actually 10
#define MAX_REG 14

#define NO_REG -1

#define FIRST_PARAM_REG 10

#define FREE_REG 4


struct compiler {
    bool regUsed[MAX_REG]{};

    bool styleRegUsed[MAX_REG]{};
    int styleSeek{};

    int paramRegCount{};

    FILE *outfile{};
    int label{};
    

    int localOffset{};
    int paramOffset{};
};

// forward declaration
int PrimSize(enum ASTPRIM type);

typedef struct compiler *Compiler;

Compiler Compiler_New(char *outfile);
void Compiler_Free(Compiler);

void Compiler_ResetOffset(Compiler self);
int Compiler_GetLocalOffset(Compiler self, enum ASTPRIM type);
int Compiler_GetParamOffset(Compiler self, enum ASTPRIM type);

#endif