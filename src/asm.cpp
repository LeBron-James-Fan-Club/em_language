#include <string>
#include "asm.h"
#include "misc.h"

static const char *REGISTER_NAMES[] = {
        "$zero", "$at",
        "$v0", "$v1",
        "$a0", "$a1", "$a2", "$a3",
        "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
        "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
        "$t8", "$t9",
        "$k0", "$k1",
        "$gp", "$sp", "$fp", "$ra",
};

static Register reglist[MAX_REG] = {T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, A0, A1, A2, A3};

static void freeReg(Compiler self, int reg1);

static void Compiler_FreeAllStyleReg(Compiler self);

template<typename T>
void print_arg(FILE *file, bool first, const T &arg);

template<>
void print_arg(FILE *file, bool first, const char *const &arg) {
    if (first) {
        fprintf(file, "\t");
    } else {
        fprintf(file, ", ");
    }

    fprintf(file, "%s", arg);
}

template<>
void print_arg(FILE *file, bool first, char *const &arg) {
    print_arg(file, first, static_cast<const char *const &>(arg));
}

template<>
void print_arg(FILE *file, bool first, const std::string &arg) {
    print_arg(file, first, arg.c_str());
}

template<>
void print_arg(FILE *file, bool first, const int &arg) {
    if (first) {
        fprintf(file, "\t");
    } else {
        fprintf(file, ", ");
    }

    fprintf(file, "%d", arg);
}

template<>
void print_arg(FILE *file, bool first, const Register &arg) {
    print_arg(file, first, REGISTER_NAMES[arg]);
}

// Base case: No additional arguments
static void write_mnemonic(Compiler self, const char *mnemonic) {
    fprintf(self->outfile, "\t%s\n", mnemonic);
}

// Variadic template: Handle one or more arguments
template<typename First, typename... Rest>
static void write_mnemonic(Compiler self, const char *mnemonic, First first_arg, Rest... rest_args) {
    // Print the mnemonic first
    fprintf(self->outfile, "\t%s", mnemonic);

    // Print the first argument
    print_arg(self->outfile, true, first_arg);

    // Print the rest of the arguments (separated by commas)
    ((print_arg(self->outfile, false, rest_args)), ...);

    // End the line
    fprintf(self->outfile, "\n");
}

static std::string mem_offset(Register base, int offset) {
    return std::to_string(offset) + "(" + REGISTER_NAMES[base] + ")";
}

static std::string as_label(int index) {
    return std::string{"L"} + std::to_string(index);
}

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
    fputs("\nswitch:\n", self->outfile);
    write_mnemonic(self, "move", Register::T0, Register::A0);
    // scan no of cases
    write_mnemonic(self, "lw", Register::T1, mem_offset(Register::A1, 0));
    fputs("\nnext:\n", self->outfile);
    write_mnemonic(self, "addi", Register::A1, Register::A1, 4);
    write_mnemonic(self, "lw", Register::T2, mem_offset(Register::A1, 0));
    // load label
    write_mnemonic(self, "addi", Register::A1, Register::A1, 4);
    write_mnemonic(self, "bne", Register::T0, Register::T2, "fail");
    write_mnemonic(self, "lw", Register::T3, mem_offset(Register::A1, 0));
    write_mnemonic(self, "jr", Register::T3);
    fputs("\nfail:\n", self->outfile);
    write_mnemonic(self, "addi", Register::T1, Register::T1, -1);
    write_mnemonic(self, "bgtz", Register::T1, "next");
    write_mnemonic(self, "addi", Register::A1, Register::A1, 4);
    // last is just label so scan that
    // and go to default
    write_mnemonic(self, "lw", Register::T3, mem_offset(Register::A1, 0));
    write_mnemonic(self, "jr", Register::T3);
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

    self->Compiler_ResetOffset();

    // 0 is $ra and 1 is $fp

    fputs(".text\n", self->outfile);
    if (ctx->functionId->_class != C_STATIC) {
        fprintf(self->outfile, "\t.globl %s\n", name);
    }
    fprintf(self->outfile, "\n%s:\n", name);

    // ! STYLE HEADER IS CREATED HERE
    fputs("\n# Frame:\t", self->outfile);
    int skip = 0;
    for (SymTableEntry curr = ctx->functionId->member; curr != nullptr; curr = curr->next) {
        if (skip++ < 4) {
            // skip if first four
            continue;
        }
        fprintf(self->outfile, "%s, ", curr->name);
    }

    fprintf(self->outfile, "%s, %s", REGISTER_NAMES[Register::FP], REGISTER_NAMES[Register::RA]);

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
        fprintf(self->outfile, "%s", REGISTER_NAMES[reglist[FIRST_PARAM_REG + i]]);
        if ((i + 1) < skip) {
            fputs(", ", self->outfile);
        }
    }

    fputs("\n# Clobbers:\t", self->outfile);

    for (int i = 0; i < skip; i++) {
        fprintf(self->outfile, "%s", REGISTER_NAMES[reglist[FIRST_PARAM_REG + i]]);
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
        loclCurr->posn = self->Compiler_GetLocalOffset(loclCurr->type) - 4;
    }

    for (; paramCurr != nullptr; paramCurr = paramCurr->next) {
        // for remaining params they get pushed on stack
        paramCurr->posn = self->localOffset + self->Compiler_GetParamOffset(paramCurr->type);
    }

    // need to add local offset to all offsets somehow

    fputs("\n# Locals:\n", self->outfile);
    for (SymTableEntry curr = st->loclHead; curr != nullptr; curr = curr->next) {
        fprintf(self->outfile, "#\t-`%s` in %d($sp)\n", curr->name, curr->posn);
    }

    fputs("\n\n", self->outfile);

    write_mnemonic(self, "begin");
    fputs("\n", self->outfile);
    write_mnemonic(self, "push", Register::RA);

    // Actual offset for locals if have been initialised
    if (st->loclHead) {
        write_mnemonic(self, "addiu", Register::SP, Register::SP, -self->localOffset);
    }

    fputs("\n", self->outfile);
}

