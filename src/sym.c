#define _GNU_SOURCE

#include "sym.h"

// TODO: cross-compatability for windows
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "defs.h"
#include "misc.h"
#include "scan.h"

static void freeList(SymTableEntry head);

static SymTableEntry SymTableEntryNew(char *name, enum ASTPRIM type,
                                      SymTableEntry ctype,
                                      enum STRUCTTYPE stype,
                                      enum STORECLASS class, ArrayDim dims,
                                      int nElems, int posn);
static void pushSym(SymTableEntry *head, SymTableEntry *tail, SymTableEntry e);
static void dumpSym(SymTableEntry sym, int indent);
static void dumpTable(SymTableEntry head, char *name, int indent);

static SymTableEntry SymTableEntryNew(char *name, enum ASTPRIM type,
                                      SymTableEntry ctype,
                                      enum STRUCTTYPE stype,
                                      enum STORECLASS class, ArrayDim dims,
                                      int nElems, int posn) {
    SymTableEntry e = calloc(1, sizeof(struct symTableEntry));
    if (e == NULL) {
        fatal("InternalError: Unable to allocate memory for SymTableEntry");
    }

    debug("Adding symbol %s, nElems %d", name, nElems);

    e->name = strdup(name);
    e->type = type;
    e->ctype = ctype;

    e->stype = stype;
    e->class = class;

    e->dims = dims;
    e->nElems = nElems;

    if (ptrtype(type) || inttype(type)) {
        e->size = nElems * type_size(type, ctype);
    }

    debug("nElems %d, size %d", e->nElems, e->size);
    e->posn = posn;

    return e;
}

static void pushSym(SymTableEntry *head, SymTableEntry *tail, SymTableEntry e) {
    if (head == NULL || tail == NULL || e == NULL) {
        fatal("InternalError: head, tail or e is NULL");
    }

    if (*tail != NULL) {
        (*tail)->next = e;
        *tail = e;
    } else {
        *head = *tail = e;
    }

    e->next = NULL;
}

char *SymTableEntry_MakeAnon(SymTable this, int *anon) {
    if (anon != NULL) {
        *anon = this->anon;
    }

    char *name;
    asprintf(&name, "anon_%d", this->anon++);
    return name;
}

SymTableEntry SymTable_AddGlob(SymTable this, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               enum STORECLASS class, ArrayDim dims, int nelems,
                               int posn) {
    debug("Adding global symbol %s %d", name, nelems);
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, class, dims, nelems, posn);
    debug("Added global symbol %s %d", name, nelems);

    pushSym(&this->globHead, &this->globTail, e);

    return e;
}

SymTableEntry SymTable_AddLocl(SymTable this, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               ArrayDim dims, int nelems) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_LOCAL, dims, nelems, 0);

    pushSym(&this->loclHead, &this->loclTail, e);
    return e;
}

SymTableEntry SymTable_AddParam(SymTable this, char *name, enum ASTPRIM type,
                                SymTableEntry ctype, enum STRUCTTYPE stype,
                                ArrayDim dims) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_PARAM, dims, 1, 0);

    pushSym(&this->paramHead, &this->paramTail, e);
    return e;
}

SymTableEntry SymTable_AddMemb(SymTable this, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               ArrayDim dims, int nelems) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_MEMBER, dims, nelems, 0);

    pushSym(&this->membHead, &this->membTail, e);
    return e;
}

SymTableEntry SymTable_AddStruct(SymTable this, char *name) {
    SymTableEntry e =
        SymTableEntryNew(name, P_STRUCT, NULL, S_VAR, C_STRUCT, NULL, 0, 0);

    pushSym(&this->structHead, &this->structTail, e);
    return e;
}

SymTableEntry SymTable_AddUnion(SymTable this, char *name) {
    SymTableEntry e =
        SymTableEntryNew(name, P_UNION, NULL, S_VAR, C_UNION, NULL, 0, 0);

    pushSym(&this->unionHead, &this->unionTail, e);
    return e;
}

