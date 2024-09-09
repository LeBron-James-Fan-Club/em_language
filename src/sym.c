#include "sym.h"

// TODO: cross-compatability for windows
#define _GNU_SOURCE
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
                                      enum STORECLASS _class, int nElems,
                                      int posn);
static void pushSym(SymTableEntry *head, SymTableEntry *tail, SymTableEntry e);
static void dumpSym(SymTableEntry sym, int indent);
static void dumpTable(SymTableEntry head, char *name, int indent);

static SymTableEntry SymTableEntryNew(char *name, enum ASTPRIM type,
                                      SymTableEntry ctype,
                                      enum STRUCTTYPE stype,
                                      enum STORECLASS _class, int nElems,
                                      int posn) {
    SymTableEntry e = calloc(1, sizeof(struct symTableEntry));
    if (e == NULL) {
        fatal("InternalError: Unable to allocate memory for SymTableEntry");
    }

    debug("Adding symbol %s, nElems %d", name, nElems);

    e->name = strdup(name);
    e->type = type;
    e->ctype = ctype;

    e->stype = stype;
    e->_class = _class;

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

char *SymTableEntry_MakeAnon(SymTable self, int *anon) {
    if (anon != NULL) {
        *anon = self->anon;
    }

    char *name;
    asprintf(&name, "anon_%d", self->anon++);
    return name;
}

SymTableEntry SymTable_AddGlob(SymTable self, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               enum STORECLASS _class, int nelems, int posn) {
    debug("Adding global symbol %s %d", name, nelems);
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, _class, nelems, posn);
    debug("Added global symbol %s %d", name, nelems);

    pushSym(&self->globHead, &self->globTail, e);

    return e;
}

SymTableEntry SymTable_AddLocl(SymTable self, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int nelems) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_LOCAL, nelems, 0);

    pushSym(&self->loclHead, &self->loclTail, e);
    return e;
}

SymTableEntry SymTable_AddParam(SymTable self, char *name, enum ASTPRIM type,
                                SymTableEntry ctype, enum STRUCTTYPE stype) {
    SymTableEntry e = SymTableEntryNew(name, type, ctype, stype, C_PARAM, 1, 0);

    pushSym(&self->paramHead, &self->paramTail, e);
    return e;
}

SymTableEntry SymTable_AddMemb(SymTable self, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int nelems) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_MEMBER, nelems, 0);

    pushSym(&self->membHead, &self->membTail, e);
    return e;
}

SymTableEntry SymTable_AddStruct(SymTable self, char *name) {
    SymTableEntry e =
        SymTableEntryNew(name, P_STRUCT, NULL, S_VAR, C_STRUCT, 0, 0);

    pushSym(&self->structHead, &self->structTail, e);
    return e;
}

SymTableEntry SymTable_AddUnion(SymTable self, char *name) {
    SymTableEntry e =
        SymTableEntryNew(name, P_UNION, NULL, S_VAR, C_UNION, 0, 0);

    pushSym(&self->unionHead, &self->unionTail, e);
    return e;
}

SymTableEntry SymTable_AddEnum(SymTable self, char *name, enum STORECLASS _class,
                               int value) {
    SymTableEntry e =
        SymTableEntryNew(name, P_INT, NULL, S_VAR, _class, 0, value);

    pushSym(&self->enumHead, &self->enumTail, e);
    return e;
}

SymTableEntry SymTable_AddTypeDef(SymTable self, char *name, enum ASTPRIM type,
                                  SymTableEntry ctype) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, S_VAR, C_TYPEDEF, 0, 0);

    pushSym(&self->typeHead, &self->typeTail, e);
    return e;
}

SymTableEntry SymTable_FindSymInList(Scanner s, SymTableEntry head,
                                     enum STORECLASS _class) {
    for (; head != NULL; head = head->next) {
        if (head->name != NULL && !strcmp(s->text, head->name) &&
            (_class == C_NONE || head->_class == _class)) {
            return head;
        }
    }
    return NULL;
}

SymTableEntry SymTable_FindGlob(SymTable self, Scanner s) {
    return SymTable_FindSymInList(s, self->globHead, C_NONE);
}

SymTableEntry SymTable_FindLocl(SymTable self, Scanner s, Context c) {
    SymTableEntry e;
    if (c->functionId) {
        e = SymTable_FindSymInList(s, c->functionId->member, C_NONE);
        if (e) return e;
    }
    return SymTable_FindSymInList(s, self->loclHead, C_NONE);
}

SymTableEntry SymTable_FindMember(SymTable self, Scanner s) {
    return SymTable_FindSymInList(s, self->membHead, C_NONE);
}

SymTableEntry SymTable_FindStruct(SymTable self, Scanner s) {
    return SymTable_FindSymInList(s, self->structHead, C_NONE);
}

SymTableEntry SymTable_FindUnion(SymTable self, Scanner s) {
    return SymTable_FindSymInList(s, self->unionHead, C_NONE);
}

SymTableEntry SymTable_FindEnumType(SymTable st, Scanner s) {
    return SymTable_FindSymInList(s, st->enumHead, C_ENUMTYPE);
}

SymTableEntry SymTable_FindEnumVal(SymTable st, Scanner s) {
    return SymTable_FindSymInList(s, st->enumHead, C_ENUMVAL);
}

SymTableEntry SymTable_FindTypeDef(SymTable self, Scanner s) {
    return SymTable_FindSymInList(s, self->typeHead, C_TYPEDEF);
}

SymTableEntry SymTable_FindSymbol(SymTable self, Scanner s, Context c) {
    SymTableEntry e;
    e = SymTable_FindLocl(self, s, c);
    if (e) return e;

    return SymTable_FindGlob(self, s);
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
        head = head->next;
        free(tmp);
    }
}

void SymTable_Free(SymTable self) {
    freeList(self->globHead);
    freeList(self->loclHead);
    freeList(self->paramHead);
    freeList(self->membHead);
    freeList(self->structHead);
    freeList(self->unionHead);
    freeList(self->enumHead);
    freeList(self->typeHead);

    free(self);
}

void SymTable_FreeParams(SymTable self) {
    freeList(self->paramHead);
    // Its set to null somewhere else
}

void SymTable_FreeLocls(SymTable self) {
    freeList(self->loclHead);
    self->loclHead = self->loclTail = NULL;
    // freeList(self->paramHead);
    self->paramHead = self->paramTail = NULL;
}

void SymTable_SetText(SymTable self, Scanner s, SymTableEntry e) {
    if (e->strValue) {
        free(e->strValue);
    }
    e->size = strlen(s->text) + 1;
    e->isStr = true;
    e->strValue = strdup(s->text);
}

static void dumpSym(SymTableEntry sym, int indent) {
    for (int i = 0; i < ident; i++) printf(" ");

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

    switch (sym->_class) {
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
            if (sym->_class == C_ENUMVAL) {
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

void SymTable_Dump(SymTable self) {
    dumpTable(self->globHead, "Globals", 0);
    printf("\n");
    dumpTable(self->enumHead, "Enums", 0);
    printf("\n");
    dumpTable(self->typeHead, "Typedefs", 0);
}