// Own file because it might be used later for other vars - e.g. local
#ifndef SYM_H
#define SYM_H

#include <stdbool.h>

#define _GNU_SOURCE
#include <stdio.h>

#include "defs.h"
#include "scan.h"
#include "comp.h"

#define MAX_SYMBOLS 1024

enum STRUCTTYPE { S_VAR, S_FUNC, S_LABEL, S_ARRAY };

typedef struct symTableEntry {

    char *name;
    enum ASTPRIM type;
    enum STRUCTTYPE stype;
    enum STORECLASS class;
    
    int size;  // Number of elements in the symbol
    int offset; // offset from stack

    // for annoymous strings, for now
    char *strValue;
    bool hasValue;
    int value;
} SymTableEntry;

struct symTable {
    SymTableEntry Gsym[MAX_SYMBOLS];
    
    int globs;
    int locls;

    // for annoymous
    int anon;
};

typedef struct symTable *SymTable;

SymTable SymTable_New(void);
void SymTable_Free(SymTable);
int SymTable_Find(SymTable this, Scanner s, enum STRUCTTYPE stype);
int SymTable_Add(SymTable this, Compiler c, Scanner s, enum ASTPRIM type,
                 enum STRUCTTYPE stype, enum STORECLASS class, int size,
                 bool isAnon);
void SymTable_SetValue(SymTable this, int id, int intvalue);
void SymTable_SetText(SymTable this, Scanner s, int id);

#endif