SymTableEntry SymTable_AddEnum(SymTable this, char *name, enum STORECLASS class,
                               int value) {
    SymTableEntry e =
        SymTableEntryNew(name, P_INT, NULL, S_VAR, class, NULL, 0, value);

    pushSym(&this->enumHead, &this->enumTail, e);
    return e;
}

SymTableEntry SymTable_AddTypeDef(SymTable this, char *name, enum ASTPRIM type,
                                  SymTableEntry ctype) {
    // TODO: check if we need dims for this later
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, S_VAR, C_TYPEDEF, NULL, 0, 0);

    pushSym(&this->typeHead, &this->typeTail, e);
    return e;
}

SymTableEntry SymTable_FindSymInList(Scanner s, SymTableEntry head,
                                     enum STORECLASS class) {
    for (; head != NULL; head = head->next) {
        if (head->name != NULL && !strcmp(s->text, head->name) &&
            (class == C_NONE || head->class == class)) {
            return head;
        }
    }
    return NULL;
}

SymTableEntry SymTable_FindGlob(SymTable this, Scanner s) {
    return SymTable_FindSymInList(s, this->globHead, C_NONE);
}

SymTableEntry SymTable_FindLocl(SymTable this, Scanner s, Context c) {
    SymTableEntry e;
    if (c->functionId) {
        e = SymTable_FindSymInList(s, c->functionId->member, C_NONE);
        if (e) return e;
    }
    return SymTable_FindSymInList(s, this->loclHead, C_NONE);
}

SymTableEntry SymTable_FindMember(SymTable this, Scanner s) {
    return SymTable_FindSymInList(s, this->membHead, C_NONE);
}

SymTableEntry SymTable_FindStruct(SymTable this, Scanner s) {
    return SymTable_FindSymInList(s, this->structHead, C_NONE);
}

SymTableEntry SymTable_FindUnion(SymTable this, Scanner s) {
    return SymTable_FindSymInList(s, this->unionHead, C_NONE);
}

SymTableEntry SymTable_FindEnumType(SymTable st, Scanner s) {
    return SymTable_FindSymInList(s, st->enumHead, C_ENUMTYPE);
}

SymTableEntry SymTable_FindEnumVal(SymTable st, Scanner s) {
    return SymTable_FindSymInList(s, st->enumHead, C_ENUMVAL);
}

SymTableEntry SymTable_FindTypeDef(SymTable this, Scanner s) {
    return SymTable_FindSymInList(s, this->typeHead, C_TYPEDEF);
}

SymTableEntry SymTable_FindSymbol(SymTable this, Scanner s, Context c) {
    SymTableEntry e;
    e = SymTable_FindLocl(this, s, c);
    if (e) return e;

    return SymTable_FindGlob(this, s);
}

SymTable SymTable_New(void) {
    SymTable g = calloc(1, sizeof(struct symTable));
    if (g == NULL) {
        fatal("InternalError: Unable to allocate memory for SymTable");
    }
    g->anon = 1;

    return g;
}

static void freeList(SymTableEntry head) {
    SymTableEntry tmp;
    while (head != NULL) {
        tmp = head;
        if (head->name) {
            free(head->name);
        }
        if (head->member) {
            freeList(head->member);
        }
        if (head->strValue) {
            free(head->strValue);
        }
        if (head->initList) {
            free(head->initList);
        }
        if (head->dims) {
            for (ArrayDim dims = head->dims; dims != NULL;) {
                ArrayDim next = dims->next;
                free(dims);
                dims = next;
            }
        }
        head = head->next;
        free(tmp);
    }
}

void SymTable_Free(SymTable this) {
    freeList(this->globHead);
    freeList(this->loclHead);
    freeList(this->paramHead);
    freeList(this->membHead);
    freeList(this->structHead);
    freeList(this->unionHead);
    freeList(this->enumHead);
    freeList(this->typeHead);

    free(this);
}

