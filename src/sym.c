#include "sym.h"

SymTable SymTable_New(void) {
    SymTable g = calloc(1, sizeof(struct symTable));
    if (g == NULL) {
        fprintf(stderr, "Error: Unable to initialise global table\n");
        exit(-1);
    }
    return g;
}

void SymTable_Free(SymTable this) { free(this); }

int SymTable_GlobFind(SymTable this, Scanner s, enum STRUCTTYPE stype) {
    for (int i = 0; i < this->globs; i++) {
        if (*s->text == *this->Gsym[i].name &&
            !strcmp(s->text, this->Gsym[i].name)) {
            if (stype != this->Gsym[i].stype) {
                fprintf(stderr,
                        "Error: Variable %s already declared and wrong "
                        "structure type %d\n",
                        s->text, stype);
                exit(-1);
            }
            return i;
        }
    }
    // If it doesnt exist just add it anyways lmao
    if (stype == S_LABEL) {
        int y;

        if ((y = this->globs++) >= MAX_SYMBOLS) {
            fprintf(stderr, "Error: Exceeded max symbols %d\n", MAX_SYMBOLS);
            exit(-1);
        }
        this->Gsym[y].name = strdup(s->text);
        return y;
    }
    return -1;
}

int SymTable_GlobAdd(SymTable this, Scanner s, enum ASTPRIM type,
                     enum STRUCTTYPE stype) {
    int y;
    if ((y = SymTable_GlobFind(this, s, stype)) != -1) {
        if (type != this->Gsym[y].type) {
            fprintf(
                stderr,
                "Error: Variable %s already declared and wrong type %d != %d\n",
                s->text, type, this->Gsym[y].type);
            exit(-1);
        }
        //printf("Found %d\n", type);
        return y;
    }

    if ((y = this->globs++) >= MAX_SYMBOLS) {
        fprintf(stderr, "Error: Exceeded max symbols %d\n", MAX_SYMBOLS);
        exit(-1);
    }

    printf("Adding %s %d\n", s->text, type);
    this->Gsym[y].name = strdup(s->text);
    this->Gsym[y].type = type;
    printf("Added %s %d\n", s->text, type);
    this->Gsym[y].stype = stype;

    return y;
}