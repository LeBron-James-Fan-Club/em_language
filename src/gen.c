#include "gen.h"

#include <stdbool.h>
#include <stdio.h>

#include "ast.h"
#include "misc.h"
#include "scan.h"
#include "sym.h"

#define NO_LABEL -1

static int genAST(Compiler this, SymTable st, int label, Context ctx, ASTnode n,
                  int parentASTop);
static int genIFAST(Compiler this, SymTable st, Context ctx, ASTnode n);
static int genWHILEAST(Compiler this, SymTable st, Context ctx, ASTnode n);
static int genFUNCCALLAST(Compiler this, SymTable st, Context ctx, ASTnode n);

int Compiler_Gen(Compiler this, SymTable st, Context ctx, ASTnode n) {
    int reg;
    reg = genAST(this, st, NO_LABEL, ctx, n, -1);
    return reg;
}

static int genAST(Compiler this, SymTable st, int label, Context ctx, ASTnode n,
                  int parentASTop) {
    int leftReg, rightReg;

    // May use stack system to prevent stack overflow

    if (n == NULL) {
        return NO_REG;
    }

    switch (n->op) {
        case A_IF:
            return genIFAST(this, st, ctx, n);
        case A_WHILE:
            return genWHILEAST(this, st, ctx, n);
        case A_GLUE:
            genAST(this, st, NO_LABEL, ctx, n->left, n->op);
            Compiler_FreeAllReg(this);
            fprintf(this->outfile, "\n");
            genAST(this, st, NO_LABEL, ctx, n->right, n->op);
            Compiler_FreeAllReg(this);
            if (n->right != NULL) fprintf(this->outfile, "\n");
            return NO_REG;
        case A_FUNCTION:
            MIPS_PreFunc(this, st, ctx);
            genAST(this, st, NO_LABEL, ctx, n->left, n->op);
            MIPS_PostFunc(this, st, ctx);
            return NO_REG;
        case A_FUNCCALL:
            return genFUNCCALLAST(this, st, ctx, n);
        default:
            // stfu compiler
            break;
    }

    // Mr compiler could you stfu
    leftReg =
        n->left ? genAST(this, st, NO_LABEL, ctx, n->left, n->op) : NO_REG;
    rightReg =
        n->right ? genAST(this, st, NO_LABEL, ctx, n->right, n->op) : NO_REG;

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
                return MIPS_EqualJump(this, leftReg, rightReg, label);
            else
                return MIPS_EqualSet(this, leftReg, rightReg);
        case A_NE:
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return MIPS_NotEqualJump(this, leftReg, rightReg, label);
            else
                return MIPS_NotEqualSet(this, leftReg, rightReg);
        case A_LT:
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return MIPS_LessThanJump(this, leftReg, rightReg, label);
            else
                return MIPS_LessThanSet(this, leftReg, rightReg);
        case A_GT:
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return MIPS_GreaterThanEqualJump(this, leftReg, rightReg,
                                                 label);
            else
                return MIPS_GreaterThanSet(this, leftReg, rightReg);
        case A_LE:
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return MIPS_LessThanEqualJump(this, leftReg, rightReg, label);
            else
                return MIPS_LessThanEqualSet(this, leftReg, rightReg);
        case A_GE:
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return MIPS_LessThanEqualJump(this, leftReg, rightReg, label);
            else
                return MIPS_GreaterThanEqualSet(this, leftReg, rightReg);
        case A_INTLIT:
            return MIPS_Load(this, n->v.intvalue);
        case A_IDENT:
            if (n->rvalue || parentASTop == A_DEREF) {
                if (st->Gsym[n->v.id].class == C_GLOBAL) {
                    return MIPS_LoadGlob(this, st, n->v.id, n->op);
                } else {
                    return MIPS_LoadLocal(this, st, n->v.id, n->op);
                }
            } else {
                return NO_REG;
            }
        case A_ASSIGN:
            if (n->right == NULL) {
                fatal("InternalError: Right side of assignment is NULL");
            }
            switch (n->right->op) {
                case A_IDENT:
                    if (st->Gsym[n->right->v.id].class == C_GLOBAL) {
                        return MIPS_StoreGlob(this, leftReg, st,
                                              n->right->v.id);
                    } else {
                        return MIPS_StoreLocal(this, leftReg, st,
                                               n->right->v.id);
                    }
                case A_DEREF:
                    //! bug: storing wrong type - idk if this is fixed yet
                    return MIPS_StoreRef(this, leftReg, rightReg,
                                         n->right->type);
                default:
                    fatala("InternalError: Unknown AST operator for assign %d",
                          n->right->op);
            }
        case A_PRINT:
            if (n->type == P_CHAR) {
                MIPS_PrintChar(this, leftReg);
            } else if (n->type == P_CHARPTR) {
                MIPS_PrintStr(this, leftReg);
            } else {
                MIPS_PrintInt(this, leftReg);
            }
            Compiler_FreeAllReg(this);
            return NO_REG;
        case A_INPUT:
            return MIPS_InputInt(this);
        case A_POKE:
            MIPS_Poke(this, leftReg, rightReg);
            Compiler_FreeAllReg(this);
            return NO_REG;
        case A_PEEK:
            return MIPS_Peek(this, leftReg, rightReg);
        case A_LABEL:
            MIPS_GotoLabel(this, st, n->v.id);
            return NO_REG;
        case A_GOTO:
            MIPS_GotoJump(this, st, n->v.id);
            return NO_REG;
        case A_WIDEN:
            return MIPS_Widen(this, leftReg, n->type);
        case A_RETURN:
            // TODO: Why am I returning id only?
            // What if its a number?
            MIPS_Return(this, st, leftReg, ctx);
            return NO_REG;
        case A_ADDR:
            return MIPS_Address(this, st, n->v.id);
        case A_DEREF:
            if (n->rvalue) {
                if (n->left == NULL) {
                    fatal("InternalError: Left side of deref is NULL");
                }
                return MIPS_Deref(this, leftReg, n->left->type);
            } else {
                return leftReg;
            }
        case A_SCALE:
            switch (n->v.size) {
                // optimization if power of 2 shift it
                case 2:
                    return MIPS_ShiftLeftConstant(this, leftReg, 1);
                case 4:
                    return MIPS_ShiftLeftConstant(this, leftReg, 2);
                default:
                    // loads reg with size
                    // and multiplies leftreg with size
                    rightReg = MIPS_Load(this, n->v.size);
                    return MIPS_Mul(this, leftReg, rightReg);
            }
        case A_STRLIT:
            return MIPS_LoadGlobStr(this, st, n->v.id);

        case A_AND:
            return MIPS_BitAND(this, leftReg, rightReg);
        case A_OR:
            return MIPS_BitOR(this, leftReg, rightReg);
        case A_XOR:
            return MIPS_BitXOR(this, leftReg, rightReg);
        case A_LSHIFT:
            return MIPS_ShiftLeft(this, leftReg, rightReg);
        case A_RSHIFT:
            return MIPS_ShiftRight(this, leftReg, rightReg);
        case A_NEGATE:
            return MIPS_Negate(this, leftReg);
        case A_INVERT:
            return MIPS_BitNOT(this, leftReg);
        case A_LOGNOT:
            return MIPS_LogNOT(this, leftReg);
        case A_PREINC:
        case A_PREDEC:
            if (n->left == NULL) {
                fatal("InternalError: Left side of preinc is NULL");
            }
            return MIPS_LoadGlob(this, st, n->left->v.id, n->op);
        case A_POSTINC:
        case A_POSTDEC:
            return MIPS_LoadGlob(this, st, n->v.id, n->op);
        case A_TOBOOL:
            return MIPS_ToBool(this, parentASTop, leftReg, label);
        default:
            fatala("InternalError: Unknown AST operator %d", n->op);
    }
    // so the compiler will stop complaining
    return 0;
}

