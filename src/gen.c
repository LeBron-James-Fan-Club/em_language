#define _GNU_SOURCE

#include "gen.h"

#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "misc.h"
#include "scan.h"
#include "sym.h"

// TODO: change labels to strings
// TODO: as im going to implement compiler directives that names the labels

static int genAST(Compiler this, SymTable st, Context ctx, ASTnode n, char *ifLabel,
                  char *loopTopLabel, char *loopEndLabel, enum ASTOP parentASTop);
static int genIFAST(Compiler this, SymTable st, Context ctx, ASTnode n,
                    char *loopTopLabel, char *loopEndLabel);
static int genWHILEAST(Compiler this, SymTable st, Context ctx, ASTnode n);
static int genSWITCHAST(Compiler this, SymTable st, Context ctx, ASTnode n);
static int genFUNCCALLAST(Compiler this, SymTable st, Context ctx, ASTnode n);
static int genTERNARYAST(Compiler this, SymTable st, Context ctx, ASTnode n);
static int genLOGANDORAST(Compiler this, SymTable st, Context ctx, ASTnode n);

int Compiler_Gen(Compiler this, SymTable st, Context ctx, ASTnode n) {
    int reg;
    reg = genAST(this, st, ctx, n, NO_LABEL, NO_LABEL, NO_LABEL, -1);
    return reg;
}

