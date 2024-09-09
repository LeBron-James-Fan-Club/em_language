#include "gen.h"

#include <stdbool.h>
#include <stdio.h>

#include "ast.h"
#include "misc.h"
#include "scan.h"
#include "sym.h"

#define NO_LABEL -1

// TODO: change labels to strings
// TODO: as im going to implement compiler directives that names the labels

static int genAST(Compiler self, SymTable st, Context ctx, ASTnode n, int label,
                  int loopTopLabel, int loopEndLabel, int parentASTop);
static int genIFAST(Compiler self, SymTable st, Context ctx, ASTnode n,
                    int loopTopLabel, int loopEndLabel);
static int genWHILEAST(Compiler self, SymTable st, Context ctx, ASTnode n);
static int genSWITCHAST(Compiler self, SymTable st, Context ctx, ASTnode n);
static int genFUNCCALLAST(Compiler self, SymTable st, Context ctx, ASTnode n);
static int genTERNARYAST(Compiler self, SymTable st, Context ctx, ASTnode n);

int Compiler_Gen(Compiler self, SymTable st, Context ctx, ASTnode n) {
    int reg;
    reg = genAST(self, st, ctx, n, NO_LABEL, NO_LABEL, NO_LABEL, -1);
    return reg;
}

static int genAST(Compiler self, SymTable st, Context ctx, ASTnode n,
                  int ifLabel, int loopTopLabel, int loopEndLabel,
                  int parentASTop) {
    int leftReg, rightReg;

    // May use stack system to prevent stack overflow

    if (n == NULL) {
        return NO_REG;
    }

    debug("AST mem: %p OP: %d left %p right %p mid %p", n, n->op, n->left,
          n->right, n->mid);

    switch (n->op) {
        case A_IF:
            return genIFAST(self, st, ctx, n, loopTopLabel, loopEndLabel);
        case A_WHILE:
            return genWHILEAST(self, st, ctx, n);
        case A_SWITCH:
            return genSWITCHAST(self, st, ctx, n);
        case A_GLUE:
            genAST(self, st, ctx, n->left, ifLabel, loopTopLabel, loopEndLabel,
                   n->op);
            Compiler_FreeAllReg(self, NO_REG);
            fprintf(self->outfile, "\n");
            genAST(self, st, ctx, n->right, ifLabel, loopTopLabel, loopEndLabel,
                   n->op);
            Compiler_FreeAllReg(self, NO_REG);
            if (n->right != NULL) fprintf(self->outfile, "\n");
            return NO_REG;
        case A_FUNCTION:
            MIPS_PreFunc(self, st, ctx);
            genAST(self, st, ctx, n->left, NO_LABEL, NO_LABEL, NO_LABEL, n->op);
            MIPS_PostFunc(self, ctx);
            return NO_REG;
        case A_FUNCCALL:
            return genFUNCCALLAST(self, st, ctx, n);
        case A_TERNARY:
            return genTERNARYAST(self, st, ctx, n);
        default:
            // stfu compiler
            break;
    }

    // Mr compiler could you stfu
    leftReg = n->left ? genAST(self, st, ctx, n->left, NO_LABEL, NO_LABEL,
                               NO_LABEL, n->op)
                      : NO_REG;
    rightReg = n->right ? genAST(self, st, ctx, n->right, NO_LABEL, NO_LABEL,
                                 NO_LABEL, n->op)
                        : NO_REG;

    switch (n->op) {
        case A_ADD:
            return MIPS_Add(self, leftReg, rightReg);
        case A_SUBTRACT:
            return MIPS_Sub(self, leftReg, rightReg);
        case A_MULTIPLY:
            return MIPS_Mul(self, leftReg, rightReg);
        case A_DIVIDE:
            return MIPS_Div(self, leftReg, rightReg);
        case A_MODULO:
            return MIPS_Mod(self, leftReg, rightReg);
        case A_EQ:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_EqualJump(self, leftReg, rightReg, ifLabel);
            else
                return MIPS_EqualSet(self, leftReg, rightReg);
        case A_NE:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_NotEqualJump(self, leftReg, rightReg, ifLabel);
            else
                return MIPS_NotEqualSet(self, leftReg, rightReg);
        case A_LT:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_LessThanJump(self, leftReg, rightReg, ifLabel);
            else
                return MIPS_LessThanSet(self, leftReg, rightReg);
        case A_GT:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_GreaterThanEqualJump(self, leftReg, rightReg,
                                                 ifLabel);
            else
                return MIPS_GreaterThanSet(self, leftReg, rightReg);
        case A_LE:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_LessThanEqualJump(self, leftReg, rightReg, ifLabel);
            else
                return MIPS_LessThanEqualSet(self, leftReg, rightReg);
        case A_GE:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_LessThanEqualJump(self, leftReg, rightReg, ifLabel);
            else
                return MIPS_GreaterThanEqualSet(self, leftReg, rightReg);
        case A_LOGAND:
            return MIPS_LogAnd(self, leftReg, rightReg);
        case A_LOGOR:
            return MIPS_LogOr(self, leftReg, rightReg);
        case A_INTLIT:
            return MIPS_Load(self, n->intvalue);
        case A_IDENT:
            if (n->rvalue || parentASTop == A_DEREF) {
                if (n->sym->_class == C_GLOBAL || n->sym->_class == C_EXTERN ||
                    n->sym->_class == C_STATIC) {
                    return MIPS_LoadGlob(self, n->sym, n->op);
                } else {
                    return MIPS_LoadLocal(self, n->sym, n->op);
                }
            } else {
                debug("IDENT %s has NO_REG", n->sym->name);
                return NO_REG;
            }
        case A_ASPLUS:
        case A_ASMINUS:
        case A_ASSTAR:
        case A_ASSLASH:
        case A_ASMOD:
        case A_ASSIGN:
            // left is the destination
            // we move right cause it uses right of tree

            switch (n->op) {
                case A_ASPLUS:
                    leftReg = MIPS_Add(self, leftReg, rightReg);
                    free(n->right);
                    n->right = n->left;
                    n->left = NULL;
                    break;
                case A_ASMINUS:
                    leftReg = MIPS_Sub(self, leftReg, rightReg);
                    free(n->right);
                    n->right = n->left;
                    n->left = NULL;
                    break;
                case A_ASSTAR:
                    leftReg = MIPS_Mul(self, leftReg, rightReg);
                    free(n->right);
                    n->right = n->left;
                    n->left = NULL;
                    break;
                case A_ASSLASH:
                    leftReg = MIPS_Div(self, leftReg, rightReg);
                    free(n->right);
                    n->right = n->left;
                    n->left = NULL;
                    break;
                case A_ASMOD:
                    leftReg = MIPS_Mod(self, leftReg, rightReg);
                    free(n->right);
                    n->right = n->left;
                    n->left = NULL;
                    break;
            }

            if (n->right == NULL) {
                fatal("InternalError: Right side of assignment is NULL");
            }
            switch (n->right->op) {
                case A_IDENT:
                    if (n->right->sym->_class == C_GLOBAL) {
                        return MIPS_StoreGlob(self, leftReg, n->right->sym);
                    } else {
                        return MIPS_StoreLocal(self, leftReg, n->right->sym);
                    }
                case A_DEREF:
                    //! bug: storing wrong type - idk if self is fixed yet
                    return MIPS_StoreRef(self, leftReg, rightReg,
                                         n->right->type);
                default:
                    fatala("InternalError: Unknown AST operator for assign %d",
                           n->right->op);
            }
        case A_PRINT:
            // If $a0 is used then we need to push it

            if (self->paramRegCount > 0) {
                MIPS_RegPush(self, FIRST_PARAM_REG);
            }
            if (n->type == P_CHAR) {
                MIPS_PrintChar(self, leftReg);
            } else if ((n->type & P_CHAR) && ptrtype(n->type)) {
                MIPS_PrintStr(self, leftReg);
            } else {
                MIPS_PrintInt(self, leftReg);
            }
            if (self->paramRegCount > 0) {
                MIPS_RegPop(self, FIRST_PARAM_REG);
            }

            Compiler_FreeAllReg(self, NO_REG);
            return NO_REG;
        case A_INPUT:

            if (self->paramRegCount > 0) {
                MIPS_RegPush(self, FIRST_PARAM_REG);
            }
            int reg;
            if (n->type == pointer_to(P_CHAR)) {
                reg = NO_REG;
                MIPS_InputString(self, n->sym->name, n->sym->size);
            } else if (n->type == P_CHAR) {
                reg = MIPS_InputChar(self);
            } else {
                reg = MIPS_InputInt(self);
            }
            if (self->paramRegCount > 0) {
                MIPS_RegPop(self, FIRST_PARAM_REG);
            }

            return reg;
        case A_POKE:
            MIPS_Poke(self, leftReg, rightReg);
            Compiler_FreeAllReg(self, NO_REG);
            return NO_REG;
        case A_PEEK:
            return MIPS_Peek(self, leftReg, rightReg);
        case A_LABEL:
            MIPS_GotoLabel(self, n->sym);
            return NO_REG;
        case A_GOTO:
            MIPS_GotoJump(self, n->sym);
            return NO_REG;
        case A_WIDEN:
            // Unused - might remove later ngl
            return leftReg;
        case A_RETURN:
            MIPS_Return(self, leftReg, ctx);
            return NO_REG;
        case A_ADDR:
            return MIPS_Address(self, n->sym);
        case A_DEREF:
            if (n->rvalue) {
                if (n->left == NULL) {
                    fatal("InternalError: Left side of deref is NULL");
                }
                return MIPS_Deref(self, leftReg, n->left->type);
            } else {
                return leftReg;
            }
        case A_SCALE:
            switch (n->size) {
                // optimization if power of 2 shift it
                case 2:
                    return MIPS_ShiftLeftConstant(self, leftReg, 1);
                case 4:
                    return MIPS_ShiftLeftConstant(self, leftReg, 2);
                default:
                    // loads reg with size
                    // and multiplies leftreg with size
                    rightReg = MIPS_Load(self, n->size);
                    return MIPS_Mul(self, leftReg, rightReg);
            }
        case A_STRLIT:
            return MIPS_LoadGlobStr(self, n->sym);
        case A_AND:
            return MIPS_BitAND(self, leftReg, rightReg);
        case A_OR:
            return MIPS_BitOR(self, leftReg, rightReg);
        case A_XOR:
            return MIPS_BitXOR(self, leftReg, rightReg);
        case A_LSHIFT:
            return MIPS_ShiftLeft(self, leftReg, rightReg);
        case A_RSHIFT:
            return MIPS_ShiftRight(self, leftReg, rightReg);
        case A_NEGATE:
            return MIPS_Negate(self, leftReg);
        case A_INVERT:
            return MIPS_BitNOT(self, leftReg);
        case A_LOGNOT:
            return MIPS_LogNOT(self, leftReg);
        case A_PREINC:
        case A_PREDEC:
            debug("Preinc/dec found :)");
            if (n->left == NULL) {
                fatal("InternalError: Left side of preinc is NULL");
            }
            if (n->left->sym->_class == C_GLOBAL ||
                n->left->sym->_class == C_EXTERN ||
                n->left->sym->_class == C_STATIC) {
                return MIPS_LoadGlob(self, n->left->sym, n->op);
            } else {
                return MIPS_LoadLocal(self, n->left->sym, n->op);
            }
        case A_POSTINC:
        case A_POSTDEC:
            debug("Postinc/dec found :)");
            if (n->sym->_class == C_GLOBAL || n->sym->_class == C_EXTERN ||
                n->sym->_class == C_STATIC) {
                return MIPS_LoadGlob(self, n->sym, n->op);
            } else {
                return MIPS_LoadLocal(self, n->sym, n->op);
            }
        case A_TOBOOL:
            return MIPS_ToBool(self, parentASTop, leftReg, ifLabel);
        case A_BREAK:
            MIPS_Jump(self, loopEndLabel);
            return NO_REG;
        case A_CONTINUE:
            MIPS_Jump(self, loopTopLabel);
            return NO_REG;
        case A_CAST:
            return leftReg;
        default:
            fatala("InternalError: Unknown AST operator %d", n->op);
    }
    // so the compiler will stop complaining
    return 0;
}

