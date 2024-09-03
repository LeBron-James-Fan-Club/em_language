#include "asm.h"

#include "misc.h"

static char *reglist[MAX_REG] = {"$t0", "$t1", "$t2", "$t3", "$t4",
                                 "$t5", "$t6", "$t7", "$t8", "$t9",
                                 "$a0", "$a1", "$a2", "$a3"};

static int allocReg(Compiler this);
static void freeReg(Compiler this, int reg1);

int PrimSize(enum ASTPRIM type) {
    if (ptrtype(type)) {
        return 4;
    }
    switch (type) {
        case P_CHAR:
            return 1;
        case P_INT:
            return 4;
        default:
            debug("primsize error");
            fatala("InternalError: Unknown type %d", type);
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

/**
 * Push to stack in reverse
 *  $a3 - $a0
 * increment starting by (8 + offset of local vars)
 * do not use negatives for offsets
 * e.g. lw $t0, 8($fp)
 *      lw $t1, 12($fp)
 *      lw $t2, 16($fp)
 *      lw $t3, 20($fp)
 *      lw $t4, 24($fp)
 * TODO: need to redo local variables - done
 * offset need to be after 0 and then negatives
 */

void MIPS_PreFunc(Compiler this, SymTable st, Context ctx) {
    char *name = ctx->functionId->name;

    int paramReg = FIRST_PARAM_REG;

    Compiler_ResetOffset(this);

    // 0 is $ra and 1 is $fp

    fprintf(this->outfile,
            ".text\n"
            "\t.globl %s\n"
            "\n"
            "%s:\n",
            name, name);

    debug("Function: %s", name);

    SymTableEntry paramCurr = ctx->functionId->member;

    for (; paramCurr != NULL; paramCurr = paramCurr->next) {
        if (paramReg > FIRST_PARAM_REG + 3) {
            break;
        }

        paramCurr->isFirstFour = true;
        debug("paramReg: %d", paramReg);
        paramCurr->paramReg = paramReg++;
        this->paramRegCount++;
    }

    for (; paramCurr != NULL; paramCurr = paramCurr->next) {
        // for remaining params they get pushed on stack
        paramCurr->offset = Compiler_GetParamOffset(this, paramCurr->type);
    }

    SymTableEntry loclCurr = st->loclHead;

    for (; loclCurr != NULL; loclCurr = loclCurr->next) {
        // same thing but for local
        loclCurr->offset = Compiler_GetLocalOffset(this, loclCurr->type);
    }

    // need to add local offset to all offsets somehow

    fprintf(this->outfile,
            "\tbegin\n\n"
            "\tpush $ra\n");

    // Actual offset for locals if have been initialised
    if (st->loclHead) {
        fprintf(this->outfile, "\taddiu\t$sp, $sp, %d\n",
                -(this->localOffset - 8));
        for (loclCurr = st->loclHead; loclCurr != NULL;
             loclCurr = loclCurr->next) {
            if (loclCurr->hasValue) {
                int r = MIPS_Load(this, loclCurr->value);
                MIPS_StoreLocal(this, r, loclCurr);
                freeReg(this, r);
            }
        }
    }

    fputs("\n", this->outfile);
}

void MIPS_PostFunc(Compiler this, Context ctx) {
    // TODO: if there are no early returns
    // TODO: we don't add return label
    MIPS_ReturnLabel(this, ctx);
    fprintf(this->outfile,
            "\taddiu\t$sp, $sp, %d\n"
            "\tpop\t$ra\n"
            "\tend\n"
            "\tjr\t$ra\n\n",
            this->localOffset - 8);
}

int MIPS_Load(Compiler this, int value) {
    int r = allocReg(this);
    fprintf(this->outfile, "\tli\t%s, %d\n", reglist[r], value);
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

void MIPS_PrintStr(Compiler this, int r) {
    fprintf(this->outfile, "\tli\t$v0, 4\n");
    fprintf(this->outfile, "\tmove\t$a0, %s\n", reglist[r]);
    fprintf(this->outfile, "\tsyscall\n");
}

int MIPS_LoadGlobStr(Compiler this, SymTableEntry sym) {
    int r = allocReg(this);
    fprintf(this->outfile, "\tla\t%s, %s\n", reglist[r], sym->name);
    return r;
}

int MIPS_LoadGlob(Compiler this, SymTableEntry sym, enum ASTOP op) {
    int r = allocReg(this);
    int r2;
    switch (sym->type) {
        case P_INT:
            fprintf(this->outfile, "\tlw\t%s, %s\n", reglist[r], sym->name);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r], sym->name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r2],
                        sym->name);
                freeReg(this, r2);
            }

            break;
        case P_CHAR:
            fprintf(this->outfile, "\tlbu\t%s, %s\n", reglist[r], sym->name);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsb\t%s, %s\n", reglist[r], sym->name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsb\t%s, %s\n", reglist[r2],
                        sym->name);
                freeReg(this, r2);
            }

            break;
        // case P_CHARPTR:
        // case P_INTPTR:
        default:
            fprintf(this->outfile, "\tlw\t%s, %s\n", reglist[r], sym->name);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r], sym->name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r2],
                        sym->name);
                freeReg(this, r2);
            }
            break;
            // default:
            //     debug("load glob error");
            //    fatala("InternalError: Unknown type %d", sym->type);
    }
    return r;
}

