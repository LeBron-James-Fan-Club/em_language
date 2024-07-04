#include "gen.h"

#include <stdbool.h>

#include "ast.h"
#include "scan.h"
#include "sym.h"

static int genAST(Compiler this, int reg, SymTable st, ASTnode n,
                  int parentASTop);
static int genIFAST(Compiler this, SymTable st, ASTnode n);
static int genWHILEAST(Compiler this, SymTable st, ASTnode n);

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
        case A_WHILE:
            return genWHILEAST(this, st, n);
        case A_GLUE:
            genAST(this, NO_REG, st, n->left, n->op);
            Compiler_FreeAllReg(this);
            genAST(this, NO_REG, st, n->right, n->op);
            Compiler_FreeAllReg(this);
            return NO_REG;
        case A_FUNCTION:
            MIPS_PreFunc(this, st, n->v.id);
            genAST(this, NO_REG, st, n->left, n->op);
            MIPS_PostFunc(this);
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
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return MIPS_EqualJump(this, leftReg, rightReg, reg);
            else
                return MIPS_EqualSet(this, leftReg, rightReg);
        case A_NE:
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return MIPS_NotEqualJump(this, leftReg, rightReg, reg);
            else
                return MIPS_NotEqualSet(this, leftReg, rightReg);
        case A_LT:
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return MIPS_LessThanJump(this, leftReg, rightReg, reg);
            else
                return MIPS_LessThanSet(this, leftReg, rightReg);
        case A_GT:
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return MIPS_GreaterThanEqualJump(this, leftReg, rightReg, reg);
            else
                return MIPS_GreaterThanSet(this, leftReg, rightReg);
        case A_LE:
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return MIPS_LessThanEqualJump(this, leftReg, rightReg, reg);
            else
                return MIPS_LessThanEqualSet(this, leftReg, rightReg);
        case A_GE:
            if (parentASTop == A_IF || parentASTop == A_WHILE)
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

static int genWHILEAST(Compiler this, SymTable st, ASTnode n) {
    int Lstart, Lend;

    Lstart = label(this);
    Lend = label(this);

    MIPS_Label(this, Lstart);

    // reg acts as parameter for label
    genAST(this, Lend, st, n->left, n->op);
    Compiler_FreeAllReg(this);

    genAST(this, NO_REG, st, n->right, n->op);
    Compiler_FreeAllReg(this);

    MIPS_Jump(this, Lstart);
    MIPS_Label(this, Lend);

    return NO_REG;
}
