#include "asm.h"

static char *reglist[MAX_REG] = {"$t0", "$t1", "$t2", "$t3", "$t4",
                                 "$t5", "$t6", "$t7", "$t8", "$t9",
                                 "$a0", "$a1", "$a2", "$a3"};

static int psize[] = {0, 0, 1, 4, 4, 4, 4};

static int allocReg(Compiler this);
static void freeReg(Compiler this, int reg1);

int PrimSize(enum ASTPRIM type) {
    if (type < P_NONE || type > P_INTPTR) {
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
    char *name = st->Gsym[ctx->functionId].name;

    int paramReg = FIRST_PARAM_REG;

    Compiler_ResetOffset(this);

    // 0 is $ra and 1 is $fp

    fprintf(this->outfile,
            ".text\n"
            "\t.globl %s\n"
            "\n"
            "%s:\n",
            name, name);

    printf("\n\nFunction: %s\n", name);

    int i;
    bool param = false;
    
    // another god damn temp fix until i find the source
    // of the problem
    for (i = MAX_SYMBOLS - 1; i > st->locls; i--) {
        if (st->Gsym[i].class != C_PARAM) break;

        if (paramReg++ > FIRST_PARAM_REG + 3) {
            break;
        }
        param = true;
        

        st->Gsym[i].offset = Compiler_GetParamOffset(this, st->Gsym[i].type);
    }

    // temp fix

    if (paramReg > FIRST_PARAM_REG + 3) {
        paramReg--;
    }

    paramReg -= FIRST_PARAM_REG;
    for (int j = paramReg - 1; j >= 0; j--) {
        printf("Pushing param reg %d\n", j + FIRST_PARAM_REG);
        MIPS_StoreParam(this, j + FIRST_PARAM_REG);
    }

    bool foundLocal = false;
    int startLocal = 0;
    
    // WTF WHY DOES THIS FUCKING WORK
    if (param) {
        this->paramOffset -= 4;
    }

    for (; i > st->locls; i--) {
        // for remaining params they get pushed on stack
        // This also includes local variables
        
        if (st->Gsym[i].class == C_LOCAL) {
            if (!foundLocal) {
                printf("found local\n");
                foundLocal = true;
                startLocal = i;
            }
            printf("Local variable %s\n", st->Gsym[i].name);
            st->Gsym[i].offset =
                this->paramOffset +
                Compiler_GetLocalOffset(this, st->Gsym[i].type);
        } else {
            st->Gsym[i].offset =
                Compiler_GetParamOffset(this, st->Gsym[i].type);
        }
    }

    // need to add local offset to all offsets somehow

    fprintf(this->outfile,
            "\tpush $ra\n"
            "\tBEGIN\n\n");

    // Actual offset for locals if have been initialised
    if (foundLocal) {
        fprintf(this->outfile, "\taddi\t$sp, $sp, %d\n",
                -(this->localOffset - 8));
        for (i = startLocal; i > st->locls; i--) {
            if (st->Gsym[i].class == C_LOCAL && st->Gsym[i].hasValue) {
                int r = MIPS_Load(this, st->Gsym[i].value);
                MIPS_StoreLocal(this, r, st, i);
                freeReg(this, r);
            }
        }
    }

    fputs("\n", this->outfile);
}

void MIPS_PostFunc(Compiler this, SymTable st, Context ctx) {
    MIPS_ReturnLabel(this, st, ctx);
    fprintf(this->outfile,
            "\tEND\n"
            "\tpop\t$ra\n"
            "\tjr\t$ra\n\n");
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

void MIPS_PrintStr(Compiler this, int r) {
    fprintf(this->outfile, "\tli\t$v0, 4\n");
    fprintf(this->outfile, "\tmove\t$a0, %s\n", reglist[r]);
    fprintf(this->outfile, "\tsyscall\n");
}

int MIPS_LoadGlobStr(Compiler this, SymTable st, int id) {
    int r = allocReg(this);
    fprintf(this->outfile, "\tla\t%s, %s\n", reglist[r], st->Gsym[id].name);
    return r;
}

int MIPS_LoadGlob(Compiler this, SymTable st, int id, enum ASTOP op) {
    int r = allocReg(this);
    int r2;
    switch (st->Gsym[id].type) {
        case P_INT:
            fprintf(this->outfile, "\tlw\t%s, %s\n", reglist[r],
                    st->Gsym[id].name);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r],
                        st->Gsym[id].name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r2],
                        st->Gsym[id].name);
                freeReg(this, r2);
            }

            break;
        case P_CHAR:
            fprintf(this->outfile, "\tlbu\t%s, %s\n", reglist[r], reglist[r]);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsb\t%s, %s\n", reglist[r],
                        st->Gsym[id].name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsb\t%s, %s\n", reglist[r2],
                        st->Gsym[id].name);
                freeReg(this, r2);
            }

            break;
        case P_CHARPTR:
        case P_INTPTR:
            fprintf(this->outfile, "\tlw\t%s, %s\n", reglist[r], reglist[r]);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r],
                        st->Gsym[id].name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %s\n", reglist[r2],
                        st->Gsym[id].name);
                freeReg(this, r2);
            }
            break;
        default:
            fprintf(stderr, "Error!: Unknown type %d\n", st->Gsym[id].type);
            exit(-1);
    }
    return r;
}