int MIPS_LoadLocal(Compiler this, SymTableEntry sym, enum ASTOP op) {
    int r = allocReg(this);
    int r2;

    if (sym->isFirstFour) {
        debug("paramReg: %d", sym->paramReg);
        fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r],
                reglist[sym->paramReg]);

        if (op == A_PREINC || op == A_PREDEC) {
            fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                    reglist[r], op == A_PREINC ? "1" : "-1");
            fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r],
                    reglist[sym->paramReg]);
        }

        if (op == A_POSTINC || op == A_POSTDEC) {
            r2 = allocReg(this);
            fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2], reglist[r]);
            fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                    reglist[r2], op == A_POSTINC ? "1" : "-1");
            fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r],
                    reglist[sym->paramReg]);
            freeReg(this, r2);
        }
        return r;
    }

    switch (sym->type) {
        case P_INT:
            fprintf(this->outfile, "\tlw\t%s, %d($sp)\n", reglist[r],
                    sym->offset);
            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, -%d($sp)\n", reglist[r],
                        sym->offset);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r2],
                        sym->offset);
                freeReg(this, r2);
            }

            break;
        case P_CHAR:
            fprintf(this->outfile, "\tlbu\t%s, %d($sp)\n", reglist[r],
                    sym->offset);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsb\t%s, %d($sp)\n", reglist[r],
                        sym->offset);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsb\t%s, %d($sp)\n", reglist[r2],
                        sym->offset);
                freeReg(this, r2);
            }

            break;
        // case P_CHARPTR:
        // case P_INTPTR:
        default:
            if (!ptrtype(sym->type)) {
                debug("load local error");
                fatala("InternalError: Unknown type %d", sym->type);
            }
            fprintf(this->outfile, "\tlw\t%s, %d($sp)\n", reglist[r],
                    sym->offset);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r],
                        sym->offset);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r2],
                        sym->offset);
                freeReg(this, r2);
            }
            break;
    }
    return r;
}

int MIPS_StoreGlob(Compiler this, int r1, SymTableEntry sym) {
    if (r1 == NO_REG) {
        fatal("InternalError: Trying to store an empty register");
    }

    switch (sym->type) {
        case P_INT:
            fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r1], sym->name);
            break;
        case P_CHAR:
            fprintf(this->outfile, "\tsb\t%s, %s\n", reglist[r1], sym->name);
            break;
        default:
            if (!ptrtype(sym->type)) {
                debug("store glob error");
                fatala("InternalError: Unknown type %d", sym->type);
            }

            fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r1], sym->name);
            break;
    }

    return r1;
}

void MIPS_StoreParam(Compiler this, int r1) {
    if (r1 == NO_REG) {
        fatal("InternalError: Trying to store an empty register");
    }

    fprintf(this->outfile, "\tpush\t%s\n", reglist[r1]);
}

int MIPS_StoreLocal(Compiler this, int r1, SymTableEntry sym) {
    if (r1 == NO_REG) {
        fatal("InternalError: Trying to store an empty register");
    }

    if (sym->isFirstFour) {
        fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[sym->paramReg],
                reglist[r1]);
        return r1;
    }

    switch (sym->type) {
        case P_INT:
            fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r1],
                    sym->offset);
            break;
        case P_CHAR:
            fprintf(this->outfile, "\tsb\t%s, %d($sp)\n", reglist[r1],
                    sym->offset);
            break;
        default:
            if (!ptrtype(sym->type)) {
                debug("store local error");
                fatala("InternalError: Unknown type %d", sym->type);
            }

            fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r1],
                    sym->offset);
            break;
    }

    return r1;
}