static int genIFAST(Compiler this, SymTable st, Context ctx, ASTnode n) {
    int Lfalse, Lend;

    Lfalse = label(this);
    if (n->right) Lend = label(this);

    // reg acts as parameter for label
    genAST(this, st, Lfalse, ctx, n->left, n->op);
    Compiler_FreeAllReg(this);

    genAST(this, st, NO_LABEL, ctx, n->mid, n->op);
    Compiler_FreeAllReg(this);

    if (n->right) MIPS_Jump(this, Lend);
    MIPS_Label(this, Lfalse);

    if (n->right) {
        genAST(this, st, NO_LABEL, ctx, n->right, n->op);
        Compiler_FreeAllReg(this);
        MIPS_Label(this, Lend);
    }

    return NO_REG;
}

static int genWHILEAST(Compiler this, SymTable st, Context ctx, ASTnode n) {
    int Lstart, Lend;

    Lstart = label(this);
    Lend = label(this);

    MIPS_Label(this, Lstart);

    genAST(this, st, Lend, ctx, n->left, n->op);
    Compiler_FreeAllReg(this);

    genAST(this, st, NO_LABEL, ctx, n->right, n->op);
    Compiler_FreeAllReg(this);

    MIPS_Jump(this, Lstart);
    MIPS_Label(this, Lend);

    return NO_REG;
}

static int genFUNCCALLAST(Compiler this, SymTable st, Context ctx, ASTnode n) {
    ASTnode tree = n->left;
    int reg, numArgs = 0;
    int maxArg = tree ? tree->v.size : 0;

    while (tree) {
        reg = genAST(this, st, NO_LABEL, ctx, tree->right, n->op);

        MIPS_ArgCopy(this, reg, tree->v.size, maxArg);

        if (numArgs == 0) numArgs = tree->v.size;
        Compiler_FreeAllReg(this);

        tree = tree->left;
        numArgs++;
    }

    return MIPS_Call(this, st, n->v.id, maxArg);
}