static int genIFAST(Compiler self, SymTable st, Context ctx, ASTnode n,
                    int loopTopLabel, int loopEndLabel) {
    int Lfalse, Lend;

    Lfalse = Compiler_GenLabel(self);
    if (n->right) Lend = Compiler_GenLabel(self);

    // reg acts as parameter for label
    genAST(self, st, ctx, n->left, Lfalse, NO_LABEL, NO_LABEL, n->op);
    Compiler_FreeAllReg(self, NO_REG);

    genAST(self, st, ctx, n->mid, NO_LABEL, loopTopLabel, loopEndLabel, n->op);
    Compiler_FreeAllReg(self, NO_REG);

    if (n->right) MIPS_Jump(self, Lend);
    MIPS_Label(self, Lfalse);

    if (n->right) {
        genAST(self, st, ctx, n->right, NO_LABEL, NO_LABEL, NO_LABEL, n->op);
        Compiler_FreeAllReg(self, NO_REG);
        MIPS_Label(self, Lend);
    }

    return NO_REG;
}

static int genWHILEAST(Compiler self, SymTable st, Context ctx, ASTnode n) {
    int Lstart, Lend;

    Lstart = Compiler_GenLabel(self);
    Lend = Compiler_GenLabel(self);

    MIPS_Label(self, Lstart);

    genAST(self, st, ctx, n->left, Lend, Lstart, Lend, n->op);
    Compiler_FreeAllReg(self, NO_REG);

    genAST(self, st, ctx, n->right, NO_LABEL, Lstart, Lend, n->op);
    Compiler_FreeAllReg(self, NO_REG);

    MIPS_Jump(self, Lstart);
    MIPS_Label(self, Lend);

    return NO_REG;
}

