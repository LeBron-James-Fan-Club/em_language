// Own file because it might be used later for other vars - e.g. local
#ifndef SYM_H
#define SYM_H

#include <stdbool.h>

#define _GNU_SOURCE
#include <stdio.h>

#include "defs.h"
#include "scan.h"


// forward declaration
typedef struct context *Context;

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
SymTableEntry SymTable_AddGlob(SymTable this, Scanner s, enum ASTPRIM type,
                      SymTableEntry ctype, enum STRUCTTYPE stype, int size);
SymTableEntry SymTable_AddLocl(SymTable this, Scanner s, enum ASTPRIM type,
                      SymTableEntry ctype, enum STRUCTTYPE stype, int size);
SymTableEntry SymTable_AddParam(SymTable this, Scanner s, enum ASTPRIM type,
                       SymTableEntry ctype, enum STRUCTTYPE stype, int size);
SymTableEntry SymTable_AddMemb(SymTable this, Scanner s, enum ASTPRIM type,
                      SymTableEntry ctype, enum STRUCTTYPE stype, int size);
SymTableEntry SymTable_AddStruct(SymTable this, Scanner s, enum ASTPRIM type,
                        SymTableEntry ctype, enum STRUCTTYPE stype, int size);

SymTableEntry SymTable_FindGlob(SymTable this, Scanner s);
SymTableEntry SymTable_FindLocl(SymTable this, Scanner s);
SymTableEntry SymTable_FindMember(SymTable this, Scanner s);
SymTableEntry SymTable_FindStruct(SymTable this, Scanner s);
SymTableEntry SymTable_FindSymInList(Scanner s, SymTableEntry head);
SymTableEntry SymTable_FindSymbol(SymTable this, Scanner s, Context c);
void SymTable_SetValue(SymTable this, SymTableEntry e, int intvalue);
void SymTable_SetText(SymTable this, Scanner s, SymTableEntry e);
void SymTable_FreeLocls(SymTable this);

#endif