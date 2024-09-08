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
                                      enum STORECLASS class, int size,
                                      int offset);
static void pushSym(SymTableEntry *head, SymTableEntry *tail, SymTableEntry e);

static SymTableEntry SymTableEntryNew(char *name, enum ASTPRIM type,
                                      SymTableEntry ctype,
                                      enum STRUCTTYPE stype,
                                      enum STORECLASS class, int size,
                                      int posn) {
    SymTableEntry e = calloc(1, sizeof(struct symTableEntry));
    if (e == NULL) {
        fatal("InternalError: Unable to allocate memory for SymTableEntry");
    }

    e->name = strdup(name);
    e->type = type;
    e->ctype = ctype;

    e->stype = stype;
    e->class = class;

    e->size = size;
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
                               enum STORECLASS class, int nelems, int posn) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, class, nelems, posn);

    pushSym(&this->globHead, &this->globTail, e);

    return e;
}

SymTableEntry SymTable_AddLocl(SymTable this, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int nelems) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_LOCAL, nelems, 0);

    pushSym(&this->loclHead, &this->loclTail, e);
    return e;
}

SymTableEntry SymTable_AddParam(SymTable this, char *name, enum ASTPRIM type,
                                SymTableEntry ctype, enum STRUCTTYPE stype) {
    SymTableEntry e = SymTableEntryNew(name, type, ctype, stype, C_PARAM, 1, 0);

    pushSym(&this->paramHead, &this->paramTail, e);
    return e;
}

SymTableEntry SymTable_AddMemb(SymTable this, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int nelems) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_MEMBER, nelems, 0);

    pushSym(&this->membHead, &this->membTail, e);
    return e;
}

SymTableEntry SymTable_AddStruct(SymTable this, char *name) {
    SymTableEntry e =
        SymTableEntryNew(name, P_STRUCT, NULL, S_VAR, C_STRUCT, 0, 0);

    pushSym(&this->structHead, &this->structTail, e);
    return e;
}

SymTableEntry SymTable_AddUnion(SymTable this, char *name) {
    SymTableEntry e =
        SymTableEntryNew(name, P_UNION, NULL, S_VAR, C_UNION, 0, 0);

    pushSym(&this->unionHead, &this->unionTail, e);
    return e;
}

SymTableEntry SymTable_AddEnum(SymTable this, char *name, enum STORECLASS class,
                               int value) {
    SymTableEntry e =
        SymTableEntryNew(name, P_INT, NULL, S_VAR, class, 0, value);

    pushSym(&this->enumHead, &this->enumTail, e);
    return e;
}

SymTableEntry SymTable_AddTypeDef(SymTable this, char *name, enum ASTPRIM type,
                                  SymTableEntry ctype) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, S_VAR, C_TYPEDEF, 0, 0);

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

void SymTable_SetText(SymTable this, Scanner s, SymTableEntry e) {
    if (e->strValue) {
        free(e->strValue);
    }
    e->size = strlen(s->text) + 1;
    e->isStr = true;
    e->strValue = strdup(s->text);
}