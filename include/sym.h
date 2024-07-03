// Own file because it might be used later for other vars - e.g. local
#ifndef SYM_H
#define SYM_H

#include "scan.h"

#define MAX_SYMBOLS 1024

typedef struct symTableEntry {
    char *name;
} SymTableEntry;

struct symTable {
    SymTableEntry Gsym[MAX_SYMBOLS];
     SymTableEntry Lsym[MAX_SYMBOLS];
    int labels;
    int globs;
};

typedef struct symTable *SymTable;

SymTable SymTable_New(void);
void SymTable_Free(SymTable);
int SymTable_GlobFind(SymTable, Scanner s);
int SymTable_GlobAdd(SymTable, Scanner s);

int SymTable_LabelFind(SymTable, Scanner s);
int SymTable_LabelAdd(SymTable, Scanner s);
// For globals

#endif