static int genAST(Compiler this, SymTable st, Context ctx, ASTnode n,
                  char *ifLabel, char *loopTopLabel, char *loopEndLabel,
                  enum ASTOP parentASTop) {
    int leftReg, rightReg;

    // May use stack system to prevent stack overflow

    if (n == NULL) {
        return NO_REG;
    }

    debug("AST mem: %p OP: %d left %p right %p mid %p", n, n->op, n->left,
          n->right, n->mid);

    switch (n->op) {
        case A_IF:
            return genIFAST(this, st, ctx, n, loopTopLabel, loopEndLabel);
        case A_WHILE:
            return genWHILEAST(this, st, ctx, n);
        case A_SWITCH:
            this->sawSwitch = true;
            return genSWITCHAST(this, st, ctx, n);
        case A_GLUE:
            genAST(this, st, ctx, n->left, ifLabel, loopTopLabel, loopEndLabel,
                   n->op);
            Compiler_FreeAllReg(this, NO_REG);
            fprintf(this->outfile, "\n");
            genAST(this, st, ctx, n->right, ifLabel, loopTopLabel, loopEndLabel,
                   n->op);
            Compiler_FreeAllReg(this, NO_REG);
            if (n->right != NULL) fprintf(this->outfile, "\n");
            return NO_REG;
        case A_FUNCTION:
            MIPS_PreFunc(this, st, ctx);
            genAST(this, st, ctx, n->left, NO_LABEL, NO_LABEL, NO_LABEL, n->op);
            MIPS_PostFunc(this, ctx);
            return NO_REG;
        case A_FUNCCALL:
            return genFUNCCALLAST(this, st, ctx, n);
        case A_TERNARY:
            return genTERNARYAST(this, st, ctx, n);
        case A_LOGAND:
            return genLOGANDORAST(this, st, ctx, n);
        case A_LOGOR:
            return genLOGANDORAST(this, st, ctx, n);
        default:
            // stfu compiler
            break;
    }

    // Mr compiler could you stfu
    leftReg = n->left ? genAST(this, st, ctx, n->left, NO_LABEL, NO_LABEL,
                               NO_LABEL, n->op)
                      : NO_REG;
    rightReg = n->right ? genAST(this, st, ctx, n->right, NO_LABEL, NO_LABEL,
                                 NO_LABEL, n->op)
                        : NO_REG;

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
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_EqualJump(this, leftReg, rightReg, ifLabel);
            else
                return MIPS_EqualSet(this, leftReg, rightReg);
        case A_NE:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_NotEqualJump(this, leftReg, rightReg, ifLabel);
            else
                return MIPS_NotEqualSet(this, leftReg, rightReg);
        case A_LT:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_LessThanJump(this, leftReg, rightReg, ifLabel);
            else
                return MIPS_LessThanSet(this, leftReg, rightReg);
        case A_GT:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_GreaterThanEqualJump(this, leftReg, rightReg,
                                                 ifLabel);
            else
                return MIPS_GreaterThanSet(this, leftReg, rightReg);
        case A_LE:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_LessThanEqualJump(this, leftReg, rightReg, ifLabel);
            else
                return MIPS_LessThanEqualSet(this, leftReg, rightReg);
        case A_GE:
            if (parentASTop == A_IF || parentASTop == A_WHILE ||
                parentASTop == A_TERNARY)
                return MIPS_LessThanEqualJump(this, leftReg, rightReg, ifLabel);
            else
                return MIPS_GreaterThanEqualSet(this, leftReg, rightReg);
        case A_INTLIT:
            return MIPS_Load(this, n->intvalue);
        case A_IDENT:
            if (n->rvalue || parentASTop == A_DEREF) {
                if (n->sym->class == C_GLOBAL || n->sym->class == C_EXTERN ||
                    n->sym->class == C_STATIC) {
                    return MIPS_LoadGlob(this, n->sym, n->op);
                } else {
                    return MIPS_LoadLocal(this, n->sym, n->op);
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
                    leftReg = MIPS_Add(this, leftReg, rightReg);
                    free(n->right);
                    n->right = n->left;
                    n->left = NULL;
                    break;
                case A_ASMINUS:
                    leftReg = MIPS_Sub(this, leftReg, rightReg);
                    free(n->right);
                    n->right = n->left;
                    n->left = NULL;
                    break;
                case A_ASSTAR:
                    leftReg = MIPS_Mul(this, leftReg, rightReg);
                    free(n->right);
                    n->right = n->left;
                    n->left = NULL;
                    break;
                case A_ASSLASH:
                    leftReg = MIPS_Div(this, leftReg, rightReg);
                    free(n->right);
                    n->right = n->left;
                    n->left = NULL;
                    break;
                case A_ASMOD:
                    leftReg = MIPS_Mod(this, leftReg, rightReg);
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
                    if (n->right->sym->class == C_GLOBAL) {
                        return MIPS_StoreGlob(this, leftReg, n->right->sym);
                    } else {
                        return MIPS_StoreLocal(this, leftReg, n->right->sym);
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
            // If $a0 is used then we need to push it

            if (this->paramRegCount > 0) {
                MIPS_RegPush(this, FIRST_PARAM_REG);
            }
            if (n->type == P_CHAR) {
                MIPS_PrintChar(this, leftReg);
            } else if ((n->type & P_CHAR) && ptrtype(n->type)) {
                MIPS_PrintStr(this, leftReg);
            } else {
                MIPS_PrintInt(this, leftReg);
            }
            if (this->paramRegCount > 0) {
                MIPS_RegPop(this, FIRST_PARAM_REG);
            }

            Compiler_FreeAllReg(this, NO_REG);
            return NO_REG;
        case A_INPUT:

            if (this->paramRegCount > 0) {
                MIPS_RegPush(this, FIRST_PARAM_REG);
            }
            int reg;
            if (n->type == pointer_to(P_CHAR)) {
                reg = NO_REG;
                MIPS_InputString(this, n->sym->name, n->sym->nElems);
            } else if (n->type == P_CHAR) {
                reg = MIPS_InputChar(this);
            } else {
                reg = MIPS_InputInt(this);
            }
            if (this->paramRegCount > 0) {
                MIPS_RegPop(this, FIRST_PARAM_REG);
            }

            return reg;
        case A_POKE:
            MIPS_Poke(this, leftReg, rightReg);
            Compiler_FreeAllReg(this, NO_REG);
            return NO_REG;
        case A_PEEK:
            return MIPS_Peek(this, leftReg, rightReg);
        case A_LABEL:
            MIPS_GotoLabel(this, n->sym);
            return NO_REG;
        case A_GOTO:
            MIPS_GotoJump(this, n->sym);
            return NO_REG;
        case A_WIDEN:
            // Unused - might remove later ngl
            return leftReg;
        case A_RETURN:
            MIPS_Return(this, leftReg, ctx);
            return NO_REG;
        case A_ADDR:
            return MIPS_Address(this, n->sym);
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
            switch (n->size) {
                // optimization if power of 2 shift it
                case 2:
                    return MIPS_ShiftLeftConstant(this, leftReg, 1);
                case 4:
                    return MIPS_ShiftLeftConstant(this, leftReg, 2);
                default:
                    // loads reg with size
                    // and multiplies leftreg with size
                    rightReg = MIPS_Load(this, n->size);
                    return MIPS_Mul(this, leftReg, rightReg);
            }
        case A_STRLIT:
            return MIPS_LoadGlobStr(this, n->sym);
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
            debug("Preinc/dec found :)");
            if (n->left == NULL) {
                fatal("InternalError: Left side of preinc is NULL");
            }
            if (n->left->sym->class == C_GLOBAL ||
                n->left->sym->class == C_EXTERN ||
                n->left->sym->class == C_STATIC) {
                return MIPS_LoadGlob(this, n->left->sym, n->op);
            } else {
                return MIPS_LoadLocal(this, n->left->sym, n->op);
            }
        case A_POSTINC:
        case A_POSTDEC:
            debug("Postinc/dec found :)");
            if (n->sym->class == C_GLOBAL || n->sym->class == C_EXTERN ||
                n->sym->class == C_STATIC) {
                return MIPS_LoadGlob(this, n->sym, n->op);
            } else {
                return MIPS_LoadLocal(this, n->sym, n->op);
            }
        case A_TOBOOL:
            return MIPS_ToBool(this, parentASTop, leftReg, ifLabel);
        case A_BREAK:
            MIPS_Jump(this, loopEndLabel);
            return NO_REG;
        case A_CONTINUE:
            MIPS_Jump(this, loopTopLabel);
            return NO_REG;
        case A_CAST:
            return leftReg;
        default:
            fatala("InternalError: Unknown AST operator %d", n->op);
    }
    // so the compiler will stop complaining
    return 0;
}

static int genIFAST(Compiler this, SymTable st, Context ctx, ASTnode n,
                    char *loopTopLabel, char *loopEndLabel) {
    char *Lfalse, *Lend;

    if (n->label.hasCustomLabel) {
        asprintf(&Lfalse, "%s_is_false", n->label.customLabel);
    } else {
        asprintf(&Lfalse, "L%d", Compiler_GenLabel(this));
    }

    if (n->right) {
        if (n->label.hasCustomLabel) {
            asprintf(&Lend, "%s_end", n->label.customLabel);
        } else {
            asprintf(&Lend, "L%d", Compiler_GenLabel(this));
        }
    }

    // reg acts as parameter for label
    genAST(this, st, ctx, n->left, Lfalse, NO_LABEL, NO_LABEL, n->op);
    Compiler_FreeAllReg(this, NO_REG);

    genAST(this, st, ctx, n->mid, NO_LABEL, loopTopLabel, loopEndLabel, n->op);
    Compiler_FreeAllReg(this, NO_REG);

    if (n->right) MIPS_Jump(this, Lend);
    MIPS_Label(this, Lfalse);

    if (n->right) {
        genAST(this, st, ctx, n->right, NO_LABEL, NO_LABEL, NO_LABEL, n->op);
        Compiler_FreeAllReg(this, NO_REG);
        MIPS_Label(this, Lend);
    }

    free(Lfalse);
    if (n->right) free(Lend);

    return NO_REG;
}

static int genWHILEAST(Compiler this, SymTable st, Context ctx, ASTnode n) {
    char *Lstart, *Lend;

    if (n->label.hasCustomLabel) {
        asprintf(&Lstart, "%s_start", n->label.customLabel);
        asprintf(&Lend, "%s_end", n->label.customLabel);
    } else {
        asprintf(&Lstart, "L%d", Compiler_GenLabel(this));
        asprintf(&Lend, "L%d", Compiler_GenLabel(this));
    }

    MIPS_Label(this, Lstart);

    genAST(this, st, ctx, n->left, Lend, Lstart, Lend, n->op);
    Compiler_FreeAllReg(this, NO_REG);

    genAST(this, st, ctx, n->right, NO_LABEL, Lstart, Lend, n->op);
    Compiler_FreeAllReg(this, NO_REG);

    MIPS_Jump(this, Lstart);
    MIPS_Label(this, Lend);

    free(Lstart);
    free(Lend);

    return NO_REG;
}

static int genSWITCHAST(Compiler this, SymTable st, Context ctx, ASTnode n) {
    char **caseLabel, *defaultLabel;
    char *LjumpTop, *Lend;
    int *caseVal, reg, caseCount = 0;
    ASTnode ca;

    if (n->label.hasCustomLabel) {
        asprintf(&LjumpTop, "%s_switch", n->label.customLabel);
        asprintf(&Lend, "%s_end", n->label.customLabel);
    } else {
        asprintf(&LjumpTop, "L%d", Compiler_GenLabel(this));
        asprintf(&Lend, "L%d", Compiler_GenLabel(this));
    }

    caseVal = calloc(n->intvalue + 1, sizeof(int));
    caseLabel = calloc(n->intvalue + 1, sizeof(char *));

    defaultLabel = Lend;

    reg = genAST(this, st, ctx, n->left, NO_LABEL, NO_LABEL, NO_LABEL, A_NONE);
    MIPS_Jump(this, LjumpTop);

    Compiler_FreeAllReg(this, NO_REG);

    ca = n->right;
    for (int i = 0; ca != NULL; i++, ca = ca->right) {
        if (ca->label.hasCustomLabel) {
            asprintf(&caseLabel[i], "case_%s", ca->label.customLabel);
        } else {
            asprintf(&caseLabel[i], "L%d", Compiler_GenLabel(this));
        }
        caseVal[i] = ca->intvalue;
        MIPS_Label(this, caseLabel[i]);
        if (ca->op == A_DEFAULT) {
            defaultLabel = caseLabel[i];
        } else {
            debug("op (SWITCH) %d", ca->op);
            caseCount++;
        }
        if (ca->left)
            genAST(this, st, ctx, ca->left, NO_LABEL, NO_LABEL, Lend, A_NONE);
        Compiler_FreeAllReg(this, NO_REG);
    }

    MIPS_Jump(this, Lend);

    MIPS_Switch(this, reg, caseCount, LjumpTop, caseLabel, caseVal,
                defaultLabel, n->label);
    MIPS_Label(this, Lend);

    free(caseVal);

    for (int i = 0; i < n->intvalue + 1; i++) {
        free(caseLabel[i]);
    }
    free(caseLabel);
    
    free(LjumpTop);
    free(Lend);

    return NO_REG;
}

static int genFUNCCALLAST(Compiler this, SymTable st, Context ctx, ASTnode n) {
    ASTnode tree = n->left;
    int reg, numArgs = 0;

    Compiler_SpillAllRegs(this);

    while (tree) {
        reg = genAST(this, st, ctx, tree->right, NO_LABEL, NO_LABEL, NO_LABEL,
                     n->op);

        MIPS_ArgCopy(this, reg, tree->size);

        if (numArgs == 0) numArgs = tree->size;
        Compiler_FreeAllReg(this, NO_REG);

        tree = tree->left;
        numArgs++;
    }

    return MIPS_Call(this, n->sym);
}

static int genTERNARYAST(Compiler this, SymTable st, Context ctx, ASTnode n) {
    char *Lfalse, *Lend;
    int reg, expReg;

    if (n->label.hasCustomLabel) {
        asprintf(&Lfalse, "%s_false", n->label.customLabel);
        asprintf(&Lend, "%s_end", n->label.customLabel);
    } else {
        asprintf(&Lfalse, "L%d", Compiler_GenLabel(this));
        asprintf(&Lend, "L%d", Compiler_GenLabel(this));
    }

    genAST(this, st, ctx, n->left, Lfalse, NO_LABEL, NO_LABEL, n->op);
    Compiler_FreeAllReg(this, NO_REG);

    reg = allocReg(this);

    expReg = genAST(this, st, ctx, n->mid, NO_LABEL, NO_LABEL, NO_LABEL, n->op);
    MIPS_Move(this, expReg, reg);
    Compiler_FreeAllReg(this, reg);

    MIPS_Jump(this, Lend);
    MIPS_Label(this, Lfalse);

    expReg =
        genAST(this, st, ctx, n->right, NO_LABEL, NO_LABEL, NO_LABEL, n->op);
    MIPS_Move(this, expReg, reg);
    Compiler_FreeAllReg(this, reg);
    MIPS_Label(this, Lend);

    free(Lfalse);
    free(Lend);

    return reg;
}

// TODO: Implement custom labels for && and ||
static int genLOGANDORAST(Compiler this, SymTable st, Context ctx, ASTnode n) {
    char *Lfalse, *Lend;

    if (n->label.hasCustomLabel) {
        asprintf(&Lfalse, "%s_false", n->label.customLabel);
        asprintf(&Lend, "%s_end", n->label.customLabel);
    } else {
        asprintf(&Lfalse, "L%d", Compiler_GenLabel(this));
        asprintf(&Lend, "L%d", Compiler_GenLabel(this));
    }

    int reg;

    reg = genAST(this, st, ctx, n->left, NO_LABEL, NO_LABEL, NO_LABEL, 0);
    MIPS_Boolean(this, reg, n->op, Lfalse);
    Compiler_FreeAllReg(this, NO_REG);

    reg = genAST(this, st, ctx, n->right, NO_LABEL, NO_LABEL, NO_LABEL, 0);
    MIPS_Boolean(this, reg, n->op, Lfalse);
    Compiler_FreeAllReg(this, reg);

    if (n->op == A_LOGAND) {
        MIPS_LoadBoolean(this, reg, 1);
        MIPS_Jump(this, Lend);
        MIPS_Label(this, Lfalse);
        MIPS_LoadBoolean(this, reg, 0);
    } else {
        MIPS_LoadBoolean(this, reg, 0);
        MIPS_Jump(this, Lend);
        MIPS_Label(this, Lfalse);
        MIPS_LoadBoolean(this, reg, 1);
    }
    MIPS_Label(this, Lend);
    
    free(Lfalse);
    free(Lend);

    return reg;
}