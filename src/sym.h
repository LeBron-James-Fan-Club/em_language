// Own file because it might be used later for other vars - e.g. local
#ifndef SYM_H
#define SYM_H

#include "defs.h"
#include "scan.h"

// forward declaration
typedef struct context *Context;
bool ptrtype(enum ASTPRIM type);
bool inttype(enum ASTPRIM type);


#define MAX_SYMBOLS 1024

enum STRUCTTYPE { S_VAR, S_FUNC, S_LABEL, S_ARRAY };

struct symTableEntry {
    char *name{};
    enum ASTPRIM type;
    // pointer if type struct
    struct symTableEntry *ctype{};
    enum STORECLASS _class;
    enum STRUCTTYPE stype;

    union {
        int posn{};
        int paramReg;  // param register if first 4
    };

    // is the first 4 parameters?
    union {
        bool isFirstFour{};
        bool isStr;
    };

    int nElems{};
    int size{};  // Number of elements in the symbol

    // for annoymous strings, for now
    char *strValue{};

    int *initList{};

    struct symTableEntry *next{};

    // holds parameter list
    struct symTableEntry *member{};
};

typedef struct symTableEntry *SymTableEntry;

// another forward decl
int type_size(enum ASTPRIM type, SymTableEntry cType);

struct symTable {
    SymTableEntry globHead{}, globTail{};
    SymTableEntry loclHead{}, loclTail{};
    SymTableEntry paramHead{}, paramTail{};
    SymTableEntry membHead{}, membTail{};
    SymTableEntry structHead{}, structTail{};
    SymTableEntry unionHead{}, unionTail{};
    SymTableEntry enumHead{}, enumTail{};
    SymTableEntry typeHead{}, typeTail{};

    // for annoymous
    int anon{1};

    ~symTable();
};

typedef struct symTable *SymTable;

SymTableEntry SymTable_AddGlob(SymTable self, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               enum STORECLASS _class, int nelems, int posn);
SymTableEntry SymTable_AddLocl(SymTable self, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int nelems);
SymTableEntry SymTable_AddParam(SymTable self, char *name, enum ASTPRIM type,
                                SymTableEntry ctype, enum STRUCTTYPE stype);
SymTableEntry SymTable_AddMemb(SymTable self, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int nelems);
SymTableEntry SymTable_AddStruct(SymTable self, char *name);
SymTableEntry SymTable_AddUnion(SymTable self, char *name);
SymTableEntry SymTable_AddEnum(SymTable self, char *name, enum STORECLASS _class,
                               int value);
SymTableEntry SymTable_AddTypeDef(SymTable self, char *name, enum ASTPRIM type,
                                  SymTableEntry ctype);
char *SymTableEntry_MakeAnon(SymTable self, int *anon);

SymTableEntry SymTable_FindGlob(SymTable self, Scanner s);
SymTableEntry SymTable_FindLocl(SymTable self, Scanner s, Context c);
SymTableEntry SymTable_FindMember(SymTable self, Scanner s);
SymTableEntry SymTable_FindStruct(SymTable self, Scanner s);
SymTableEntry SymTable_FindSymInList(Scanner s, SymTableEntry head,
                                     enum STORECLASS _class);
SymTableEntry SymTable_FindSymbol(SymTable self, Scanner s, Context c);
SymTableEntry SymTable_FindUnion(SymTable self, Scanner s);
SymTableEntry SymTable_FindEnumType(SymTable st, Scanner s);
SymTableEntry SymTable_FindEnumVal(SymTable st, Scanner s);
SymTableEntry SymTable_FindTypeDef(SymTable self, Scanner s);

void SymTable_SetText(SymTable self, Scanner s, SymTableEntry e);
void SymTable_FreeLocls(SymTable self);
void SymTable_FreeParams(SymTable self);

void SymTable_Dump(SymTable self);

#endif