void SymTable_FreeParams(SymTable this) {
    freeList(this->paramHead);
    // Its set to null somewhere else
}

void SymTable_FreeLocls(SymTable this) {
    freeList(this->loclHead);
    this->loclHead = this->loclTail = NULL;
    // freeList(this->paramHead);
    this->paramHead = this->paramTail = NULL;
}

void SymTable_SetText(SymTable this, char *text, SymTableEntry e) {
    if (e->strValue) {
        free(e->strValue);
    }
    e->size = strlen(text) + 1;
    e->isStr = true;
    e->strValue = strdup(text);
}

static void dumpSym(SymTableEntry sym, int indent) {
    for (int i = 0; i < indent; i++) printf(" ");

    switch (sym->type & (~0xf)) {
        case P_VOID:
            printf("void ");
            break;
        case P_CHAR:
            printf("i8 ");
            break;
        case P_INT:
            printf("i32 ");
            break;
        case P_STRUCT:
            if (sym->ctype != NULL) {
                printf("struct %s ", sym->ctype->name);
            } else {
                printf("struct %s ", sym->name);
            }
            break;
        case P_UNION:
            if (sym->ctype != NULL) {
                printf("union %s ", sym->ctype->name);
            } else {
                printf("union %s ", sym->name);
            }
            break;
        default:
            printf("unknown type ");
            break;
    }
    for (int i = 0; i < (sym->type & 0xf); i++) printf("*");
    printf("%s", sym->name);

    switch (sym->stype) {
        case S_VAR:
            break;
        case S_FUNC:
            printf("()");
            break;
        case S_LABEL:
            printf("(label)");
            break;
        case S_ARRAY:
            printf("[]");
            break;
        default:
            printf("(unknown)");
            break;
    }

    switch (sym->class) {
        case C_GLOBAL:
            printf(": global");
            break;
        case C_EXTERN:
            printf(": extern");
            break;
        case C_STATIC:
            printf(": static");
            break;
        case C_LOCAL:
            printf(": local");
            break;
        case C_PARAM:
            printf(": param");
            break;
        case C_STRUCT:
            printf(": struct");
            break;
        case C_UNION:
            printf(": union");
            break;
        case C_ENUMTYPE:
            printf(": enumtype");
            break;
        case C_ENUMVAL:
            printf(": enumval");
            break;
        case C_TYPEDEF:
            printf(": typedef");
            break;
        case C_MEMBER:
            printf(": member");
            break;
        case C_NONE:
            printf(": none");
            break;
        default:
            printf(": unknown");
            break;
    }

    switch (sym->stype) {
        case S_VAR:
            if (sym->class == C_ENUMVAL) {
                printf(", value %d\n", sym->posn);
            } else {
                printf(", size %d\n", sym->size);
            }
            break;
        case S_FUNC:
            printf(", %d params\n", sym->nElems);
            break;
        case S_ARRAY:
            printf(", %d elements, size %d\n", sym->nElems, sym->size);
            break;
    }

    switch (sym->stype & (~0xf)) {
        case P_STRUCT:
        case P_UNION:
            dumpTable(sym->member, NULL, indent + 4);
            break;
    }

    switch (sym->stype) {
        case S_FUNC:
            dumpTable(sym->member, NULL, indent + 4);

            break;
    }
}

static void dumpTable(SymTableEntry head, char *name, int indent) {
    if (head != NULL && name != NULL) {
        printf("%s\n-------------\n", name);
    }

    for (SymTableEntry sym = head; sym != NULL; sym = sym->next) {
        dumpSym(sym, indent);
    }
}

void SymTable_Dump(SymTable this) {
    dumpTable(this->globHead, "Globals", 0);
    printf("\n");
    dumpTable(this->enumHead, "Enums", 0);
    printf("\n");
    dumpTable(this->typeHead, "Typedefs", 0);
}