void MIPS_PostFunc(Compiler self, Context ctx) {
    // TODO: if there are no early returns
    // TODO: we don't add return label
    MIPS_ReturnLabel(self, ctx);
    if (self->localOffset > 0) {
        write_mnemonic(self, "addiu", Register::SP, Register::SP, self->localOffset);
    }

    write_mnemonic(self, "pop", Register::RA);
    write_mnemonic(self, "end");
    write_mnemonic(self, "jr", Register::RA);
    fputs("\n", self->outfile);

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
    write_mnemonic(self, "li", reglist[r], value);
    return r;
}

int MIPS_Add(Compiler self, int r1, int r2) {
    write_mnemonic(self, "add", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_Mul(Compiler self, int r1, int r2) {
    write_mnemonic(self, "mul", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_Sub(Compiler self, int r1, int r2) {
    // not communative
    write_mnemonic(self, "sub", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_Div(Compiler self, int r1, int r2) {
    // also not communative
    write_mnemonic(self, "div", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_Mod(Compiler self, int r1, int r2) {
    // not communative
    write_mnemonic(self, "mod", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

void MIPS_PrintInt(Compiler self, int r) {
    write_mnemonic(self, "li", Register::V0, 1);
    write_mnemonic(self, "move", Register::A0, reglist[r]);
    write_mnemonic(self, "syscall");
}

void MIPS_PrintChar(Compiler self, int r) {
    write_mnemonic(self, "li", Register::V0, 11);
    write_mnemonic(self, "move", Register::A0, reglist[r]);
    write_mnemonic(self, "syscall");
}

void MIPS_PrintStr(Compiler self, int r) {
    write_mnemonic(self, "li", Register::V0, 4);
    write_mnemonic(self, "move", Register::A0, reglist[r]);
    write_mnemonic(self, "syscall");
}

int MIPS_LoadGlobStr(Compiler self, SymTableEntry sym) {
    int r = allocReg(self);
    write_mnemonic(self, "la", reglist[r], sym->name);
    return r;
}

int MIPS_LoadGlob(Compiler self, SymTableEntry sym, enum ASTOP op) {
    int r = allocReg(self);
    int r2;
    switch (sym->type) {
        case P_INT:
            write_mnemonic(self, "lw", reglist[r], sym->name);

            if (op == A_PREINC || op == A_PREDEC) {
                write_mnemonic(self, "addi", reglist[r], reglist[r], op == A_PREINC ? "1" : "-1");
                write_mnemonic(self, "sw", reglist[r], sym->name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                write_mnemonic(self, "move", reglist[r2], reglist[r]);
                write_mnemonic(self, "addi", reglist[r2], reglist[r2], op == A_POSTINC ? "1" : "-1");
                write_mnemonic(self, "sw", reglist[r2], sym->name);
                freeReg(self, r2);
            }

            break;
        case P_CHAR:
            write_mnemonic(self, "lbu", reglist[r], sym->name);

            if (op == A_PREINC || op == A_PREDEC) {
                write_mnemonic(self, "addi", reglist[r], reglist[r], op == A_PREINC ? "1" : "-1");
                write_mnemonic(self, "sb", reglist[r], sym->name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                write_mnemonic(self, "move", reglist[r2], reglist[r]);
                write_mnemonic(self, "addi", reglist[r2], reglist[r2], op == A_POSTINC ? "1" : "-1");
                write_mnemonic(self, "sb", reglist[r2], sym->name);
                freeReg(self, r2);
            }

            break;
            // case P_CHARPTR:
            // case P_INTPTR:
        default:
            write_mnemonic(self, "lw", reglist[r], sym->name);

            if (op == A_PREINC || op == A_PREDEC) {
                write_mnemonic(self, "addi", reglist[r], reglist[r], op == A_PREINC ? "1" : "-1");
                write_mnemonic(self, "sw", reglist[r], sym->name);
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                write_mnemonic(self, "move", reglist[r2], reglist[r]);
                write_mnemonic(self, "addi", reglist[r2], reglist[r2], op == A_POSTINC ? "1" : "-1");
                write_mnemonic(self, "sw", reglist[r2], sym->name);
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
        write_mnemonic(self, "move", reglist[r], reglist[sym->paramReg]);

        if (op == A_PREINC || op == A_PREDEC) {
            write_mnemonic(self, "addi", reglist[r], reglist[r], op == A_PREINC ? "1" : "-1");
            write_mnemonic(self, "move", reglist[r], reglist[sym->paramReg]);
        }

        if (op == A_POSTINC || op == A_POSTDEC) {
            r2 = allocReg(self);
            write_mnemonic(self, "move", reglist[r2], reglist[r]);
            write_mnemonic(self, "addi", reglist[r2], reglist[r2], op == A_POSTINC ? "1" : "-1");
            write_mnemonic(self, "move", reglist[r], reglist[sym->paramReg]);
            freeReg(self, r2);
        }
        return r;
    }

    switch (sym->type) {
        case P_INT:
            write_mnemonic(self, "lw", reglist[r], mem_offset(Register::SP, sym->posn));
            if (op == A_PREINC || op == A_PREDEC) {
                write_mnemonic(self, "addi", reglist[r], reglist[r], op == A_PREINC ? "1" : "-1");
                write_mnemonic(self, "sw", reglist[r], mem_offset(Register::SP, sym->posn));
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                write_mnemonic(self, "move", reglist[r2], reglist[r]);
                write_mnemonic(self, "addi", reglist[r2], reglist[r2], op == A_POSTINC ? "1" : "-1");
                write_mnemonic(self, "sw", reglist[r2], mem_offset(Register::SP, sym->posn));
                freeReg(self, r2);
            }

            break;
        case P_CHAR:
            write_mnemonic(self, "lbu", reglist[r], mem_offset(Register::SP, sym->posn));

            if (op == A_PREINC || op == A_PREDEC) {
                write_mnemonic(self, "addi", reglist[r], reglist[r], op == A_PREINC ? "1" : "-1");
                write_mnemonic(self, "sb", reglist[r], mem_offset(Register::SP, sym->posn));
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                write_mnemonic(self, "move", reglist[r2], reglist[r]);
                write_mnemonic(self, "addi", reglist[r2], reglist[r2], op == A_POSTINC ? "1" : "-1");
                write_mnemonic(self, "sb", reglist[r2], mem_offset(Register::SP, sym->posn));
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
            write_mnemonic(self, "lw", reglist[r], mem_offset(Register::SP, sym->posn));

            if (op == A_PREINC || op == A_PREDEC) {
                write_mnemonic(self, "addi", reglist[r], reglist[r], op == A_PREINC ? "1" : "-1");
                write_mnemonic(self, "sw", reglist[r], mem_offset(Register::SP, sym->posn));
            }

            if (op == A_POSTINC || op == A_POSTDEC) {
                r2 = allocReg(self);
                write_mnemonic(self, "move", reglist[r2], reglist[r]);
                write_mnemonic(self, "addi", reglist[r2], reglist[r2], op == A_POSTINC ? "1" : "-1");
                write_mnemonic(self, "sw", reglist[r2], mem_offset(Register::SP, sym->posn));
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
            write_mnemonic(self, "sw", reglist[r1], sym->name);
            break;
        case P_CHAR:
            write_mnemonic(self, "sb", reglist[r1], sym->name);
            break;
        default:
            if (!ptrtype(sym->type)) {
                debug("store glob error");
                fatala("InternalError: Unknown type %d", sym->type);
            }

            write_mnemonic(self, "sw", reglist[r1], sym->name);
            break;
    }

    return r1;
}

void MIPS_StoreParam(Compiler self, int r1) {
    if (r1 == NO_REG) {
        fatal("InternalError: Trying to store an empty register");
    }

    write_mnemonic(self, "push", reglist[r1]);
}

int MIPS_StoreLocal(Compiler self, int r1, SymTableEntry sym) {
    if (r1 == NO_REG) {
        fatal("InternalError: Trying to store an empty register");
    }

    if (sym->isFirstFour) {
        write_mnemonic(self, "move", reglist[sym->paramReg], reglist[r1]);
        return r1;
    }

    switch (sym->type) {
        case P_INT:
            write_mnemonic(self, "sw", reglist[r1], mem_offset(Register::SP, sym->posn));
            break;
        case P_CHAR:
            write_mnemonic(self, "sb", reglist[r1], mem_offset(Register::SP, sym->posn));
            break;
        default:
            if (!ptrtype(sym->type)) {
                debug("store local error");
                fatala("InternalError: Unknown type %d", sym->type);
            }

            write_mnemonic(self, "sw", reglist[r1], mem_offset(Register::SP, sym->posn));
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
            write_mnemonic(self, "sb", reglist[r1], mem_offset(reglist[r2], 0));
            break;
        case 4:
            write_mnemonic(self, "sw", reglist[r1], mem_offset(reglist[r2], 0));
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
        fprintf(self->outfile, "%s:\n\t.asciiz \"%s\"\n", sym->name, sym->strValue);
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
                if (sym->initList != nullptr && type == pointer_to(P_CHAR) && initValue != 0) {
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
    write_mnemonic(self, "li", Register::V0, 5);
    write_mnemonic(self, "syscall");

    // Call store glob later on
    // fprintf(self->outfile, "\tsw\t$v0, %s\n", g->Gsym[id].name);
    int r = allocReg(self);
    write_mnemonic(self, "move", reglist[r], Register::V0);
    return r;
}

int MIPS_InputChar(Compiler self) {
    write_mnemonic(self, "li", Register::V0, 12);
    write_mnemonic(self, "syscall");
    int r = allocReg(self);
    write_mnemonic(self, "move", reglist[r], Register::V0);
    return r;
}

void MIPS_InputString(Compiler self, char *name, int size) {
    write_mnemonic(self, "la", Register::A0, name);
    write_mnemonic(self, "li", Register::A1, size);
    write_mnemonic(self, "li", Register::V0, 8);
    write_mnemonic(self, "syscall");
}

int MIPS_EqualSet(Compiler self, int r1, int r2) {
    write_mnemonic(self, "seq", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

// ALl the jumps are reversed -> eq -> neq

int MIPS_EqualJump(Compiler self, int r1, int r2, int l) {
    write_mnemonic(self, "bne", reglist[r1], reglist[r2], as_label(l));
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

int MIPS_NotEqualSet(Compiler self, int r1, int r2) {
    write_mnemonic(self, "sne", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_NotEqualJump(Compiler self, int r1, int r2, int l) {
    write_mnemonic(self, "beq", reglist[r1], reglist[r2], as_label(l));
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

int MIPS_LessThanSet(Compiler self, int r1, int r2) {
    write_mnemonic(self, "slt", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_LessThanJump(Compiler self, int r1, int r2, int l) {
    write_mnemonic(self, "bge", reglist[r1], reglist[r2], as_label(l));
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

int MIPS_GreaterThanSet(Compiler self, int r1, int r2) {
    write_mnemonic(self, "sgt", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_GreaterThanJump(Compiler self, int r1, int r2, int l) {
    write_mnemonic(self, "ble", reglist[r1], reglist[r2], as_label(l));
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

int MIPS_LessThanEqualSet(Compiler self, int r1, int r2) {
    write_mnemonic(self, "sle", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_LessThanEqualJump(Compiler self, int r1, int r2, int l) {
    write_mnemonic(self, "bgt", reglist[r1], reglist[r2], as_label(l));
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

int MIPS_GreaterThanEqualSet(Compiler self, int r1, int r2) {
    write_mnemonic(self, "sge", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_GreaterThanEqualJump(Compiler self, int r1, int r2, int l) {
    write_mnemonic(self, "blt", reglist[r1], reglist[r2], as_label(l));
    Compiler_FreeAllReg(self, NO_REG);
    return NO_REG;
}

void MIPS_Label(Compiler self, int l) {
    fprintf(self->outfile, "L%d:\n", l);
}

void MIPS_Jump(Compiler self, int l) {
    write_mnemonic(self, "b", as_label(l));
}

void MIPS_GotoLabel(Compiler self, SymTableEntry sym) {
    fprintf(self->outfile, "%s:\n", sym->name);
}

void MIPS_GotoJump(Compiler self, SymTableEntry sym) {
    write_mnemonic(self, "b", sym->name);
}

void MIPS_ReturnLabel(Compiler self, Context ctx) {
    fprintf(self->outfile, "%s_end:\n", ctx->functionId->name);
}

void MIPS_ReturnJump(Compiler self, Context ctx) {
    fprintf(self->outfile, "\tb\t%s_end\n", ctx->functionId->name);
}

void MIPS_Return(Compiler self, int r, Context ctx) {
    if (ctx->functionId->type == P_INT) {
        write_mnemonic(self, "move", Register::V0, reglist[r]);
    } else if (ctx->functionId->type == P_CHAR) {
        write_mnemonic(self, "move", Register::V0, reglist[r]);
        // I dont think we need the below
        // fprintf(self->outfile, "\tandi\t$v0, %s, 0xFF\n", reglist[r]);
    } else {
        fatala("InternalError: Unknown type %d", ctx->functionId->type);
    }
    MIPS_ReturnJump(self, ctx);
}

int MIPS_Call(Compiler self, SymTableEntry sym) {
    int outr = allocReg(self);
    write_mnemonic(self, "jal", sym->name);
    write_mnemonic(self, "move", reglist[outr], Register::V0);
    // Calculates the length of parameters
    int offset = 0;

    // If $a0-a3 are used
    for (int i = 0; i < self->paramRegCount; i++) {
        write_mnemonic(self, "push", reglist[FIRST_PARAM_REG + i]);
    }

    for (SymTableEntry curr = sym->member; curr != nullptr; curr = curr->next) {
        offset += PrimSize(curr->type);
    }
    if (offset > 16)
        write_mnemonic(self, "addiu", Register::SP, Register::SP, offset - 16);

    for (int i = 0; i < self->paramRegCount; i++) {
        write_mnemonic(self, "pop", reglist[FIRST_PARAM_REG + i]);
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
        write_mnemonic(self, "push", reglist[r]);
    } else {
        debug("argPos: %d", argPos);
        debug("calculation %d", FIRST_PARAM_REG + argPos - 1);
        write_mnemonic(self, "move", reglist[FIRST_PARAM_REG + argPos - 1], reglist[r]);
    }
}

void MIPS_RegPush(Compiler self, int r) {
    write_mnemonic(self, "push", reglist[r]);
}

void MIPS_RegPop(Compiler self, int r) {
    write_mnemonic(self, "pop", reglist[r]);
}

void MIPS_Move(Compiler self, int r1, int r2) {
    write_mnemonic(self, "move", reglist[r2], reglist[r1]);
    freeReg(self, r1);
}

int MIPS_Address(Compiler self, SymTableEntry sym) {
    int r = allocReg(self);
    if (sym->_class == C_GLOBAL || sym->_class == C_EXTERN || sym->_class == C_STATIC) {
        write_mnemonic(self, "la", reglist[r], sym->name);
    } else {
        write_mnemonic(self, "la", reglist[r], mem_offset(Register::SP, sym->posn));
    }
    return r;
}

int MIPS_Deref(Compiler self, int r, enum ASTPRIM type) {
    //! bug: derefing not occuring for b[3]

    enum ASTPRIM newType = value_at(type);
    int size = PrimSize(newType);

    switch (size) {
        case 1:
            write_mnemonic(self, "lbu", reglist[r], mem_offset(reglist[r], 0));
            break;
        case 4:
            write_mnemonic(self, "lw", reglist[r], mem_offset(reglist[r], 0));
            break;
        default:
            fatala("InternalError: Unknown pointer type %d", type);
    }
    return r;
}

int MIPS_ShiftLeftConstant(Compiler self, int r, int c) {
    write_mnemonic(self, "sll", reglist[r], reglist[r], c);
    return r;
}

int MIPS_ShiftLeft(Compiler self, int r1, int r2) {
    // swap values if it doesnt work
    write_mnemonic(self, "sllv", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_ShiftRight(Compiler self, int r1, int r2) {
    write_mnemonic(self, "srav", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_Negate(Compiler self, int r) {
    write_mnemonic(self, "neg", reglist[r], reglist[r]);
    return r;
}

int MIPS_BitNOT(Compiler self, int r) {
    write_mnemonic(self, "not", reglist[r], reglist[r]);
    return r;
}

int MIPS_LogNOT(Compiler self, int r) {
    write_mnemonic(self, "nor", reglist[r], reglist[r], reglist[r]);
    return r;
}

int MIPS_BitAND(Compiler self, int r1, int r2) {
    write_mnemonic(self, "and", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_BitOR(Compiler self, int r1, int r2) {
    write_mnemonic(self, "or", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_BitXOR(Compiler self, int r1, int r2) {
    write_mnemonic(self, "xor", reglist[r2], reglist[r1], reglist[r2]);
    freeReg(self, r1);
    return r2;
}

int MIPS_ToBool(Compiler self, enum ASTOP parentOp, int r, int label) {
    if (parentOp == A_WHILE || parentOp == A_IF) {
        // fake instruction
        // used to be bltu - but for some reason it never works
        write_mnemonic(self, "beq", reglist[r], Register::ZERO, as_label(label));
        return r;
    } else {
        write_mnemonic(self, "seq", reglist[r], reglist[r], Register::ZERO);
    }

    return r;
}

int MIPS_LogOr(Compiler self, int r1, int r2) {
    int Ltrue = Compiler_GenLabel(self);
    int Lend = Compiler_GenLabel(self);

    write_mnemonic(self, "bnez", reglist[r1], as_label(Ltrue));
    write_mnemonic(self, "bnez", reglist[r2], as_label(Ltrue));

    write_mnemonic(self, "li", reglist[r1], 0);
    write_mnemonic(self, "b", as_label(Lend));

    MIPS_Label(self, Ltrue);
    write_mnemonic(self, "li", reglist[r1], 1);
    MIPS_Label(self, Lend);

    freeReg(self, r2);
    return r1;
}

int MIPS_LogAnd(Compiler self, int r1, int r2) {
    int Lfalse = Compiler_GenLabel(self);
    int Lend = Compiler_GenLabel(self);

    write_mnemonic(self, "beqz", reglist[r1], as_label(Lfalse));
    write_mnemonic(self, "beqz", reglist[r2], as_label(Lfalse));

    write_mnemonic(self, "li", reglist[r1], 1);
    write_mnemonic(self, "b", as_label(Lend));

    MIPS_Label(self, Lfalse);
    write_mnemonic(self, "li", reglist[r1], 0);
    MIPS_Label(self, Lend);
    freeReg(self, r2);
    return r1;
}

void MIPS_Poke(Compiler self, int r1, int r2) {
    write_mnemonic(self, "sw", reglist[r1], mem_offset(reglist[r2], 0));
}

int MIPS_Peek(Compiler self, int r1, int r2) {
    write_mnemonic(self, "lw", reglist[r1], mem_offset(reglist[r2], 0));
    freeReg(self, r2);
    return r1;
}

void MIPS_Switch(Compiler self, int r, int caseCount, int topLabel, int *caseLabel, int *caseVal, int defaultLabel) {
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
    write_mnemonic(self, "move", Register::A0, reglist[r]);
    write_mnemonic(self, "la", Register::A1, as_label(label));
    // fprintf(self->outfile, "\tla\t$v1, L%d\n", label);

    write_mnemonic(self, "jal", "switch");
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
