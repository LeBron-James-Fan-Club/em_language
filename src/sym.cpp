#include "sym.h"

// TODO: cross-compatability for windows
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
    SymTableEntry e = new symTableEntry;

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
    if (head == nullptr || tail == nullptr || e == nullptr) {
        fatal("InternalError: head, tail or e is nullptr");
    }

    if (*tail != nullptr) {
        (*tail)->next = e;
        *tail = e;
    } else {
        *head = *tail = e;
    }

    e->next = nullptr;
}

char *symTable::SymTableEntry_MakeAnon(int *anon) {
    if (anon != nullptr) {
        *anon = this->anon;
    }

    char *name;
    asprintf(&name, "anon_%d", this->anon++);
    return name;
}

SymTableEntry symTable::SymTable_AddGlob(char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               enum STORECLASS _class, int nelems, int posn) {
    debug("Adding global symbol %s %d", name, nelems);
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, _class, nelems, posn);
    debug("Added global symbol %s %d", name, nelems);

    pushSym(&this->globHead, &this->globTail, e);

    return e;
}

SymTableEntry symTable::SymTable_AddLocl(char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int nelems) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_LOCAL, nelems, 0);

    pushSym(&this->loclHead, &this->loclTail, e);
    return e;
}

SymTableEntry symTable::SymTable_AddParam(char *name, enum ASTPRIM type,
                                SymTableEntry ctype, enum STRUCTTYPE stype) {
    SymTableEntry e = SymTableEntryNew(name, type, ctype, stype, C_PARAM, 1, 0);

    pushSym(&this->paramHead, &this->paramTail, e);
    return e;
}

SymTableEntry symTable::SymTable_AddMemb(char *name, enum ASTPRIM type,
                               SymTableEntry ctype, enum STRUCTTYPE stype,
                               int nelems) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, stype, C_MEMBER, nelems, 0);

    pushSym(&this->membHead, &this->membTail, e);
    return e;
}

SymTableEntry symTable::SymTable_AddStruct(char *name) {
    SymTableEntry e =
        SymTableEntryNew(name, P_STRUCT, nullptr, S_VAR, C_STRUCT, 0, 0);

    pushSym(&this->structHead, &this->structTail, e);
    return e;
}

SymTableEntry symTable::SymTable_AddUnion(char *name) {
    SymTableEntry e =
        SymTableEntryNew(name, P_UNION, nullptr, S_VAR, C_UNION, 0, 0);

    pushSym(&this->unionHead, &this->unionTail, e);
    return e;
}

SymTableEntry symTable::SymTable_AddEnum(char *name, enum STORECLASS _class,
                               int value) {
    SymTableEntry e =
        SymTableEntryNew(name, P_INT, nullptr, S_VAR, _class, 0, value);

    pushSym(&this->enumHead, &this->enumTail, e);
    return e;
}

SymTableEntry symTable::SymTable_AddTypeDef(char *name, enum ASTPRIM type,
                                  SymTableEntry ctype) {
    SymTableEntry e =
        SymTableEntryNew(name, type, ctype, S_VAR, C_TYPEDEF, 0, 0);

    pushSym(&this->typeHead, &this->typeTail, e);
    return e;
}

SymTableEntry symTable::SymTable_FindSymInList(Scanner s, SymTableEntry head,
                                     enum STORECLASS _class) {
    for (; head != nullptr; head = head->next) {
        if (head->name != nullptr && !strcmp(s->text, head->name) &&
            (_class == C_NONE || head->_class == _class)) {
            return head;
        }
    }
    return nullptr;
}

SymTableEntry symTable::SymTable_FindGlob(Scanner s) {
    return SymTable_FindSymInList(s, this->globHead, C_NONE);
}

SymTableEntry symTable::SymTable_FindLocl(Scanner s, Context c) {
    SymTableEntry e;
    if (c->functionId) {
        e = SymTable_FindSymInList(s, c->functionId->member, C_NONE);
        if (e) return e;
    }
    return SymTable_FindSymInList(s, this->loclHead, C_NONE);
}

SymTableEntry symTable::SymTable_FindMember(Scanner s) {
    return SymTable_FindSymInList(s, this->membHead, C_NONE);
}

SymTableEntry symTable::SymTable_FindStruct(Scanner s) {
    return SymTable_FindSymInList(s, this->structHead, C_NONE);
}

SymTableEntry symTable::SymTable_FindUnion(Scanner s) {
    return SymTable_FindSymInList(s, this->unionHead, C_NONE);
}

SymTableEntry symTable::SymTable_FindEnumType(Scanner s) {
    return SymTable_FindSymInList(s, this->enumHead, C_ENUMTYPE);
}

SymTableEntry symTable::SymTable_FindEnumVal(Scanner s) {
    return SymTable_FindSymInList(s, this->enumHead, C_ENUMVAL);
}

SymTableEntry symTable::SymTable_FindTypeDef(Scanner s) {
    return SymTable_FindSymInList(s, this->typeHead, C_TYPEDEF);
}

SymTableEntry symTable::SymTable_FindSymbol(Scanner s, Context c) {
    SymTableEntry e;
    e = this->SymTable_FindLocl(s, c);
    if (e) return e;

    return this->SymTable_FindGlob(s);
}

static void freeList(SymTableEntry head) {
    SymTableEntry tmp;
    while (head != nullptr) {
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
        delete tmp;
    }
}

void symTable::SymTable_FreeParams() {
    freeList(this->paramHead);
    // Its set to null somewhere else
}

void symTable::SymTable_FreeLocls() {
    freeList(this->loclHead);
    this->loclHead = this->loclTail = nullptr;
    // freeList(this->paramHead);
    this->paramHead = this->paramTail = nullptr;
}

void symTable::SymTable_SetText(Scanner s, SymTableEntry e) {
    if (e->strValue) {
        free(e->strValue);
    }
    e->size = strlen(s->text) + 1;
    e->isStr = true;
    e->strValue = strdup(s->text);
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
            if (sym->ctype != nullptr) {
                printf("struct %s ", sym->ctype->name);
            } else {
                printf("struct %s ", sym->name);
            }
            break;
        case P_UNION:
            if (sym->ctype != nullptr) {
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
            dumpTable(sym->member, nullptr, indent + 4);
            break;
    }

    switch (sym->stype) {
        case S_FUNC:
            dumpTable(sym->member, nullptr, indent + 4);

            break;
    }
}

static void dumpTable(SymTableEntry head, char *name, int indent) {
    if (head != nullptr && name != nullptr) {
        printf("%s\n-------------\n", name);
    }

    for (SymTableEntry sym = head; sym != nullptr; sym = sym->next) {
        dumpSym(sym, indent);
    }
}

void symTable::SymTable_Dump() {
    dumpTable(this->globHead, "Globals", 0);
    printf("\n");
    dumpTable(this->enumHead, "Enums", 0);
    printf("\n");
    dumpTable(this->typeHead, "Typedefs", 0);
}

symTable::~symTable() {
    freeList(this->globHead);
    freeList(this->loclHead);
    freeList(this->paramHead);
    freeList(this->membHead);
    freeList(this->structHead);
    freeList(this->unionHead);
    freeList(this->enumHead);
    freeList(this->typeHead);
}
