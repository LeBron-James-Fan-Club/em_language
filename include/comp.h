#ifndef COMP_H
#define COMP_H

#include <stdio.h>

#include "sym.h"

// Actually 10
#define MAX_REG 10

#define NO_REG -1

struct compiler {
    bool regUsed[MAX_REG];
    FILE *outfile;
    int label;
};

typedef struct compiler *Compiler;

Compiler Compiler_New(char *outfile);
void Compiler_Free(Compiler);

#endif