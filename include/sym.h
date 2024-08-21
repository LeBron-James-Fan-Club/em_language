// Own file because it might be used later for other vars - e.g. local
#ifndef SYM_H
#define SYM_H

#include <stdbool.h>

#define _GNU_SOURCE
#include <stdio.h>

#include "defs.h"
#include "scan.h"

#define MAX_SYMBOLS 1024

enum STRUCTTYPE { S_VAR, S_FUNC, S_LABEL, S_ARRAY };

struct symTableEntry {
    char *name;
    enum ASTPRIM type;
    // pointer if type struct
    struct symTableEntry *ctype;
    enum STORECLASS class;
    enum STRUCTTYPE stype;

    union {
        int nElems;  // params
        int offset;  // offset from stack
    };

    int size;  // Number of elements in the symbol

    // for annoymous strings, for now
    char *strValue;
    bool hasValue;
    int value;

    struct symTableEntry *next;

    // holds parameter list
    struct symTableEntry *member;
};

typedef struct symTableEntry *SymTableEntry;

struct symTable {
    SymTableEntry globHead, globTail;
    SymTableEntry loclHead, loclTail;
    SymTableEntry paramHead, paramTail;
    SymTableEntry membHead, membTail;
    SymTableEntry structHead, structTail;

    // for annoymous
    int anon;
};

typedef struct symTable *SymTable;

SymTable SymTable_New(void);
void SymTable_Free(SymTable);
int SymTable_Find(SymTable this, Scanner s, enum STRUCTTYPE stype);
SymTableEntry addGlob(SymTable this, char *name, enum ASTPRIM type,
                      SymTableEntry ctype, enum STRUCTTYPE stype, int size);
SymTableEntry addLocl(SymTable this, char *name, enum ASTPRIM type,
                      SymTableEntry ctype, enum STRUCTTYPE stype, int size);
SymTableEntry addParam(SymTable this, char *name, enum ASTPRIM type,
                       SymTableEntry ctype, enum STRUCTTYPE stype, int size);
SymTableEntry addMemb(SymTable this, char *name, enum ASTPRIM type,
                      SymTableEntry ctype, enum STRUCTTYPE stype, int size);
SymTableEntry addStruct(SymTable this, char *name, enum ASTPRIM type,
                        SymTableEntry ctype, enum STRUCTTYPE stype, int size);
SymTableEntry findSymInList(char *name, SymTableEntry head);
void SymTable_SetValue(SymTable this, int id, int intvalue);
void SymTable_SetText(SymTable this, Scanner s, int id);
void SymTable_ResetLocls(SymTable this);

#endif