static int genSWITCHAST(Compiler self, SymTable st, Context ctx, ASTnode n) {
    int *caseVal, *caseLabel;
    int LjumpTop, Lend;
    int reg, defaultLabel = 0, caseCount = 0;
    ASTnode ca;

    caseVal = calloc(n->intvalue + 1, sizeof(int));
    caseLabel = calloc(n->intvalue + 1, sizeof(int));

    LjumpTop = Compiler_GenLabel(self);
    Lend = Compiler_GenLabel(self);

    defaultLabel = Lend;

    reg = genAST(self, st, ctx, n->left, NO_LABEL, NO_LABEL, NO_LABEL, A_NONE);
    MIPS_Jump(self, LjumpTop);

    Compiler_FreeAllReg(self, NO_REG);

    ca = n->right;
    for (int i = 0; ca != NULL; i++, ca = ca->right) {
        caseLabel[i] = Compiler_GenLabel(self);
        caseVal[i] = ca->intvalue;
        debug("case value: %d", ca->intvalue);
        debug("caseVal[%d] = %d", i, caseVal[i]);
        MIPS_Label(self, caseLabel[i]);
        if (ca->op == A_DEFAULT) {
            debug("hit default");
            defaultLabel = caseLabel[i];
        } else {
            debug("op (SWITCH) %d", ca->op);
            caseCount++;
        }
        if (ca->left)
            genAST(self, st, ctx, ca->left, NO_LABEL, NO_LABEL, Lend, A_NONE);
        Compiler_FreeAllReg(self, NO_REG);
    }

    MIPS_Jump(self, Lend);

    MIPS_Switch(self, reg, caseCount, LjumpTop, caseLabel, caseVal,
                defaultLabel);
    MIPS_Label(self, Lend);

    free(caseVal);
    free(caseLabel);

    return NO_REG;
}

