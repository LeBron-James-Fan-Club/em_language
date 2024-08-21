#include "sym.h"

#include "misc.h"


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
                                      int offset) {
    SymTableEntry e = calloc(1, sizeof(struct symTableEntry));
    if (e == NULL) {
        fatal("InternalError: Unable to allocate memory for SymTableEntry");
    }

    e->name = name;
    e->type = type;
    e->ctype = ctype;

    e->stype = stype;
    e->class = class;

    e->size = size;
    e->offset = offset;

    return e;
}

static void pushSym(SymTableEntry *head, SymTableEntry *tail, SymTableEntry e) {
    if (head == NULL || tail == NULL || e == NULL) {
        fatal("InternalError: head, tail or e is NULL");
    }

    if (tail != NULL) {
        (*tail)->next = e;
        *tail = e;
    } else {
        *head = *tail = e;
    }

    e->next = NULL;
}

SymTableEntry SymTable_addGlob(SymTable this, char *name, enum ASTPRIM type,
                             SymTableEntry ctype, enum STRUCTTYPE stype,
                             int size) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_GLOBAL, size, 0);

    pushSym(&this->globHead, &this->globTail, e);
    return e;
}

SymTableEntry SymTable_addLocl(SymTable this, char *name, enum ASTPRIM type,
                             SymTableEntry ctype, enum STRUCTTYPE stype,
                             int size) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_LOCAL, size, 0);

    pushSym(&this->loclHead, &this->loclTail, e);
    return e;
}

SymTableEntry SymTable_addParam(SymTable this, char *name, enum ASTPRIM type,
                              SymTableEntry ctype, enum STRUCTTYPE stype,
                              int size) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_PARAM, size, 0);

    pushSym(&this->paramHead, &this->paramTail, e);
    return e;
}

SymTableEntry SymTable_addMemb(SymTable this, char *name, enum ASTPRIM type,
                             SymTableEntry ctype, enum STRUCTTYPE stype,
                             int size) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_MEMBER, size, 0);

    pushSym(&this->membHead, &this->membTail, e);
    return e;
}

SymTableEntry SymTable_addStruct(SymTable this, char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int size) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_STRUCT, size, 0);

    pushSym(&this->structHead, &this->structTail, e);
    return e;
}

SymTableEntry SymTable_findSymInList(char *name, SymTableEntry head) {
    for (; head != NULL; head = head->next) {
        if (head->name != NULL && !strcmp(name, head->name)) {
            return head;
        }
    }
    return NULL;
}

SymTableEntry SymTable_findGlob(SymTable this, char *s) {
    return findSymInList(s, this->globHead);
}

SymTableEntry SymTable_findLocl(SymTable this, char *s) {
    return findSymInList(s, this->loclHead);
}

SymTableEntry SymTable_findMember(SymTable this, char *s) {
    return findSymInList(s, this->membHead);
}

SymTableEntry SymTable_findStruct(SymTable this, char *s) {
    return findSymInList(s, this->structHead);
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

    free(this);
}

/*
void SymTable_CopyFuncParams(SymTable this, int slot) {
    int id = slot + 1;
    for (int i = 0; i < this->Gsym[slot].nElems; i++, id++) {
        int newId = SymTable_LoclNew(this);
        debug("Copying %s to %s", this->Gsym[id].name, this->Gsym[newId].name);
        SymTable_Update(this, newId, strdup(this->Gsym[id].name),
                        this->Gsym[id].type, this->Gsym[id].stype, C_LOCAL,
                        this->Gsym[id].size, 0);
    }
}
*/