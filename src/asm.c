#include "asm.h"

static char *reglist[MAX_REG] = {
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
};

static int psize[] = {0, 0, 1, 4, 4, 4, 4};

static int allocReg(Compiler this);
static void freeReg(Compiler this, int reg1);

int PrimSize(enum ASTPRIM type) {
    if (type < P_NONE || type > P_INTPTR) {
        printf("what\n");
        fprintf(stderr, "Error: Unknown type %d\n", type);
        exit(-1);
    }
    return psize[type];
}

void MIPS_Pre(Compiler this) {
    fputs(
        ".text\n"
        "\t.globl main\n"
        "\n"
        "main:\n",
        this->outfile);
}

void MIPS_Post(Compiler this) {
    fputs(
        "\tli\t$v0, 10\n"
        "\tsyscall\n",
        this->outfile);
}

void MIPS_PreFunc(Compiler this, SymTable st, int id) {
    fprintf(this->outfile,
            ".text\n"
            "\t.globl %s\n"
            "\n"
            "%s:\n"

            "\tpush\t$fp\n"
            "\tpush\t$ra\n"
            "\tmove\t$fp, $sp\n",
            st->Gsym[id].name, st->Gsym[id].name);
}

void MIPS_PostFunc(Compiler this) {
    fputs(
        "\tpop\t$ra\n"
        "\tpop\t$fp\n"
        "\tjr\t$ra\n",
        this->outfile);
}

int MIPS_Load(Compiler this, int value) {
    int r = allocReg(this);
    fprintf(this->outfile, "\tli %s, %d\n", reglist[r], value);
    return r;
}

