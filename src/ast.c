#define _GNU_SOURCE

#include "ast.h"

#include "misc.h"
#include "sym.h"

#include <stdio.h>
#include <stdlib.h>

ASTnode ASTnode_New(enum ASTOP op, enum ASTPRIM type, ASTnode left, ASTnode mid,
                    ASTnode right, SymTableEntry ctype, SymTableEntry sym,
                    int intvalue) {
    ASTnode n = calloc(1, sizeof(struct astnode));
    if (n == NULL) {
        fatal("InternalError: Unable to allocate memory for ASTnode\n");
    }

    n->op = op;
    n->type = type;
    n->left = left;
    n->mid = mid;
    n->sym = sym;
    n->ctype = ctype;
    n->right = right;
    n->intvalue = intvalue;

    return n;
}

ASTnode ASTnode_NewLeaf(enum ASTOP op, enum ASTPRIM type, SymTableEntry ctype,
                        SymTableEntry sym, int intvalue) {
    return ASTnode_New(op, type, NULL, NULL, NULL, ctype, sym, intvalue);
}

ASTnode ASTnode_NewUnary(enum ASTOP op, enum ASTPRIM type, ASTnode left,
                         SymTableEntry ctype, SymTableEntry sym, int intvalue) {
    return ASTnode_New(op, type, left, NULL, NULL, ctype, sym, intvalue);
}

void ASTnode_Free(ASTnode this) {
    if (this->left) ASTnode_Free(this->left);
    if (this->mid) ASTnode_Free(this->mid);
    if (this->right) ASTnode_Free(this->right);
    if (this->label.customLabel) free(this->label.customLabel);
    free(this);
}

#define L_FALSE 0
#define L_START 1
#define L_END 2
#define L_NONE 3

static void makeLabel(struct label label, char **labelName, int type) {
    static int id = 1;
    if (label.hasCustomLabel) {
        switch (type) {
            case L_FALSE:
                asprintf(labelName, "%s_false", label.customLabel);
                break;
            case L_START:
                asprintf(labelName, "%s_start", label.customLabel);
                break;
            case L_END:
                asprintf(labelName, "%s_en", label.customLabel);
                break;
            default:
                asprintf(labelName, "%s", label.customLabel);
        }
    } else {
        asprintf(labelName, "L%d", id++);
    }
}