int MIPS_LoadLocal(Compiler this, SymTable st, int id, enum ASTOP op) {
    int r = allocReg(this);
    int r2;
    switch (st->Gsym[id].type) {
        case P_INT:
            fprintf(this->outfile, "\tlw\t%s, %d($sp)\n", reglist[r],
                    st->Gsym[id].offset);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, -%d($sp)\n", reglist[r],
                        st->Gsym[id].offset);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r2],
                        st->Gsym[id].offset);
                freeReg(this, r2);
            }

            break;
        case P_CHAR:
            fprintf(this->outfile, "\tlbu\t%s, %d($sp)\n", reglist[r],
                    st->Gsym[id].offset);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsb\t%s, %d($sp)\n", reglist[r],
                        st->Gsym[id].offset);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsb\t%s, %d($sp)\n", reglist[r2],
                        st->Gsym[id].offset);
                freeReg(this, r2);
            }

            break;
        case P_CHARPTR:
        case P_INTPTR:
            fprintf(this->outfile, "\tlw\t%s, %d($sp)\n", reglist[r],
                    st->Gsym[id].offset);

            if (op == A_PREINC || op == A_PREDEC) {
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r],
                        reglist[r], op == A_PREINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r],
                        st->Gsym[id].offset);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(this);
                fprintf(this->outfile, "\tmove\t%s, %s\n", reglist[r2],
                        reglist[r]);
                fprintf(this->outfile, "\taddi\t%s, %s, %s\n", reglist[r2],
                        reglist[r2], op == A_POSTINC ? "1" : "-1");
                fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r2],
                        st->Gsym[id].offset);
                freeReg(this, r2);
            }
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

void MIPS_StoreParam(Compiler this, int r1) {
    if (r1 == NO_REG) {
        fprintf(stderr, "Error: Trying to store an empty register\n");
        exit(-1);
    }

    fprintf(this->outfile, "\tpush\t%s\n", reglist[r1]);
}

