#include "asm.h"
#include "misc.h"

static char *reglist[MAX_REG] = {"$t0", "$t1", "$t2", "$t3", "$t4",
                                 "$t5", "$t6", "$t7", "$t8", "$t9",
                                 "$a0", "$a1", "$a2", "$a3"};

static void freeReg(Compiler self, int reg1);
static void Compiler_FreeAllStyleReg(Compiler self);

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

void MIPS_Pre(Compiler self) {
    /*fputs(
        ".text\n"
        "\t.globl main\n"
        "\n"
        "main:\n",
        self->outfile);*/

    // switch shit
    // TODO: only put self in if there is a switch
    // TODO: (might be a tad bit weird tho ngl)

    // accepts two parameters
    // $a0 = register to compare
    // $a1 = base address of jump table
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
        self->outfile);
}

void MIPS_Post(Compiler self) {
    Compiler_FreeAllStyleReg(self);
    /*fputs(
        "\tli\t$v0, 10\n"
        "\tsyscall\n",
        self->outfile);*/
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

void MIPS_PreFunc(Compiler self, SymTable st, Context ctx) {
    char *name = ctx->functionId->name;

    int paramReg = FIRST_PARAM_REG;

    Compiler_ResetOffset(self);

    // 0 is $ra and 1 is $fp

    fputs(".text\n", self->outfile);
    if (ctx->functionId->_class != C_STATIC) {
        fprintf(self->outfile, "\t.globl %s\n", name);
    }
    fprintf(self->outfile,
            "\n"
            "%s:\n",
            name);

    // ! STYLE HEADER IS CREATED HERE
    fputs("\n# Frame:\t", self->outfile);
    int skip = 0;
    for (SymTableEntry curr = ctx->functionId->member; curr != nullptr;
         curr = curr->next) {
        if (skip++ < 4) {
            // skip if first four
            continue;
        }
        fprintf(self->outfile, "%s, ", curr->name);
    }

    fputs("$fp, $ra", self->outfile);

    if (st->loclHead != nullptr) {
        fputs(", ", self->outfile);
    }

    for (SymTableEntry curr = st->loclHead; curr != nullptr; curr = curr->next) {
        fprintf(self->outfile, "%s", curr->name);
        if (curr->next != nullptr) {
            fputs(", ", self->outfile);
        }
    }

    fputs("\n# Uses:\t\t", self->outfile);

    if (skip > 4) skip = 4;

    debug("skip is %d", skip);

    for (int i = 0; i < skip; i++) {
        fprintf(self->outfile, "%s", reglist[FIRST_PARAM_REG + i]);
        if ((i + 1) < skip) {
            fputs(", ", self->outfile);
        }
    }

    fputs("\n# Clobbers:\t", self->outfile);

    for (int i = 0; i < skip; i++) {
        fprintf(self->outfile, "%s", reglist[FIRST_PARAM_REG + i]);
        if ((i + 1) < skip) {
            fputs(", ", self->outfile);
        }
    }

    // self->styleSeek = ftell(self->outfile);

    fputs("\n\n", self->outfile);

    debug("Function: %s", name);

    SymTableEntry paramCurr = ctx->functionId->member;

    for (; paramCurr != nullptr; paramCurr = paramCurr->next) {
        if (paramReg > FIRST_PARAM_REG + 3) {
            break;
        }

        paramCurr->isFirstFour = true;
        debug("paramReg: %d", paramReg);
        paramCurr->paramReg = paramReg++;
        self->paramRegCount++;
    }

    SymTableEntry loclCurr = st->loclHead;

    for (; loclCurr != nullptr; loclCurr = loclCurr->next) {
        // same thing but for local
        // We subtract 4 cause its 1 off
        // e.g. a[0] instead of a[1]
        loclCurr->posn = Compiler_GetLocalOffset(self, loclCurr->type) - 4;
    }

    for (; paramCurr != nullptr; paramCurr = paramCurr->next) {
        // for remaining params they get pushed on stack
        paramCurr->posn =
            self->localOffset + Compiler_GetParamOffset(self, paramCurr->type);
    }

    // need to add local offset to all offsets somehow

    fputs("\n# Locals:\n", self->outfile);
    for (SymTableEntry curr = st->loclHead; curr != nullptr; curr = curr->next) {
        fprintf(self->outfile, "#\t-`%s` in %d($sp)\n", curr->name, curr->posn);
    }

    fputs("\n\n", self->outfile);

    fprintf(self->outfile,
            "\tbegin\n\n"
            "\tpush\t$ra\n");

    // Actual offset for locals if have been initialised
    if (st->loclHead) {
        fprintf(self->outfile, "\taddiu\t$sp, $sp, %d\n", -self->localOffset);
    }

    fputs("\n", self->outfile);
}

void MIPS_PostFunc(Compiler self, Context ctx) {
    // TODO: if there are no early returns
    // TODO: we don't add return label
    MIPS_ReturnLabel(self, ctx);
    if (self->localOffset > 0) {
        fprintf(self->outfile, "\taddiu\t$sp, $sp, %d\n", self->localOffset);
    }
    fputs(
        "\tpop\t$ra\n"
        "\tend\n"
        "\tjr\t$ra\n\n",
        self->outfile);

    /*
    int toSeek = self->styleSeek;
    self->styleSeek = ftell(self->outfile);
    fseek(self->outfile, toSeek, SEEK_SET);

    int charactersAdded = 0;
    if (ctx->functionId->member) {
        fputs(", ", self->outfile);
        charactersAdded += 2;
    }

    for (int i = 0; i < MAX_REG; i++) {
        if (self->styleRegUsed[i]) {
            fprintf(self->outfile, "%s", reglist[i]);
            charactersAdded += 3;
        }
        if ((i + 1) < MAX_REG && self->styleRegUsed[i + 1]) {
            fputs(", ", self->outfile);
            charactersAdded += 2;
        }
    }
    fputs("\n", self->outfile);
    charactersAdded += 1;

    fseek(self->outfile, self->styleSeek + charactersAdded, SEEK_SET);
    */
}

int MIPS_Load(Compiler self, int value) {
    int r = allocReg(self);
    fprintf(self->outfile, "\tli\t%s, %d\n", reglist[r], value);
    return r;
}

int MIPS_Add(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tadd\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_Mul(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tmul\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_Sub(Compiler self, int r1, int r2) {
    // not communative
    fprintf(self->outfile, "\tsub\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_Div(Compiler self, int r1, int r2) {
    // also not communative
    fprintf(self->outfile, "\tdiv\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_Mod(Compiler self, int r1, int r2) {
    // not communative
    fprintf(self->outfile, "\tmod\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

void MIPS_PrintInt(Compiler self, int r) {
    fprintf(self->outfile, "\tli\t$v0, 1\n");
    fprintf(self->outfile, "\tmove\t$a0, %s\n", reglist[r]);
    fprintf(self->outfile, "\tsyscall\n");
}

void MIPS_PrintChar(Compiler self, int r) {
    fprintf(self->outfile, "\tli\t$v0, 11\n");
    fprintf(self->outfile, "\tmove\t$a0, %s\n", reglist[r]);
    fprintf(self->outfile, "\tsyscall\n");
}

void MIPS_PrintStr(Compiler self, int r) {
    fprintf(self->outfile, "\tli\t$v0, 4\n");
    fprintf(self->outfile, "\tmove\t$a0, %s\n", reglist[r]);
    fprintf(self->outfile, "\tsyscall\n");
}

int MIPS_LoadGlobStr(Compiler self, SymTableEntry sym) {
    int r = allocReg(self);
    fprintf(self->outfile, "\tla\t%s, %s\n", reglist[r], sym->name);
    return r;
}

int MIPS_LoadGlob(Compiler self, SymTableEntry sym, enum ASTOP op) {
    int r = allocReg(self);
    int r2;
    switch (sym->type) {
        case P_INT:
            fprintf(self->outfile, "\tlw\t%s, %s\n", reglist[r], sym->name);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(self->outfile, "\tsw\t%s, %s\n", reglist[r], sym->name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(self->outfile, "\tsw\t%s, %s\n", reglist[r2],
                        sym->name);
                freeReg(self, r2);
            }

            break;
        case P_CHAR:
            fprintf(self->outfile, "\tlbu\t%s, %s\n", reglist[r], sym->name);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(self->outfile, "\tsb\t%s, %s\n", reglist[r], sym->name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(self->outfile, "\tsb\t%s, %s\n", reglist[r2],
                        sym->name);
                freeReg(self, r2);
            }

            break;
        // case P_CHARPTR:
        // case P_INTPTR:
        default:
            fprintf(self->outfile, "\tlw\t%s, %s\n", reglist[r], sym->name);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(self->outfile, "\tsw\t%s, %s\n", reglist[r], sym->name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(self->outfile, "\tsw\t%s, %s\n", reglist[r2],
                        sym->name);
                freeReg(self, r2);
            }
            break;
            // default:
            //     debug("load glob error");
            //    fatala("InternalError: Unknown type %d", sym->type);
    }
    return r;
}

int MIPS_LoadLocal(Compiler self, SymTableEntry sym, enum ASTOP op) {
    int r = allocReg(self);
    int r2;

    if (sym->isFirstFour) {
        debug("paramReg: %d", sym->paramReg);
        fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r],
                reglist[sym->paramReg]);

        if (op == A_PREINC || op == A_PREDEC) {
            fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                    reglist[r], op == A_PREINC ? "1" : "-1");
            fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r],
                    reglist[sym->paramReg]);
        }

        if (op == A_POSTINC || op == A_POSTDEC) {
            r2 = allocReg(self);
            fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r2], reglist[r]);
            fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                    reglist[r2], op == A_POSTINC ? "1" : "-1");
            fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r],
                    reglist[sym->paramReg]);
            freeReg(self, r2);
        }
        return r;
    }

    switch (sym->type) {
        case P_INT:
            fprintf(self->outfile, "\tlw\t%s, %d($sp)\n", reglist[r],
                    sym->posn);
            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(self->outfile, "\tsw\t%s, %d($sp)\n", reglist[r],
                        sym->posn);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(self->outfile, "\tsw\t%s, %d($sp)\n", reglist[r2],
                        sym->posn);
                freeReg(self, r2);
            }

            break;
        case P_CHAR:
            fprintf(self->outfile, "\tlbu\t%s, %d($sp)\n", reglist[r],
                    sym->posn);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(self->outfile, "\tsb\t%s, %d($sp)\n", reglist[r],
                        sym->posn);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(self->outfile, "\tsb\t%s, %d($sp)\n", reglist[r2],
                        sym->posn);
                freeReg(self, r2);
            }

            break;
        // case P_CHARPTR:
        // case P_INTPTR:
        default:
            if (!ptrtype(sym->type)) {
                debug("load local error");
                fatala("InternalError: Unknown type %d", sym->type);
            }
            fprintf(self->outfile, "\tlw\t%s, %d($sp)\n", reglist[r],
                    sym->posn);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(self->outfile, "\tsw\t%s, %d($sp)\n", reglist[r],
                        sym->posn);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(self->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(self->outfile, "\tsw\t%s, %d($sp)\n", reglist[r2],
                        sym->posn);
                freeReg(self, r2);
            }
            break;
    }
    return r;
}