int MIPS_Add(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tadd\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_Mul(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tmul\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_Sub(Compiler this, int r1, int r2) {
    // not communative
    fprintf(this->outfile, "\tsub\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_Div(Compiler this, int r1, int r2) {
    // also not communative
    fprintf(this->outfile, "\tdiv\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_Mod(Compiler this, int r1, int r2) {
    // not communative
    fprintf(this->outfile, "\tmod\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r2);
    return r1;
}

void MIPS_PrintInt(Compiler this, int r) {
    fprintf(this->outfile, "\tli\t$v0, 1\n");
    fprintf(this->outfile, "\tmove\t$a0, %s\n", reglist[r]);
    fprintf(this->outfile, "\tsyscall\n");
}

void MIPS_PrintChar(Compiler this, int r) {
    fprintf(this->outfile, "\tli\t$v0, 11\n");
    fprintf(this->outfile, "\tmove\t$a0, %s\n", reglist[r]);
    fprintf(this->outfile, "\tsyscall\n");
}

int MIPS_LoadGlob(Compiler this, SymTable st, int id) {
    int r = allocReg(this);
    switch (st->Gsym[id].type) {
        case P_INT:
            fprintf(this->outfile, "\tlw\t%s, %s\n", reglist[r],
                    st->Gsym[id].name);
            break;
        case P_CHAR:
            fprintf(this->outfile, "\tlbu\t%s, %s\n", reglist[r],
                    st->Gsym[id].name);
            break;
        case P_CHARPTR:
        case P_INTPTR:
            fprintf(this->outfile, "\tlw\t%s, %s\n", reglist[r],
                    st->Gsym[id].name);
            break;
        default:
            fprintf(stderr, "Error!: Unknown type %d\n", st->Gsym[id].type);
            exit(-1);
    }
    return r;
}

int MIPS_StoreGlob(Compiler this, int r1, SymTable st, int id) {
    if (r1 == NO_REG) {
        fprintf(stderr, "Error: Trying to store an empty register\n");
        exit(-1);
    }

    switch (st->Gsym[id].type) {
        case P_INT:
            fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r1],
                    st->Gsym[id].name);
            break;
        case P_CHAR:
            fprintf(this->outfile, "\tsb\t%s, %s\n", reglist[r1],
                    st->Gsym[id].name);
            break;
        case P_CHARPTR:
        case P_INTPTR:
            fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r1],
                    st->Gsym[id].name);
            break;
        default:
            fprintf(stderr, "Error!!: Unknown type %d\n", st->Gsym[id].type);
            exit(-1);
    }

    return r1;
}

int MIPS_StoreRef(Compiler this, int r1, int r2, enum ASTPRIM type) {
    if (r1 == NO_REG) {
        fprintf(stderr, "Error: Trying to store an empty register\n");
        exit(-1);
    }

    if (r2 == NO_REG) {
        fprintf(stderr, "Error: Trying to store an empty register 2\n");
        exit(-1);
    }

    switch (type) {
        case P_CHAR:
            fprintf(this->outfile, "\tsb\t%s, 0(%s)\n", reglist[r1], reglist[r2]);
            break;
        case P_INT:
            fprintf(this->outfile, "\tsw\t%s, 0(%s)\n", reglist[r1], reglist[r2]);
            break;
        default:
            fprintf(stderr, "Error: Unknown pointer type %d\n", type);
            exit(-1);
    }

    return r1;
}

int MIPS_Widen(Compiler this, int r1, enum ASTPRIM newType) {
    // nothing to do, its already done by zero extending
    return r1;
}

// Needs to be below .data
void MIPS_GlobSym(Compiler this, char *sym, enum ASTPRIM type, int size) {
    int typesize = PrimSize(type);
    fprintf(this->outfile, "\t%s:\t.space %d\n", sym, typesize * size);
    switch (type) {
        case P_CHAR:
            fprintf(this->outfile, "\t.align 2\n");
            break;
        default:
        break;
    }
}

int MIPS_InputInt(Compiler this) {
    fputs(
        "\tli\t$v0, 5\n"
        "\tsyscall\n",
        this->outfile);

    // Call store glob later on
    // fprintf(this->outfile, "\tsw\t$v0, %s\n", g->Gsym[id].name);

    int r = allocReg(this);
    fprintf(this->outfile, "\tmove\t%s, $v0\n", reglist[r]);
    return r;
}

int MIPS_EqualSet(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tseq\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

// ALl the jumps are reversed -> eq -> neq

int MIPS_EqualJump(Compiler this, int r1, int r2, int l) {
    fprintf(this->outfile, "\tbne\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(this);
    return NO_REG;
}

int MIPS_NotEqualSet(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tsne\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_NotEqualJump(Compiler this, int r1, int r2, int l) {
    fprintf(this->outfile, "\tbeq\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(this);
    return NO_REG;
}

int MIPS_LessThanSet(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tslt\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_LessThanJump(Compiler this, int r1, int r2, int l) {
    fprintf(this->outfile, "\tbge\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(this);
    return NO_REG;
}

int MIPS_GreaterThanSet(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tsgt\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_GreaterThanJump(Compiler this, int r1, int r2, int l) {
    fprintf(this->outfile, "\tble\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(this);
    return NO_REG;
}

int MIPS_LessThanEqualSet(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tsle\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_LessThanEqualJump(Compiler this, int r1, int r2, int l) {
    fprintf(this->outfile, "\tbgt\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(this);
    return NO_REG;
}

int MIPS_GreaterThanEqualSet(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tsge\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_GreaterThanEqualJump(Compiler this, int r1, int r2, int l) {
    fprintf(this->outfile, "\tblt\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(this);
    return NO_REG;
}

void MIPS_Label(Compiler this, int l) { fprintf(this->outfile, "L%d:\n", l); }

void MIPS_Jump(Compiler this, int l) {
    fprintf(this->outfile, "\tb\tL%d\n", l);
}

void MIPS_GotoLabel(Compiler this, SymTable st, int id) {
    fprintf(this->outfile, "%s:\n", st->Gsym[id].name);
}

void MIPS_GotoJump(Compiler this, SymTable st, int id) {
    fprintf(this->outfile, "\tb\t%s\n", st->Gsym[id].name);
}

void MIPS_Return(Compiler this, SymTable st, int r, int id, Context ctx) {
    if (st->Gsym[id].type == P_INT) {
        fprintf(this->outfile, "\tmove\t$v0, %s\n", reglist[r]);
    } else if (st->Gsym[id].type == P_CHAR) {
        fprintf(this->outfile, "\tmove\t$v0, %s\n", reglist[r]);
        // I dont think we need the below
        // fprintf(this->outfile, "\tandi\t$v0, %s, 0xFF\n", reglist[r]);
    } else {
        fprintf(stderr, "Error!!!: Unknown type %d\n", st->Gsym[id].type);
        exit(-1);
    }
}

int MIPS_Call(Compiler this, SymTable st, int r, int id) {
    int outr = allocReg(this);
    // for future use: params
    fprintf(this->outfile, "\tmove\t$a0, %s\n", reglist[r]);
    fprintf(this->outfile, "\tjal\t%s\n", st->Gsym[id].name);
    fprintf(this->outfile, "\tmove\t%s, $v0\n", reglist[outr]);
    freeReg(this, r);
    return outr;
}

int MIPS_Address(Compiler this, SymTable st, int id) {
    int r = allocReg(this);
    fprintf(this->outfile, "\tla\t%s, %s\n", reglist[r], st->Gsym[id].name);
    return r;
}

int MIPS_Deref(Compiler this, int r, enum ASTPRIM type) {
    //! bug: derefing not occuring for b[3]
    switch (type) {
        case P_CHARPTR:
            fprintf(this->outfile, "\tlbu\t%s, 0(%s)\n", reglist[r], reglist[r]);
            break;
        case P_INTPTR:
            fprintf(this->outfile, "\tlw\t%s, 0(%s)\n", reglist[r], reglist[r]);
            break;
        default:
            fprintf(stderr, "Error: Unknown pointer type %d\n", type);
            exit(-1);
    }
    return r;
}

int MIPS_ShiftLeftConstant(Compiler this, int r, int c) {
    fprintf(this->outfile, "\tsll\t%s, %s, %d\n", reglist[r], reglist[r], c);
    return r;
}

static int allocReg(Compiler this) {
    for (int i = 0; i < MAX_REG; i++) {
        if (!this->regUsed[i]) {
            this->regUsed[i] = true;
            return i;
        }
    }
    fprintf(stderr, "Error: Out of registers\n");
    exit(-1);
}

static void freeReg(Compiler this, int reg1) {
    if (!this->regUsed[reg1]) {
        fprintf(stderr, "Error: Trying to free a free register\n");
    }
    this->regUsed[reg1] = false;
}

int label(Compiler this) { return this->label++; }

void Compiler_FreeAllReg(Compiler this) {
    for (int i = 0; i < MAX_REG; i++) {
        this->regUsed[i] = false;
    }
}

void Compiler_GenData(Compiler this, SymTable st) {
    fputs("\n.data\n", this->outfile);
    for (int i = 0; i < st->globs; i++) {
        if (st->Gsym[i].stype == S_FUNC) continue;
        MIPS_GlobSym(this, st->Gsym[i].name, st->Gsym[i].type, st->Gsym[i].size);
    }
}