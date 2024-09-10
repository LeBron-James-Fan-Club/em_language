#include "asm.h"

#include <stdio.h>

#include "misc.h"

static char *reglist[MAX_REG] = {"$t0", "$t1", "$t2", "$t3", "$t4",
                                 "$t5", "$t6", "$t7", "$t8", "$t9",
                                 "$a0", "$a1", "$a2", "$a3"};

static void freeReg(Compiler this, int reg1);
static void Compiler_FreeAllStyleReg(Compiler this);

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
    /*fputs(
        ".text\n"
        "\t.globl main\n"
        "\n"
        "main:\n",
        this->outfile);*/
}

void MIPS_Post(Compiler this) {
    // switch shit
    // accepts two parameters
    // $a0 = register to compare
    // $a1 = base address of jump table

    if (this->sawSwitch)
        fputs(
            "\nswitch:\n"
            "\tmove\t$t0, $a0\n"
            // scan no of cases
            "\tlw\t$t1, 0($a1)\n"
            "\nnext:\n"
            "\taddi\t$a1, $a1, 4\n"
            "\tlw\t$t2, 0($a1)\n"
            // load label
            "\taddi\t$a1, $a1, 4\n"
            "\tbne\t$t0, $t2, fail\n"
            "\tlw\t$t3, 0($a1)\n"
            "\tjr\t$t3\n"
            "\nfail:\n"
            "\taddi\t$t1, $t1, -1\n"
            "\tbgtz\t$t1, next\n"
            "\taddi\t$a1, $a1, 4\n"
            // last is just label so scan that
            // and go to default
            "\tlw\t$t3, 0($a1)\n"
            "\tjr\t$t3\n",
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

/*
Structure of the stack
(Parameters after 4th param)
($fp) + 8
($ra)
(local variables)
*/

void MIPS_PreFunc(Compiler this, SymTable st, Context ctx) {
    char *name = ctx->functionId->name;

    int paramReg = FIRST_PARAM_REG;

    Compiler_ResetOffset(this);

    // 0 is $ra and 1 is $fp

    fputs(".text\n", this->outfile);
    if (ctx->functionId->class != C_STATIC) {
        fprintf(this->outfile, "\t.globl %s\n", name);
    }
    fprintf(this->outfile,
            "\n"
            "%s:\n",
            name);

    // ! STYLE HEADER IS CREATED HERE
    fputs("\n# Frame:\t", this->outfile);
    int skip = 0;
    for (SymTableEntry curr = ctx->functionId->member; curr != NULL;
         curr = curr->next) {
        if (skip++ < 4) {
            // skip if first four
            continue;
        }
        fprintf(this->outfile, "%s, ", curr->name);
    }

    fputs("$fp, $ra", this->outfile);

    if (st->loclHead != NULL) {
        fputs(", ", this->outfile);
    }

    for (SymTableEntry curr = st->loclHead; curr != NULL; curr = curr->next) {
        fprintf(this->outfile, "%s", curr->name);
        if (curr->next != NULL) {
            fputs(", ", this->outfile);
        }
    }

    fputs("\n# Uses:\t\t", this->outfile);

    if (skip > 4) skip = 4;

    debug("skip is %d", skip);

    for (int i = 0; i < skip; i++) {
        fprintf(this->outfile, "%s", reglist[FIRST_PARAM_REG + i]);
        if ((i + 1) < skip) {
            fputs(", ", this->outfile);
        }
    }

    fputs("\n# Clobbers:\t", this->outfile);

    for (int i = 0; i < skip; i++) {
        fprintf(this->outfile, "%s", reglist[FIRST_PARAM_REG + i]);
        if ((i + 1) < skip) {
            fputs(", ", this->outfile);
        }
    }

    // this->styleSeek = ftell(this->outfile);

    fputs("\n\n", this->outfile);

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

    SymTableEntry loclCurr = st->loclHead;

    for (; loclCurr != NULL; loclCurr = loclCurr->next) {
        // same thing but for local
        // We subtract 4 cause its 1 off
        // e.g. a[0] instead of a[1]
        loclCurr->posn = Compiler_GetLocalOffset(this, loclCurr->type) - 4;
    }

    for (; paramCurr != NULL; paramCurr = paramCurr->next) {
        // for remaining params they get pushed on stack
        paramCurr->posn =
            this->localOffset + Compiler_GetParamOffset(this, paramCurr->type);
    }

    // need to add local offset to all offsets somehow

    fputs("\n# Locals:\n", this->outfile);
    for (SymTableEntry curr = st->loclHead; curr != NULL; curr = curr->next) {
        fprintf(this->outfile, "#\t-`%s` in %d($sp)\n", curr->name, curr->posn);
    }

    fputs("\n\n", this->outfile);

    fprintf(this->outfile,
            "\tbegin\n\n"
            "\tpush\t$ra\n");

    // Actual offset for locals if have been initialised
    if (st->loclHead) {
        fprintf(this->outfile, "\taddiu\t$sp, $sp, %d\n", -this->localOffset);
    }

    fputs("\n", this->outfile);
}

void MIPS_PostFunc(Compiler this, Context ctx) {
    // TODO: if there are no early returns
    // TODO: we don't add return label
    MIPS_ReturnLabel(this, ctx);
    if (this->localOffset > 0) {
        fprintf(this->outfile, "\taddiu\t$sp, $sp, %d\n", this->localOffset);
    }
    fputs(
        "\tpop\t$ra\n"
        "\tend\n"
        "\tjr\t$ra\n\n",
        this->outfile);
    Compiler_FreeAllStyleReg(this);

    /*
    int toSeek = this->styleSeek;
    this->styleSeek = ftell(this->outfile);
    fseek(this->outfile, toSeek, SEEK_SET);

    int charactersAdded = 0;
    if (ctx->functionId->member) {
        fputs(", ", this->outfile);
        charactersAdded += 2;
    }

    for (int i = 0; i < MAX_REG; i++) {
        if (this->styleRegUsed[i]) {
            fprintf(this->outfile, "%s", reglist[i]);
            charactersAdded += 3;
        }
        if ((i + 1) < MAX_REG && this->styleRegUsed[i + 1]) {
            fputs(", ", this->outfile);
            charactersAdded += 2;
        }
    }
    fputs("\n", this->outfile);
    charactersAdded += 1;

    fseek(this->outfile, this->styleSeek + charactersAdded, SEEK_SET);
    */
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
    freeReg(this, r1);
    return r2;
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
                    sym->posn);
            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r],
                        sym->posn);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r2],
                        sym->posn);
                freeReg(this, r2);
            }

            break;
        case P_CHAR:
            fprintf(this->outfile, "\tlbu\t%s, %d($sp)\n", reglist[r],
                    sym->posn);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsb\t%s, %d($sp)\n", reglist[r],
                        sym->posn);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsb\t%s, %d($sp)\n", reglist[r2],
                        sym->posn);
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
                    sym->posn);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r],
                        sym->posn);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r2],
                        sym->posn);
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
                    sym->posn);
            break;
        case P_CHAR:
            fprintf(this->outfile, "\tsb\t%s, %d($sp)\n", reglist[r1],
                    sym->posn);
            break;
        default:
            if (!ptrtype(sym->type)) {
                debug("store local error");
                fatala("InternalError: Unknown type %d", sym->type);
            }

            fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r1],
                    sym->posn);
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