static int genFUNCCALLAST(Compiler self, SymTable st, Context ctx, ASTnode n) {
    ASTnode tree = n->left;
    int reg, numArgs = 0;

    while (tree) {
        reg = genAST(self, st, ctx, tree->right, NO_LABEL, NO_LABEL, NO_LABEL,
                     n->op);

        MIPS_ArgCopy(self, reg, tree->size);

        if (numArgs == 0) numArgs = tree->size;
        Compiler_FreeAllReg(self, NO_REG);

        tree = tree->left;
        numArgs++;
    }

    return MIPS_Call(self, n->sym);
}

static int genTERNARYAST(Compiler self, SymTable st, Context ctx, ASTnode n) {
    int Lfalse, Lend;
    int reg, expReg;

    Lfalse = Compiler_GenLabel(self);
    Lend = Compiler_GenLabel(self);

    genAST(self, st, ctx, n->left, Lfalse, NO_LABEL, NO_LABEL, n->op);
    Compiler_FreeAllReg(self, NO_REG);

    reg = allocReg(self);

    expReg = genAST(self, st, ctx, n->mid, NO_LABEL, NO_LABEL, NO_LABEL, n->op);
    MIPS_Move(self, expReg, reg);
    Compiler_FreeAllReg(self, reg);

    MIPS_Jump(self, Lend);
    MIPS_Label(self, Lfalse);

    expReg =
        genAST(self, st, ctx, n->right, NO_LABEL, NO_LABEL, NO_LABEL, n->op);
    MIPS_Move(self, expReg, reg);
    Compiler_FreeAllReg(self, reg);
    MIPS_Label(self, Lend);
    return reg;
}