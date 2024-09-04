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
        int nElems;    // params
        int offset;    // offset from stack
        int paramReg;  // param register if first 4
    };

    // is the first 4 parameters?
    bool isFirstFour;

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
    SymTableEntry unionHead, unionTail;
    SymTableEntry enumHead, enumTail;
    SymTableEntry typeHead, typeTail;

    // for annoymous
    int anon;
};

typedef struct symTable *SymTable;

SymTable SymTable_New(void);
void SymTable_Free(SymTable);

SymTableEntry SymTable_AddGlob(SymTable this, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               enum STORECLASS class, int size, bool isAnon);
SymTableEntry SymTable_AddLocl(SymTable this, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int size);
SymTableEntry SymTable_AddParam(SymTable this, char *name, enum ASTPRIM type,
                                SymTableEntry ctype, enum STRUCTTYPE stype,
                                int size);
SymTableEntry SymTable_AddMemb(SymTable this, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int size);
SymTableEntry SymTable_AddStruct(SymTable this, char *name, enum ASTPRIM type,
                                 SymTableEntry ctype, enum STRUCTTYPE stype,
                                 int size);
SymTableEntry SymTable_AddUnion(SymTable this, char *name, enum ASTPRIM type,
                                SymTableEntry ctype, enum STRUCTTYPE stype,
                                int size);
SymTableEntry SymTable_AddEnum(SymTable this, char *name, enum STORECLASS class,
                               int value);
SymTableEntry SymTable_AddTypeDef(SymTable this, char *name, enum ASTPRIM type,
                                  SymTableEntry ctype, enum STRUCTTYPE stype,
                                  int size);

SymTableEntry SymTable_FindGlob(SymTable this, Scanner s);
SymTableEntry SymTable_FindLocl(SymTable this, Scanner s, Context c);
SymTableEntry SymTable_FindMember(SymTable this, Scanner s);
SymTableEntry SymTable_FindStruct(SymTable this, Scanner s);
SymTableEntry SymTable_FindSymInList(Scanner s, SymTableEntry head,
                                     enum STORECLASS class);
SymTableEntry SymTable_FindSymbol(SymTable this, Scanner s, Context c);
SymTableEntry SymTable_FindUnion(SymTable this, Scanner s);
SymTableEntry SymTable_FindEnumType(SymTable st, Scanner s);
SymTableEntry SymTable_FindEnumVal(SymTable st, Scanner s);
SymTableEntry SymTable_FindTypeDef(SymTable this, Scanner s);

void SymTable_SetValue(SymTable this, SymTableEntry e, int intvalue);
void SymTable_SetText(SymTable this, Scanner s, SymTableEntry e);
void SymTable_FreeLocls(SymTable this);
void SymTable_FreeParams(SymTable this);

#endif