int MIPS_Align(enum ASTPRIM type, int offset, int dir) {
    int align;

    debug("type %d", type);

    switch (type) {
        case P_CHAR:
            return offset;
        case P_INT:
            break;
        default:
            if (!ptrtype(type)) {
                debug("align error");
                fatala("InternalError: Unknown type %d", type);
            }
    }

    align = 4;
    // aligns shit??
    offset = (offset + dir * (align - 1)) & ~(align - 1);
    debug("offset struct rn: %d", offset);
    return offset;
}

// Needs to be below .data
void MIPS_GlobSym(Compiler this, SymTableEntry sym) {
    int size;
    int initValue;
    enum ASTPRIM type;

    // ! For annoymous strings only
    if ((sym->type & P_CHAR) && ptrtype(sym->type) && sym->isStr) {
        fprintf(this->outfile, "%s:\n\t.asciiz \"%s\"\n", sym->name,
                sym->strValue);
        return;
    }

    if (sym->stype == S_ARRAY) {
        size = type_size(value_at(sym->type), sym->ctype);
        type = value_at(sym->type);
    } else {
        size = sym->size;
        type = sym->type;
    }

    // Array shit
    // * DEFAULT CASE IS FOR STRUCTS OR UNIONS

    fprintf(this->outfile, "%s:\n", sym->name);

    debug("allocating global variable %s nElems: %d", sym->name, sym->nElems);

    for (int i = 0; i < sym->nElems; i++) {
        initValue = 0;
        if (sym->initList != NULL) initValue = sym->initList[i];
        switch (size) {
            case 1:
                fprintf(this->outfile, "\t.byte %d\n", initValue);
                fputs("\t.align 2\n", this->outfile);
                break;
            case 4:
                // Quick fix but will have to figure out literal values later
                if (sym->initList != NULL && type == pointer_to(P_CHAR) &&
                    initValue != 0) {
                    fprintf(this->outfile, "\t.word anon_%d\n", initValue);
                } else {
                    fprintf(this->outfile, "\t.word %d\n", initValue);
                }

                break;
            default:
                // structs and unions baby
                fprintf(this->outfile, "\t.space %d\n", size);
        }
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

int MIPS_InputChar(Compiler this) {
    fputs(
        "\tli\t$v0, 12\n"
        "\tsyscall\n",
        this->outfile);
    int r = allocReg(this);
    fprintf(this->outfile, "\tmove\t%s, $v0\n", reglist[r]);
    return r;
}

void MIPS_InputString(Compiler this, char *name, int size) {
    fprintf(this->outfile,
            "\tla\t$a0, %s\n"
            "\tli\t$a1, %d\n"
            "\tli\t$v0, 8\n"
            "\tsyscall\n",
            name, size);
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
    Compiler_FreeAllReg(this, NO_REG);
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
    Compiler_FreeAllReg(this, NO_REG);
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
    Compiler_FreeAllReg(this, NO_REG);
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
    Compiler_FreeAllReg(this, NO_REG);
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
    Compiler_FreeAllReg(this, NO_REG);
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
    Compiler_FreeAllReg(this, NO_REG);
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

void MIPS_ArgCopy(Compiler this, int r, int argPos) {
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
        debug("argPos: %d", argPos);
        debug("calculation %d", FIRST_PARAM_REG + argPos - 1);
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

void MIPS_Move(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2], reglist[r1]);
    freeReg(this, r1);
}

int MIPS_Address(Compiler this, SymTableEntry sym) {
    int r = allocReg(this);
    if (sym->class == C_GLOBAL || sym->class == C_EXTERN ||
        sym->class == C_STATIC) {
        fprintf(this->outfile, "\tla\t%s, %s\n", reglist[r], sym->name);
    } else {
        fprintf(this->outfile, "\tla\t%s, %d($sp)\n", reglist[r], sym->posn);
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

int MIPS_LogOr(Compiler this, int r1, int r2) {
    int Ltrue = Compiler_GenLabel(this);
    int Lend = Compiler_GenLabel(this);

    fprintf(this->outfile, "\tbnez\t%s, L%d\n", reglist[r1], Ltrue);
    fprintf(this->outfile, "\tbnez\t%s, L%d\n", reglist[r2], Ltrue);

    fprintf(this->outfile, "\tli\t%s, 0\n", reglist[r1]);
    fprintf(this->outfile, "\tb\tL%d\n", Lend);

    MIPS_Label(this, Ltrue);
    fprintf(this->outfile, "\tli\t%s, 1\n", reglist[r1]);
    MIPS_Label(this, Lend);

    freeReg(this, r2);
    return r1;
}

int MIPS_LogAnd(Compiler this, int r1, int r2) {
    int Lfalse = Compiler_GenLabel(this);
    int Lend = Compiler_GenLabel(this);

    fprintf(this->outfile, "\tbeqz\t%s, L%d\n", reglist[r1], Lfalse);
    fprintf(this->outfile, "\tbeqz\t%s, L%d\n", reglist[r2], Lfalse);

    fprintf(this->outfile, "\tli\t%s, 1\n", reglist[r1]);
    fprintf(this->outfile, "\tb\tL%d\n", Lend);

    MIPS_Label(this, Lfalse);
    fprintf(this->outfile, "\tli\t%s, 0\n", reglist[r1]);
    MIPS_Label(this, Lend);
    freeReg(this, r2);
    return r1;
}

void MIPS_Poke(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tsw\t%s, 0(%s)\n", reglist[r1], reglist[r2]);
}

int MIPS_Peek(Compiler this, int r1, int r2) {
    fprintf(this->outfile, "\tlw\t%s, 0(%s)\n", reglist[r1], reglist[r2]);
    freeReg(this, r2);
    return r1;
}

void MIPS_Switch(Compiler this, int r, int caseCount, int topLabel,
                 int *caseLabel, int *caseVal, int defaultLabel) {
    int label = Compiler_GenLabel(this);
    MIPS_Label(this, label);

    if (caseCount == 0) {
        caseVal[0] = 0;
        caseLabel[0] = defaultLabel;
        caseCount = 1;
    }
    fprintf(this->outfile, "\t.word\t%d\n", caseCount);
    for (int i = 0; i < caseCount; i++) {
        fprintf(this->outfile, "\t.word\t%d, L%d\n", caseVal[i], caseLabel[i]);
    }
    fprintf(this->outfile, "\t.word\tL%d\n", defaultLabel);
    MIPS_Label(this, topLabel);

    if (this->paramRegCount > 0) {
        MIPS_RegPush(this, FIRST_PARAM_REG);
    }
    if (this->paramRegCount > 1) {
        MIPS_RegPush(this, FIRST_PARAM_REG + 1);
    }

    fprintf(this->outfile, "\tmove\t$a0, %s\n", reglist[r]);
    fprintf(this->outfile, "\tla\t$a1, L%d\n", label);

    fputs("\tjal\tswitch\n", this->outfile);

    if (this->paramRegCount > 0) {
        MIPS_RegPop(this, FIRST_PARAM_REG);
    }
    if (this->paramRegCount > 1) {
        MIPS_RegPop(this, FIRST_PARAM_REG + 1);
    }
}

int allocReg(Compiler this) {
    for (int i = 0; i < MAX_REG; i++) {
        if (!this->regUsed[i]) {
            this->styleRegUsed[i] = true;
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

int Compiler_GenLabel(Compiler this) { return this->label++; }

void Compiler_FreeAllReg(Compiler this, int keepReg) {
    for (int i = 0; i < MAX_REG; i++) {
        if (i != keepReg) this->regUsed[i] = false;
    }
}

static void Compiler_FreeAllStyleReg(Compiler this) {
    for (int i = 0; i < MAX_REG; i++) {
        this->styleRegUsed[i] = false;
    }
}

void Compiler_GenData(Compiler this, SymTable st) {
    bool found = false;
    debug("Generating globs");

    // Generate annyomous strings
    for (SymTableEntry curr = st->globHead; curr != NULL; curr = curr->next) {
        if (curr->isStr) {
            MIPS_GlobSym(this, curr);
        }
        if (!found) fputs("\n.data\n", this->outfile);
        found = true;
    }

    for (SymTableEntry curr = st->globHead; curr != NULL; curr = curr->next) {
        // TODO: Remove these lines below (2)
        // TODO: and see if it fucks up anything

        if (curr->stype == S_FUNC) continue;
        if (curr->class != C_GLOBAL) continue;
        if (curr->isStr) continue;

        if (!found) fputs("\n.data\n", this->outfile);
        found = true;
        MIPS_GlobSym(this, curr);
    }
}