int MIPS_StoreRef(Compiler this, int r1, int r2, enum ASTPRIM type) {
    if (r1 == NO_REG) {
        fatal("InternalError: Trying to store an empty register");
    }

    if (r2 == NO_REG) {
        fatal("InternalError: Trying to store an empty register for r2");
    }

    int size = PrimSize(type);

    switch (size) {
        case 1:
            fprintf(this->outfile, "\tsb\t%s, 0(%s)\n", reglist[r1],
                    reglist[r2]);
            break;
        case 4:
            fprintf(this->outfile, "\tsw\t%s, 0(%s)\n", reglist[r1],
                    reglist[r2]);
            break;
        default:
            debug("store ref error");
            fatala("InternalError: Unknown type %d", type);
    }

    return r1;
}

int MIPS_Widen(Compiler this, int r1, enum ASTPRIM newType) {
    // nothing to do, its already done by zero extending
    return r1;
}

int MIPS_Align(enum ASTPRIM type, int offset, int dir) {
    int align;

    debug("type %d", type);

    switch (type) {
        case P_CHAR:
            return offset;
        case P_INT:
            break;
        default:
            debug("align error");
            fatala("InternalError: Unknown type %d", type);
    }

    align = 4;
    // aligns shit??
    offset = (offset + dir * (align - 1)) & ~(align - 1);
    debug("offset struct rn: %d", offset);
    return offset;
}