int MIPS_StoreGlob(Compiler self, int r1, SymTableEntry sym) {
    if (r1 == NO_REG) {
        fatal("InternalError: Trying to store an empty register");
    }

    switch (sym->type) {
        case P_INT:
            fprintf(self->outfile, "\tsw\t%s, %s\n", reglist[r1], sym->name);
            break;
        case P_CHAR:
            fprintf(self->outfile, "\tsb\t%s, %s\n", reglist[r1], sym->name);
            break;
        default:
            if (!ptrtype(sym->type)) {
                debug("store glob error");
                fatala("InternalError: Unknown type %d", sym->type);
            }

            fprintf(self->outfile, "\tsw\t%s, %s\n", reglist[r1], sym->name);
            break;
    }

    return r1;
}

void MIPS_StoreParam(Compiler self, int r1) {
    if (r1 == NO_REG) {
        fatal("InternalError: Trying to store an empty register");
    }

    fprintf(self->outfile, "\tpush\t%s\n", reglist[r1]);
}

int MIPS_StoreLocal(Compiler self, int r1, SymTableEntry sym) {
    if (r1 == NO_REG) {
        fatal("InternalError: Trying to store an empty register");
    }

    if (sym->isFirstFour) {
        fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[sym->paramReg],
                reglist[r1]);
        return r1;
    }

    switch (sym->type) {
        case P_INT:
            fprintf(self->outfile, "\tsw\t%s, %d($sp)\n", reglist[r1],
                    sym->posn);
            break;
        case P_CHAR:
            fprintf(self->outfile, "\tsb\t%s, %d($sp)\n", reglist[r1],
                    sym->posn);
            break;
        default:
            if (!ptrtype(sym->type)) {
                debug("store local error");
                fatala("InternalError: Unknown type %d", sym->type);
            }

            fprintf(self->outfile, "\tsw\t%s, %d($sp)\n", reglist[r1],
                    sym->posn);
            break;
    }

    return r1;
}

