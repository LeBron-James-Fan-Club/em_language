#include "gen.h"

#include <stdbool.h>

#include "ast.h"
#include "sym.h"
#include "scan.h"

static char *reglist[MAX_REG] = {
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
};

static int allocReg(Compiler this);
static void freeReg(Compiler this, int reg1);
static int label(Compiler this);

static int genAST(Compiler this, int reg, SymTable st, ASTnode n,
                  int parentASTop);
static int genIFAST(Compiler this, SymTable st, ASTnode n);

Compiler Compiler_New(char *outfile) {
    Compiler c = calloc(1, sizeof(struct compiler));
    if (c == NULL) {
        fprintf(stderr, "Error: Unable to initialise compiler\n");
        exit(-1);
    }

    c->outfile = fopen(outfile, "w");
    if (c->outfile == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", outfile);
        exit(-1);
    }

    return c;
}

void Compiler_Free(Compiler this) {
    fclose(this->outfile);
    free(this);
}

void Compiler_GenData(Compiler this, SymTable st) {
    fputs("\n.data\n", this->outfile);
    for (int i = 0; i < st->globs; i++) {
        MIPS_GlobSym(this, st->Gsym[i].name);
    }
}

int Compiler_Gen(Compiler this, SymTable st, ASTnode n) {
    int reg;
    reg = genAST(this, NO_REG, st, n, -1);
    return reg;
}

static int genAST(Compiler this, int reg, SymTable st, ASTnode n,
                  int parentASTop) {
    int leftReg, rightReg;

    // May use stack system to prevent stack overflow

    if (n == NULL) {
        fprintf(stderr, "Error: ASTnode is NULL\n");
        exit(-1);
    }

    switch (n->op) {
        case A_IF:
            return genIFAST(this, st, n);
        case A_GLUE:
            genAST(this, NO_REG, st, n->left, n->op);
            Compiler_FreeAllReg(this);
            genAST(this, NO_REG, st, n->right, n->op);
            Compiler_FreeAllReg(this);
            return NO_REG;
        default:
            // stfu compiler
            break;
    }

    // if this doesnt work swap -1 to reg and vice versa

    if (n->left) {
        leftReg = genAST(this, NO_REG, st, n->left, -1);
    }
    if (n->right) {
        rightReg = genAST(this, leftReg, st, n->right, -1);
    }
    printf("%d\n", n->op);

    switch (n->op) {
        case A_ADD:
            return MIPS_Add(this, leftReg, rightReg);
        case A_SUBTRACT:
            return MIPS_Sub(this, leftReg, rightReg);
        case A_MULTIPLY:
            return MIPS_Mul(this, leftReg, rightReg);
        case A_DIVIDE:
            return MIPS_Div(this, leftReg, rightReg);
        case A_MODULO:
            return MIPS_Mod(this, leftReg, rightReg);
        case A_EQ:
            if (parentASTop == A_IF)
                return MIPS_EqualJump(this, leftReg, rightReg, reg);
            else
                return MIPS_EqualSet(this, leftReg, rightReg);
        case A_NE:
            if (parentASTop == A_IF)
                return MIPS_NotEqualJump(this, leftReg, rightReg, reg);
            else
                return MIPS_NotEqualSet(this, leftReg, rightReg);
        case A_LT:
            if (parentASTop == A_IF)
                return MIPS_LessThanJump(this, leftReg, rightReg, reg);
            else
                return MIPS_LessThanSet(this, leftReg, rightReg);
        case A_GT:
            if (parentASTop == A_IF)
                return MIPS_GreaterThanEqualJump(this, leftReg, rightReg, reg);
            else
                return MIPS_GreaterThanSet(this, leftReg, rightReg);
        case A_LE:
            if (parentASTop == A_IF)
                return MIPS_LessThanEqualJump(this, leftReg, rightReg, reg);
            else
                return MIPS_LessThanEqualSet(this, leftReg, rightReg);
        case A_GE:
            if (parentASTop == A_IF)
                return MIPS_LessThanEqualJump(this, leftReg, rightReg, reg);
            else
                return MIPS_GreaterThanEqualSet(this, leftReg, rightReg);
        case A_INTLIT:
            return MIPS_Load(this, n->v.intvalue);
        case A_IDENT:
            return MIPS_LoadGlob(this, st, n->v.id);
        case A_LVIDENT:
            return MIPS_StoreGlob(this, reg, st, n->v.id);
        case A_ASSIGN:
            return rightReg;
        case A_PRINT:
            MIPS_PrintInt(this, leftReg);
            Compiler_FreeAllReg(this);
            return NO_REG;
        case A_INPUT:
            return MIPS_InputInt(this);
        case A_LABEL:
            MIPS_GotoLabel(this, st, n->v.id);
            return NO_REG;
        case A_GOTO:
            MIPS_GotoJump(this, st, n->v.id);
            return NO_REG;  
        default:
            fprintf(stderr, "Error: Unknown AST operator %d\n", n->op);
            exit(-1);
    }
    // so the compiler will stop complaining
    return 0;
}

static int genIFAST(Compiler this, SymTable st, ASTnode n) {
    int Lfalse, Lend;

    Lfalse = label(this);
    if (n->right) Lend = label(this);

    // reg acts as parameter for label
    genAST(this, Lfalse, st, n->left, n->op);
    Compiler_FreeAllReg(this);

    genAST(this, NO_REG, st, n->mid, n->op);
    Compiler_FreeAllReg(this);

    if (n->right) MIPS_Jump(this, Lend);
    MIPS_Label(this, Lfalse);

    if (n->right) {
        genAST(this, NO_REG, st, n->right, n->op);
        Compiler_FreeAllReg(this);
        MIPS_Label(this, Lend);
    }

    return NO_REG;
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

static int label(Compiler this) { return this->label++; }

void Compiler_FreeAllReg(Compiler this) {
    for (int i = 0; i < MAX_REG; i++) {
        this->regUsed[i] = false;
    }
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
    fputs(
        "\tli\t$a0, '\\n'\n"
        "\tli\t$v0, 11\n"
        "\tsyscall\n",
        this->outfile);
}

int MIPS_LoadGlob(Compiler this, SymTable st, int id) {
    int r = allocReg(this);

    fprintf(this->outfile, "\tlw\t%s, %s\n", reglist[r], st->Gsym[id].name);

    return r;
}

int MIPS_StoreGlob(Compiler this, int r1, SymTable st, int id) {
    // Bug empty register
    if (r1 == -1) {
        fprintf(stderr, "Error: Trying to store an empty register\n");
        exit(-1);
    }

    fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r1],
            st->Gsym[id].name);

    return r1;
}

// Needs to be below .data
void MIPS_GlobSym(Compiler this, char *sym) {
    fprintf(this->outfile, "\t%s:\t.space 4\n", sym);
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
    fprintf(this->outfile, "%s:\n", st->Lsym[id].name);
}

void MIPS_GotoJump(Compiler this, SymTable st, int id) {
    fprintf(this->outfile, "\tb\t%s\n", st->Lsym[id].name);
}