// Needs to be below .data
void MIPS_GlobSym(Compiler this, SymTableEntry sym) {
    int size = type_size(sym->type, sym->ctype);

    // Might work idk?
    if ((sym->type & P_CHAR) && ptrtype(sym->type) && sym->hasValue) {
        fprintf(this->outfile, "\t%s:\t.asciiz \"%s\"\n", sym->name,
                sym->strValue);
        return;
    }

    switch (size) {
        case 1:
            fprintf(this->outfile, "\t%s:\t.byte %d\n", sym->name, sym->value);
            fputs("\t.align 2\n", this->outfile);
            break;
        case 4:
            fprintf(this->outfile, "\t%s:\t.word %d\n", sym->name, sym->value);
            break;
        default:
            // ! won't work on multi-dimensional arrays
            debug("sym type %d", sym->type);
            debug("sym size %d", sym->size);
            debug("other %d", (sym->type == P_STRUCT || sym->type == P_UNION)
                                  ? size
                                  : PrimSize(sym->type));
            fprintf(this->outfile, "\t%s:\t.space %d\n", sym->name,
                    (sym->type == P_STRUCT || sym->type == P_UNION)
                        ? size
                        : PrimSize(sym->type) * sym->size);
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

void MIPS_GotoLabel(Compiler this, SymTableEntry sym) {
    fprintf(this->outfile, "%s:\n", sym->name);
}

void MIPS_GotoJump(Compiler this, SymTableEntry sym) {
    fprintf(this->outfile, "\tb\t%s\n", sym->name);
}

void MIPS_ReturnLabel(Compiler this, Context ctx) {
    fprintf(this->outfile, "%s_end:\n", ctx->functionId->name);
}

void MIPS_ReturnJump(Compiler this, Context ctx) {
    fprintf(this->outfile, "\tb\t%s_end\n", ctx->functionId->name);
}

void MIPS_Return(Compiler this, int r, Context ctx) {
    if (ctx->functionId->type == P_INT) {
        fprintf(this->outfile, "\tmove\t$v0, %s\n", reglist[r]);
    } else if (ctx->functionId->type == P_CHAR) {
        fprintf(this->outfile, "\tmove\t$v0, %s\n", reglist[r]);
        // I dont think we need the below
        // fprintf(this->outfile, "\tandi\t$v0, %s, 0xFF\n", reglist[r]);
    } else {
        fatala("InternalError: Unknown type %d", ctx->functionId->type);
    }
    MIPS_ReturnJump(this, ctx);
}

int MIPS_Call(Compiler this, SymTableEntry sym) {
    int outr = allocReg(this);
    fprintf(this->outfile, "\tjal\t%s\n", sym->name);
    fprintf(this->outfile, "\tmove\t%s, $v0\n", reglist[outr]);
    // Calculates the length of parameters
    int offset = 0;

    // If $a0-a3 are used
    for (int i = 0; i < this->paramRegCount; i++) {
        fprintf(this->outfile, "\tpush\t%s\n", reglist[FIRST_PARAM_REG + i]);
    }

    for (SymTableEntry curr = sym->member; curr != NULL; curr = curr->next) {
        offset += PrimSize(curr->type);
    }
    if (offset > 16)
        fprintf(this->outfile, "\taddiu\t$sp, $sp, %d\n", offset - 16);

    for (int i = 0; i < this->paramRegCount; i++) {
        fprintf(this->outfile, "\tpop\t%s\n", reglist[FIRST_PARAM_REG + i]);
    }

    offset = 0;
    return outr;
}

void MIPS_ArgCopy(Compiler this, int r, int argPos, int maxArg) {
    /*
    Stack:
    (params)

    (old frame pointer)
    (return address)
    (local variables)
    */

    // Basically greater than 4 (+ 1 for index)
    if (argPos > 4) {
        fprintf(this->outfile, "\tpush\t%s\n", reglist[r]);
    } else {
        fprintf(this->outfile, "\tmove\t%s, %s\n",
                reglist[FIRST_PARAM_REG + argPos - 1], reglist[r]);
    }
}

void MIPS_RegPush(Compiler this, int r) {
    fprintf(this->outfile, "\tpush\t%s\n", reglist[r]);
}

void MIPS_RegPop(Compiler this, int r) {
    fprintf(this->outfile, "\tpop\t%s\n", reglist[r]);
}

int MIPS_Address(Compiler this, SymTableEntry sym) {
    int r = allocReg(this);
    if (sym->class == C_GLOBAL) {
        fprintf(this->outfile, "\tla\t%s, %s\n", reglist[r], sym->name);
    } else {
        fprintf(this->outfile, "\tla\t%s, %d($sp)\n", reglist[r], sym->offset);
    }
    return r;
}

int MIPS_Deref(Compiler this, int r, enum ASTPRIM type) {
    //! bug: derefing not occuring for b[3]

    enum ASTPRIM newType = value_at(type);
    int size = PrimSize(newType);

    switch (size) {
        case 1:
            fprintf(this->outfile, "\tlbu\t%s, 0(%s)\n", reglist[r],
                    reglist[r]);
            break;
        case 4:
            fprintf(this->outfile, "\tlw\t%s, 0(%s)\n", reglist[r], reglist[r]);
            break;
        default:
            fatala("InternalError: Unknown pointer type %d", type);
    }
    return r;
}

int MIPS_ShiftLeftConstant(Compiler this, int r, int c) {
    fprintf(this->outfile, "\tsll\t%s, %s, %d\n", reglist[r], reglist[r], c);
    return r;
}

int MIPS_ShiftLeft(Compiler this, int r1, int r2) {
    // swap values if it doesnt work
    fprintf(this->outfile, "\tsllv\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_ShiftRight(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tsrav\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_Negate(Compiler this, int r) {
    fprintf(this->outfile, "\tneg\t%s, %s\n", reglist[r], reglist[r]);
    return r;
}

int MIPS_BitNOT(Compiler this, int r) {
    fprintf(this->outfile, "\tnot\t%s, %s\n", reglist[r], reglist[r]);
    return r;
}

int MIPS_LogNOT(Compiler this, int r) {
    fprintf(this->outfile, "\tnor\t%s, %s, %s\n", reglist[r], reglist[r],
            reglist[r]);
    return r;
}

int MIPS_BitAND(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tand\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_BitOR(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tor\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_BitXOR(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\txor\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(this, r1);
    return r2;
}

int MIPS_ToBool(Compiler this, enum ASTOP parentOp, int r, int label) {
    if (parentOp == A_WHILE || parentOp == A_IF) {
        // fake instruction
        // used to be bltu - but for some reason it never works
        fprintf(this->outfile, "\tbeq\t%s, $zero, L%d\n", reglist[r], label);
        return r;
    } else {
        fprintf(this->outfile, "\tseq\t%s, %s, $zero\n", reglist[r],
                reglist[r]);
    }
    return r;
}

void MIPS_Poke(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tsw\t%s, 0(%s)\n", reglist[r1], reglist[r2]);
}

int MIPS_Peek(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tlw\t%s, 0(%s)\n", reglist[r1], reglist[r2]);
    freeReg(this, r2);
    return r1;
}

static int allocReg(Compiler this) {
    for (int i = 0; i < MAX_REG; i++) {
        if (!this->regUsed[i]) {
            this->regUsed[i] = true;
            return i;
        }
    }
    fatal("InternalError: Out of registers");
}

static void freeReg(Compiler this, int reg1) {
    if (!this->regUsed[reg1]) {
        fatal("InternalError: Trying to free a free register");
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
    bool found = false;
    debug("Generating globs");

    for (SymTableEntry curr = st->globHead; curr != NULL; curr = curr->next) {
        // TODO: Remove these lines below (2)
        // TODO: and see if it fucks up anything

        if (curr->stype == S_FUNC) continue;
        if (curr->class != C_GLOBAL) continue;
        if (!found) fputs("\n.data\n", this->outfile);
        found = true;
        MIPS_GlobSym(this, curr);
    }
}