void ASTnode_Dump(ASTnode n, SymTable st, char *label, int level) {
    char *Lfalse = NULL, *Lstart = NULL, *Lend = NULL;
    switch (n->op) {
        case A_IF:
            makeLabel(n->label, &Lfalse, L_FALSE);
            for (int i = 0; i < level; i++) printf(" ");
            printf("A_IF");
            if (n->right) {
                makeLabel(n->label, &Lend, L_END);
                printf("end %s", Lend);
            }
            printf("\n");
            ASTnode_Dump(n->left, st, Lfalse, level + 2);
            ASTnode_Dump(n->mid, st, NO_LABEL, level + 2);
            if (n->right) {
                ASTnode_Dump(n->right, st, NO_LABEL, level + 2);
            }
            free(Lfalse);
            if (n->right) free(Lend);
            return;
        case A_WHILE:
            makeLabel(n->label, &Lstart, L_START);
            for (int i = 0; i < level; i++) printf("  ");
            printf("A_WHILE start %s\n", Lstart);
            makeLabel(n->label, &Lend, L_END);
            ASTnode_Dump(n->left, st, Lend, level + 2);
            ASTnode_Dump(n->right, st, NO_LABEL, level + 2);
            free(Lstart);
            free(Lend);
            return;
        default:
            break;
    }

    if (n->op == A_GLUE) level = -2;
    if (n->left) ASTnode_Dump(n->left, st, NO_LABEL, level + 2);
    if (n->right) ASTnode_Dump(n->right, st, NO_LABEL, level + 2);

    for (int i = 0; i < level; i++) printf(" ");
    switch (n->op) {
        case A_GLUE:
            printf("\n\n");
            return;
        case A_FUNCTION:
            printf("A_FUNCTION %s\n", n->sym->name);
            return;
        case A_ADD:
            printf("A_ADD\n");
            return;
        case A_SUBTRACT:
            printf("A_SUBTRACT\n");
            return;
        case A_MULTIPLY:
            printf("A_MULTIPLY\n");
            return;
        case A_DIVIDE:
            printf("A_DIVIDE\n");
            return;
        case A_MODULO:
            printf("A_MODULO\n");
            return;
        case A_EQ:
            printf("A_EQ\n");
            return;
        case A_NE:
            printf("A_NE\n");
            return;
        case A_LT:
            printf("A_LT\n");
            return;
        case A_GT:
            printf("A_GT\n");
            return;
        case A_LE:
            printf("A_LE\n");
            return;
        case A_GE:
            printf("A_GE\n");
            return;
        case A_INTLIT:
            printf("A_INTLIT %d\n", n->intvalue);
            return;
        case A_IDENT:
            if (n->rvalue)
                printf("A_IDENT rval %s type %d\n", n->sym->name, n->type);
            else
                printf("A_IDENT %s type %d\n", n->sym->name, n->type);
            return;
        case A_ASSIGN:
            printf("A_ASSIGN type %d\n", n->type);
            return;
        case A_PRINT:
            printf("A_PRINT\n");
            return;
        case A_INPUT:
            printf("A_INPUT\n");
            return;
        case A_LABEL:
            printf("A_LABEL %s\n", n->sym->name);
            return;
        case A_GOTO:
            printf("A_GOTO %s\n", n->sym->name);
            return;
        case A_RETURN:
            printf("A_RETURN type %d\n", n->type);
            return;
        case A_WIDEN:
            printf("A_WIDEN type %d\n", n->type);
            return;
        case A_FUNCCALL:
            printf("A_FUNCCALL %s\n", n->sym->name);
            return;
        case A_ADDR:
            printf("A_ADDR %s\n", n->sym->name);
            return;
        case A_DEREF:
            if (n->rvalue)
                printf("A_DEREF rval\n");
            else
                printf("A_DEREF\n");
            return;
        case A_SCALE:
            printf("A_SCALE %d\n", n->size);
            return;
        case A_STRLIT:
            printf("A_STRLIT %s\n", n->sym->name);
            return;
        case A_PREINC:
            printf("A_PREINC\n");
            return;
        case A_PREDEC:
            printf("A_PREDEC\n");
            return;
        case A_LOGOR:
            printf("A_LOGOR\n");
            return;
        case A_LOGAND:
            printf("A_LOGAND\n");
            return;
        case A_OR:
            printf("A_OR\n");
            return;
        case A_XOR:
            printf("A_XOR\n");
            return;
        case A_AND:
            printf("A_AND\n");
            return;
        case A_LSHIFT:
            printf("A_LSHIFT\n");
            return;
        case A_RSHIFT:
            printf("A_RSHIFT\n");
            return;
        case A_NEGATE:
            printf("A_NEGATE\n");
            return;
        case A_INVERT:
            printf("A_INVERT\n");
            return;
        case A_LOGNOT:
            printf("A_LOGNOT\n");
            return;
        case A_POSTINC:
            printf("A_POSTINC\n");
            return;
        case A_POSTDEC:
            printf("A_POSTDEC\n");
            return;
        case A_TOBOOL:
            printf("A_TOBOOL\n");
            return;
        case A_PEEK:
            printf("A_PEEK\n");
            return;
        case A_POKE:
            printf("A_POKE\n");
            return;
        case A_BREAK:
            printf("A_BREAK\n");
            return;
        case A_CONTINUE:
            printf("A_CONTINUE\n");
            return;
        case A_SWITCH:
            printf("A_SWITCH\n");
            return;
        case A_CASE:
            printf("A_CASE %d\n", n->intvalue);
            return;
        case A_DEFAULT:
            printf("A_DEFAULT\n");
            return;
        case A_CAST:
            printf("A_CAST %d\n", n->type);
            return;
        case A_ASPLUS:
            printf("A_ASPLUS\n");
            return;
        case A_ASMINUS:
            printf("A_ASMINUS\n");
            return;
        case A_ASSTAR:
            printf("A_ASMULTIPLY\n");
            return;
        case A_ASSLASH:
            printf("A_ASDIVIDE\n");
            return;
        case A_ASMOD:
            printf("A_ASMOD\n");
            return;
        default:
            fatala("InternalError: Unknown AST node %d", n->op);
    }
}