#include "sym.h"

SymTable SymTable_New(void) {
    SymTable g = calloc(1, sizeof(struct symTable));
    if (g == NULL) {
        fprintf(stderr, "Error: Unable to initialise global table\n");
        exit(-1);
    }
    return g;
}

void SymTable_Free(SymTable this) {
    free(this);
}

int SymTable_GlobFind(SymTable this, Scanner s) {
    for (int i = 0; i < this->globs; i++) {
        if (*s->text == *this->Gsym[i].name && !strcmp(s->text, this->Gsym[i].name))
            return i;
    }
    return -1;
}

int SymTable_GlobAdd(SymTable this, Scanner s) {
    int y;
    if ((y = SymTable_GlobFind(this, s)) != -1) return y;
    
    if ((y = this->globs++) >= MAX_SYMBOLS) {
        fprintf(stderr, "Error: Exceeded max symbols %d\n", MAX_SYMBOLS);
        exit(-1);
    }

    this->Gsym[y].name = strdup(s->text);
    return y;
}

int SymTable_LabelFind(SymTable this, Scanner s) {
    for (int i = 0; i < this->labels; i++) {
        if (*s->text == *this->Lsym[i].name && !strcmp(s->text, this->Lsym[i].name))
            return i;
    }
    // If it doesnt exist just add it anyways lmao
    int y;

    if ((y = this->labels++) >= MAX_SYMBOLS) {
        fprintf(stderr, "Error: Exceeded max symbols %d\n", MAX_SYMBOLS);
        exit(-1);
    }
    this->Lsym[y].name = strdup(s->text);
    return y;
}

int SymTable_LabelAdd(SymTable this, Scanner s) {
    int y;
    if ((y = SymTable_LabelFind(this, s)) != -1) return y;
    
    if ((y = this->labels++) >= MAX_SYMBOLS) {
        fprintf(stderr, "Error: Exceeded max symbols %d\n", MAX_SYMBOLS);
        exit(-1);
    }

    this->Lsym[y].name = strdup(s->text);
    return y;
}