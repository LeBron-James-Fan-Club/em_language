// Own file because it might be used later for other vars - e.g. local
#ifndef SYM_H
#define SYM_H

#include "scan.h"
#include "defs.h"

#include <stdbool.h>

#define MAX_SYMBOLS 1024

enum STRUCTTYPE {
    S_VAR,
    S_FUNC,
    S_LABEL
};

typedef struct symTableEntry {
    char *name;
    enum ASTPRIM type;
    enum STRUCTTYPE stype;
} SymTableEntry;

struct symTable {
    SymTableEntry Gsym[MAX_SYMBOLS];
    int globs;
};

typedef struct symTable *SymTable;

SymTable SymTable_New(void);
void SymTable_Free(SymTable);
int SymTable_GlobFind(SymTable this, Scanner s,
                      enum STRUCTTYPE stype);
int SymTable_GlobAdd(SymTable this, Scanner s, enum ASTPRIM type, enum STRUCTTYPE stype);

#endif