int MIPS_StoreRef(Compiler self, int r1, int r2, enum ASTPRIM type) {
    if (r1 == NO_REG) {
        fatal("InternalError: Trying to store an empty register");
    }

    if (r2 == NO_REG) {
        fatal("InternalError: Trying to store an empty register for r2");
    }

    int size = PrimSize(type);

    switch (size) {
        case 1:
            fprintf(self->outfile, "\tsb\t%s, 0(%s)\n", reglist[r1],
                    reglist[r2]);
            break;
        case 4:
            fprintf(self->outfile, "\tsw\t%s, 0(%s)\n", reglist[r1],
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
void MIPS_GlobSym(Compiler self, SymTableEntry sym) {
    int size;
    int initValue;
    enum ASTPRIM type;

    // ! For annoymous strings only
    if ((sym->type & P_CHAR) && ptrtype(sym->type) && sym->isStr) {
        fprintf(self->outfile, "%s:\n\t.asciiz \"%s\"\n", sym->name,
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

    fprintf(self->outfile, "%s:\n", sym->name);

    debug("allocating global variable %s nElems: %d", sym->name, sym->nElems);

    for (int i = 0; i < sym->nElems; i++) {
        initValue = 0;
        if (sym->initList != nullptr) initValue = sym->initList[i];
        switch (size) {
            case 1:
                fprintf(self->outfile, "\t.byte %d\n", initValue);
                fputs("\t.align 2\n", self->outfile);
                break;
            case 4:
                // Quick fix but will have to figure out literal values later
                if (sym->initList != nullptr && type == pointer_to(P_CHAR) &&
                    initValue != 0) {
                    fprintf(self->outfile, "\t.word anon_%d\n", initValue);
                } else {
                    fprintf(self->outfile, "\t.word %d\n", initValue);
                }

                break;
            default:
                // structs and unions baby
                fprintf(self->outfile, "\t.space %d\n", size);
        }
    }
}

int MIPS_InputInt(Compiler self) {
    fputs(
        "\tli\t$v0, 5\n"
        "\tsyscall\n",
        self->outfile);

    // Call store glob later on
    // fprintf(self->outfile, "\tsw\t$v0, %s\n", g->Gsym[id].name);
    int r = allocReg(self);
    fprintf(self->outfile, "\tmove\t%s, $v0\n", reglist[r]);
    return r;
}

int MIPS_InputChar(Compiler self) {
    fputs(
        "\tli\t$v0, 12\n"
        "\tsyscall\n",
        self->outfile);
    int r = allocReg(self);
    fprintf(self->outfile, "\tmove\t%s, $v0\n", reglist[r]);
    return r;
}

void MIPS_InputString(Compiler self, char *name, int size) {
    fprintf(self->outfile,
            "\tla\t$a0, %s\n"
            "\tli\t$a1, %d\n"
            "\tli\t$v0, 8\n"
            "\tsyscall\n", name, size);
}

int MIPS_EqualSet(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tseq\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

// ALl the jumps are reversed -> eq -> neq

int MIPS_EqualJump(Compiler self, int r1, int r2, int l) {
    fprintf(self->outfile, "\tbne\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

int MIPS_NotEqualSet(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tsne\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_NotEqualJump(Compiler self, int r1, int r2, int l) {
    fprintf(self->outfile, "\tbeq\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

int MIPS_LessThanSet(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tslt\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_LessThanJump(Compiler self, int r1, int r2, int l) {
    fprintf(self->outfile, "\tbge\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

int MIPS_GreaterThanSet(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tsgt\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_GreaterThanJump(Compiler self, int r1, int r2, int l) {
    fprintf(self->outfile, "\tble\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

int MIPS_LessThanEqualSet(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tsle\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_LessThanEqualJump(Compiler self, int r1, int r2, int l) {
    fprintf(self->outfile, "\tbgt\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

int MIPS_GreaterThanEqualSet(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tsge\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_GreaterThanEqualJump(Compiler self, int r1, int r2, int l) {
    fprintf(self->outfile, "\tblt\t%s, %s, L%d\n", reglist[r1], reglist[r2], l);
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

void MIPS_Label(Compiler self, int l) { fprintf(self->outfile, "L%d:\n", l); }

void MIPS_Jump(Compiler self, int l) {
    fprintf(self->outfile, "\tb\tL%d\n", l);
}

void MIPS_GotoLabel(Compiler self, SymTableEntry sym) {
    fprintf(self->outfile, "%s:\n", sym->name);
}

void MIPS_GotoJump(Compiler self, SymTableEntry sym) {
    fprintf(self->outfile, "\tb\t%s\n", sym->name);
}

void MIPS_ReturnLabel(Compiler self, Context ctx) {
    fprintf(self->outfile, "%s_end:\n", ctx->functionId->name);
}

void MIPS_ReturnJump(Compiler self, Context ctx) {
    fprintf(self->outfile, "\tb\t%s_end\n", ctx->functionId->name);
}

void MIPS_Return(Compiler self, int r, Context ctx) {
    if (ctx->functionId->type == P_INT) {
        fprintf(self->outfile, "\tmove\t$v0, %s\n", reglist[r]);
    } else if (ctx->functionId->type == P_CHAR) {
        fprintf(self->outfile, "\tmove\t$v0, %s\n", reglist[r]);
        // I dont think we need the below
        // fprintf(self->outfile, "\tandi\t$v0, %s, 0xFF\n", reglist[r]);
    } else {
        fatala("InternalError: Unknown type %d", ctx->functionId->type);
    }
    MIPS_ReturnJump(self, ctx);
}

int MIPS_Call(Compiler self, SymTableEntry sym) {
    int outr = allocReg(self);
    fprintf(self->outfile, "\tjal\t%s\n", sym->name);
    fprintf(self->outfile, "\tmove\t%s, $v0\n", reglist[outr]);
    // Calculates the length of parameters
    int offset = 0;

    // If $a0-a3 are used
    for (int i = 0; i < self->paramRegCount; i++) {
        fprintf(self->outfile, "\tpush\t%s\n", reglist[FIRST_PARAM_REG + i]);
    }

    for (SymTableEntry curr = sym->member; curr != nullptr; curr = curr->next) {
        offset += PrimSize(curr->type);
    }
    if (offset > 16)
        fprintf(self->outfile, "\taddiu\t$sp, $sp, %d\n", offset - 16);

    for (int i = 0; i < self->paramRegCount; i++) {
        fprintf(self->outfile, "\tpop\t%s\n", reglist[FIRST_PARAM_REG + i]);
    }

    offset = 0;
    return outr;
}

void MIPS_ArgCopy(Compiler self, int r, int argPos) {
    /*
    Stack:
    (params)

    (old frame pointer)
    (return address)
    (local variables)
    */

    // Basically greater than 4 (+ 1 for index)
    if (argPos > 4) {
        fprintf(self->outfile, "\tpush\t%s\n", reglist[r]);
    } else {
        debug("argPos: %d", argPos);
        debug("calculation %d", FIRST_PARAM_REG + argPos - 1);
        fprintf(self->outfile, "\tmove\t%s, %s\n",
                reglist[FIRST_PARAM_REG + argPos - 1], reglist[r]);
    }
}

void MIPS_RegPush(Compiler self, int r) {
    fprintf(self->outfile, "\tpush\t%s\n", reglist[r]);
}

void MIPS_RegPop(Compiler self, int r) {
    fprintf(self->outfile, "\tpop\t%s\n", reglist[r]);
}

void MIPS_Move(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tmove\t%s, %s\n", reglist[r2], reglist[r1]);
    freeReg(self, r1);
}

int MIPS_Address(Compiler self, SymTableEntry sym) {
    int r = allocReg(self);
    if (sym->_class == C_GLOBAL || sym->_class == C_EXTERN ||
        sym->_class == C_STATIC) {
        fprintf(self->outfile, "\tla\t%s, %s\n", reglist[r], sym->name);
    } else {
        fprintf(self->outfile, "\tla\t%s, %d($sp)\n", reglist[r], sym->posn);
    }
    return r;
}

int MIPS_Deref(Compiler self, int r, enum ASTPRIM type) {
    //! bug: derefing not occuring for b[3]

    enum ASTPRIM newType = value_at(type);
    int size = PrimSize(newType);

    switch (size) {
        case 1:
            fprintf(self->outfile, "\tlbu\t%s, 0(%s)\n", reglist[r],
                    reglist[r]);
            break;
        case 4:
            fprintf(self->outfile, "\tlw\t%s, 0(%s)\n", reglist[r], reglist[r]);
            break;
        default:
            fatala("InternalError: Unknown pointer type %d", type);
    }
    return r;
}

int MIPS_ShiftLeftConstant(Compiler self, int r, int c) {
    fprintf(self->outfile, "\tsll\t%s, %s, %d\n", reglist[r], reglist[r], c);
    return r;
}

int MIPS_ShiftLeft(Compiler self, int r1, int r2) {
    // swap values if it doesnt work
    fprintf(self->outfile, "\tsllv\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_ShiftRight(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tsrav\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_Negate(Compiler self, int r) {
    fprintf(self->outfile, "\tneg\t%s, %s\n", reglist[r], reglist[r]);
    return r;
}

int MIPS_BitNOT(Compiler self, int r) {
    fprintf(self->outfile, "\tnot\t%s, %s\n", reglist[r], reglist[r]);
    return r;
}

int MIPS_LogNOT(Compiler self, int r) {
    fprintf(self->outfile, "\tnor\t%s, %s, %s\n", reglist[r], reglist[r],
            reglist[r]);
    return r;
}

int MIPS_BitAND(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tand\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_BitOR(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tor\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_BitXOR(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\txor\t%s, %s, %s\n", reglist[r2], reglist[r1],
            reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_ToBool(Compiler self, enum ASTOP parentOp, int r, int label) {
    if (parentOp == A_WHILE || parentOp == A_IF) {
        // fake instruction
        // used to be bltu - but for some reason it never works
        fprintf(self->outfile, "\tbeq\t%s, $zero, L%d\n", reglist[r], label);
        return r;
    } else {
        fprintf(self->outfile, "\tseq\t%s, %s, $zero\n", reglist[r],
                reglist[r]);
    }
    return r;
}

int MIPS_LogOr(Compiler self, int r1, int r2) {
    int Ltrue = Compiler_GenLabel(self);
    int Lend = Compiler_GenLabel(self);

    fprintf(self->outfile, "\tbnez\t%s, L%d\n", reglist[r1], Ltrue);
    fprintf(self->outfile, "\tbnez\t%s, L%d\n", reglist[r2], Ltrue);

    fprintf(self->outfile, "\tli\t%s, 0\n", reglist[r1]);
    fprintf(self->outfile, "\tb\tL%d\n", Lend);

    MIPS_Label(self, Ltrue);
    fprintf(self->outfile, "\tli\t%s, 1\n", reglist[r1]);
    MIPS_Label(self, Lend);

    freeReg(self, r2);
    return r1;
}

int MIPS_LogAnd(Compiler self, int r1, int r2) {
    int Lfalse = Compiler_GenLabel(self);
    int Lend = Compiler_GenLabel(self);

    fprintf(self->outfile, "\tbeqz\t%s, L%d\n", reglist[r1], Lfalse);
    fprintf(self->outfile, "\tbeqz\t%s, L%d\n", reglist[r2], Lfalse);

    fprintf(self->outfile, "\tli\t%s, 1\n", reglist[r1]);
    fprintf(self->outfile, "\tb\tL%d\n", Lend);

    MIPS_Label(self, Lfalse);
    fprintf(self->outfile, "\tli\t%s, 0\n", reglist[r1]);
    MIPS_Label(self, Lend);
    freeReg(self, r2);
    return r1;
}

void MIPS_Poke(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tsw\t%s, 0(%s)\n", reglist[r1], reglist[r2]);
}

int MIPS_Peek(Compiler self, int r1, int r2) {
    fprintf(self->outfile, "\tlw\t%s, 0(%s)\n", reglist[r1], reglist[r2]);
    freeReg(self, r2);
    return r1;
}

void MIPS_Switch(Compiler self, int r, int caseCount, int topLabel,
                 int *caseLabel, int *caseVal, int defaultLabel) {
    int label = Compiler_GenLabel(self);
    MIPS_Label(self, label);

    if (caseCount == 0) {
        caseVal[0] = 0;
        caseLabel[0] = defaultLabel;
        caseCount = 1;
    }
    fprintf(self->outfile, "\t.word\t%d\n", caseCount);
    for (int i = 0; i < caseCount; i++) {
        fprintf(self->outfile, "\t.word\t%d, L%d\n", caseVal[i], caseLabel[i]);
    }
    fprintf(self->outfile, "\t.word\tL%d\n", defaultLabel);
    MIPS_Label(self, topLabel);

    // TODO: PUSH $a0, $a1 to stack if it is used
    fprintf(self->outfile, "\tmove\t$a0, %s\n", reglist[r]);
    fprintf(self->outfile, "\tla\t$a1, L%d\n", label);
    // fprintf(self->outfile, "\tla\t$v1, L%d\n", label);

    fputs("\tjal\tswitch\n", self->outfile);
}

int allocReg(Compiler self) {
    for (int i = 0; i < MAX_REG; i++) {
        if (!self->regUsed[i]) {
            self->styleRegUsed[i] = true;
            self->regUsed[i] = true;
            return i;
        }
    }
    fatal("InternalError: Out of registers");
}

static void freeReg(Compiler self, int reg1) {
    if (!self->regUsed[reg1]) {
        fatal("InternalError: Trying to free a free register");
    }
    self->regUsed[reg1] = false;
}

int Compiler_GenLabel(Compiler self) { return self->label++; }

void Compiler_FreeAllReg(Compiler self, int keepReg) {
    for (int i = 0; i < MAX_REG; i++) {
        if (i != keepReg) self->regUsed[i] = false;
    }
}

static void Compiler_FreeAllStyleReg(Compiler self) {
    for (int i = 0; i < MAX_REG; i++) {
        self->styleRegUsed[i] = false;
    }
}

void Compiler_GenData(Compiler self, SymTable st) {
    bool found = false;
    debug("Generating globs");

    // Generate annyomous strings
    for (SymTableEntry curr = st->globHead; curr != nullptr; curr = curr->next) {
        if (curr->isStr) {
            MIPS_GlobSym(self, curr);
        }
        if (!found) fputs("\n.data\n", self->outfile);
        found = true;
    }

    for (SymTableEntry curr = st->globHead; curr != nullptr; curr = curr->next) {
        // TODO: Remove these lines below (2)
        // TODO: and see if it fucks up anything

        if (curr->stype == S_FUNC) continue;
        if (curr->_class != C_GLOBAL) continue;
        if (curr->isStr) continue;

        if (!found) fputs("\n.data\n", self->outfile);
        found = true;
        MIPS_GlobSym(self, curr);
    }
}