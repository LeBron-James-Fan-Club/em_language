#include "sym.h"

static int SymTable_GlobNew(SymTable this);
static int SymTable_LoclNew(SymTable this);
static int SymTable_FindSym(SymTable this, Scanner s, enum STRUCTTYPE stype, enum STORECLASS class);

SymTable SymTable_New(void) {
    SymTable g = calloc(1, sizeof(struct symTable));
    if (g == NULL) {
        fprintf(stderr, "Error: Unable to initialise global table\n");
        exit(-1);
    }
    g->locls = MAX_SYMBOLS - 1;
    return g;
}

void SymTable_Free(SymTable this) {
    for (int i = 0; i < this->globs; i++) {
        free(this->Gsym[i].name);
        if (this->Gsym[i].strValue) free(this->Gsym[i].strValue);
    }
    for (int i = this->locls + 1; i < MAX_SYMBOLS; i++) {
        free(this->Gsym[i].name);
        if (this->Gsym[i].strValue) free(this->Gsym[i].strValue);
    }
    free(this);
}

int SymTable_Find(SymTable this, Scanner s, enum STRUCTTYPE stype) {
    int id;
    if ((id = SymTable_FindSym(this, s, stype, C_GLOBAL)) == -1) {
        id = SymTable_FindSym(this, s, stype, C_LOCAL);
    }
    return id;
}

static int SymTable_FindSym(SymTable this, Scanner s, enum STRUCTTYPE stype, enum STORECLASS class) {
    int min = (class == C_GLOBAL) ? 0 : this->locls + 1;
    int max = (class == C_GLOBAL) ? this->globs : MAX_SYMBOLS;
    
    for (int i = min; i < max; i++) {
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

static int SymTable_GlobNew(SymTable this) {
    int id;
    if ((id = this->globs++) >= this->locls) {
        fprintf(stderr, "Error: Exceeded max symbols %d for globals\n",
                MAX_SYMBOLS);
        exit(-1);
    }
    return id;
}

static int SymTable_LoclNew(SymTable this) {
    int id;
    if ((id = this->locls--) <= this->globs) {
        fprintf(stderr, "Error: Exceeded max symbols %d for locals\n",
                MAX_SYMBOLS);
        exit(-1);
    }
    return id;
}

int SymTable_Add(SymTable this, Compiler c, Scanner s, enum ASTPRIM type,
                 enum STRUCTTYPE stype, enum STORECLASS class, int size,
                 bool isAnon) {
    int y;
    if ((y = SymTable_FindSym(this, s, stype, class)) != -1) {
        if (type != this->Gsym[y].type) {
            fprintf(
                stderr,
                "Error: Variable %s already declared and wrong type %d != %d\n",
                s->text, type, this->Gsym[y].type);
            exit(-1);
        }
        return y;
    }

    y = (class == C_GLOBAL) ? SymTable_GlobNew(this) : SymTable_LoclNew(this);

    if (isAnon) {
        // annoymous variable
        asprintf(&this->Gsym[y].name, "anon_%d", this->anon++);
        printf("Anon %s\n", this->Gsym[y].name);
    } else {
        this->Gsym[y].name = strdup(s->text);
    }

    this->Gsym[y].type = type;
    this->Gsym[y].stype = stype;
    this->Gsym[y].size = size;
    this->Gsym[y].class = class;
    if (class == C_LOCAL) {
        this->Gsym[y].offset = Compiler_GetLocalOffset(c, type, false);
        printf("Local offset  222 %d\n", this->Gsym[y].offset);
    }

    return y;
}

void SymTable_SetValue(SymTable this, int id, int intvalue) {
    this->Gsym[id].value = intvalue;
    this->Gsym[id].hasValue = true;
}

// Cant be fucked doing array shit initialisation
// e.g. 2, 3, 4
// Only accepts text rn
void SymTable_SetText(SymTable this, Scanner s, int id) {
    this->Gsym[id].strValue = strdup(s->text);
    this->Gsym[id].hasValue = true;
}