int MIPS_StoreLocal(Compiler this, int r1, SymTable st, int id) {
    if (r1 == NO_REG) {
        fprintf(stderr, "Error: Trying to store an empty register\n");
        exit(-1);
    }

    switch (st->Gsym[id].type) {
        case P_INT:
            fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r1],
                    st->Gsym[id].offset);
            break;
        case P_CHAR:
            fprintf(this->outfile, "\tsb\t%s, %d($sp)\n", reglist[r1],
                    st->Gsym[id].offset);
            break;
        case P_CHARPTR:
        case P_INTPTR:
            fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r1],
                    st->Gsym[id].offset);
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
            fprintf(this->outfile, "\tsb\t%s, 0(%s)\n", reglist[r1],
                    reglist[r2]);
            break;
        case P_INT:
            fprintf(this->outfile, "\tsw\t%s, 0(%s)\n", reglist[r1],
                    reglist[r2]);
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
void MIPS_GlobSym(Compiler this, SymTable st, int id) {
    int typesize = PrimSize(st->Gsym[id].type);
    enum ASTPRIM type = st->Gsym[id].type;
    if (type == P_CHARPTR && st->Gsym[id].hasValue) {
        fprintf(this->outfile, "\t%s:\t.asciiz \"%s\"\n", st->Gsym[id].name,
                st->Gsym[id].strValue);
    } else {
        if (st->Gsym[id].hasValue) {
            if (type == P_CHAR) {
                fprintf(this->outfile, "\t%s:\t.byte %d\n", st->Gsym[id].name,
                        st->Gsym[id].value);
            } else {
                fprintf(this->outfile, "\t%s:\t.word %d\n", st->Gsym[id].name,
                        st->Gsym[id].value);
            }
        } else {
            fprintf(this->outfile, "\t%s:\t.space %d\n", st->Gsym[id].name,
                    typesize * st->Gsym[id].size);
        }
        switch (type) {
            case P_CHAR:
                fprintf(this->outfile, "\t.align 2\n");
                break;
            default:
                break;
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

void MIPS_ReturnLabel(Compiler this, SymTable st, Context ctx) {
    fprintf(this->outfile, "%s_end:\n", st->Gsym[ctx->functionId].name);
}

void MIPS_ReturnJump(Compiler this, SymTable st, Context ctx) {
    fprintf(this->outfile, "\tb\t%s_end\n", st->Gsym[ctx->functionId].name);
}

void MIPS_Return(Compiler this, SymTable st, int r, Context ctx) {
    if (st->Gsym[ctx->functionId].type == P_INT) {
        fprintf(this->outfile, "\tmove\t$v0, %s\n", reglist[r]);
    } else if (st->Gsym[ctx->functionId].type == P_CHAR) {
        fprintf(this->outfile, "\tmove\t$v0, %s\n", reglist[r]);
        // I dont think we need the below
        // fprintf(this->outfile, "\tandi\t$v0, %s, 0xFF\n", reglist[r]);
    } else {
        fprintf(stderr, "Error!!!: Unknown type %d\n",
                st->Gsym[ctx->functionId].type);
        exit(-1);
    }
    MIPS_ReturnJump(this, st, ctx);
}

int MIPS_Call(Compiler this, SymTable st, int id, int numArgs) {
    int outr = allocReg(this);
    fprintf(this->outfile, "\tjal\t%s\n", st->Gsym[id].name);
    fprintf(this->outfile, "\tmove\t%s, $v0\n", reglist[outr]);
    return outr;
}

void MIPS_ArgCopy(Compiler this, int r, int argPos, int maxArg) {
    /*
    Stack:
    (params)

    (return address)
    (old frame pointer)
    (local variables)
    */

    // Basically greater than 4 (+ 1 for index)
    if (argPos > 4) {
        // Look back at this - Idk if it works or not
        // int offset = (maxArg - 4) * 4 - ((maxArg - 4) - (argPos - 4)) * 4 -
        // 4;
        // fprintf(this->outfile, "\tsw\t%s, %d($sp)\n", reglist[r], -offset);

        fprintf(this->outfile, "\tpush\t%s\n", reglist[r]);
    } else {
        fprintf(this->outfile, "\tmove\t%s, %s\n",
                reglist[FIRST_PARAM_REG + argPos - 1], reglist[r]);
    }
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
            fprintf(this->outfile, "\tlbu\t%s, 0(%s)\n", reglist[r],
                    reglist[r]);
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
    bool found = false;

    for (int i = 0; i < st->globs; i++) {
        if (st->Gsym[i].stype == S_FUNC) continue;
        if (st->Gsym[i].class != C_GLOBAL) continue;
        if (!found) fputs("\n.data\n", this->outfile);
        found = true;
        MIPS_GlobSym(this, st, i);
    }
}