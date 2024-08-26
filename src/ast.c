#include "ast.h"

#include "misc.h"
#include "sym.h"

ASTnode ASTnode_New(enum ASTOP op, enum ASTPRIM type, ASTnode left, ASTnode mid,
                    ASTnode right, SymTableEntry sym, int intvalue) {
    ASTnode n = calloc(1, sizeof(struct astnode));
    if (n == NULL) {
        fatal("InternalError: Unable to allocate memory for ASTnode\n");
    }

    n->op = op;
    n->type = type;
    n->left = left;
    n->mid = mid;
    n->sym = sym;
    n->right = right;
    n->intvalue = intvalue;

    return n;
}

ASTnode ASTnode_NewLeaf(enum ASTOP op, enum ASTPRIM type, SymTableEntry sym,
                        int intvalue) {
    return ASTnode_New(op, type, NULL, NULL, NULL, sym, intvalue);
}

ASTnode ASTnode_NewUnary(enum ASTOP op, enum ASTPRIM type, ASTnode left,
                         SymTableEntry sym, int intvalue) {
    return ASTnode_New(op, type, left, NULL, NULL, sym, intvalue);
}

void ASTnode_Free(ASTnode this) {
    if (this->left) ASTnode_Free(this->left);
    if (this->mid) ASTnode_Free(this->mid);
    if (this->right) ASTnode_Free(this->right);
    free(this);
}

static int makeLabel(void) {
    static int id = 1;
    return id++;
}

void ASTnode_Dump(ASTnode n, SymTable st, int label, int level) {
    int Lfalse, Lstart, Lend;
    switch (n->op) {
        case A_IF:
            Lfalse = makeLabel();
            for (int i = 0; i < level; i++) printf(" ");
            printf("A_IF");
            if (n->right) {
                Lend = makeLabel();
                printf("end L%d", Lend);
            }
            printf("\n");
            ASTnode_Dump(n->left, st, Lfalse, level + 2);
            ASTnode_Dump(n->mid, st, NO_LABEL, level + 2);
            if (n->right) {
                ASTnode_Dump(n->right, st, NO_LABEL, level + 2);
            }
            return;
        case A_WHILE:
            Lstart = makeLabel();
            for (int i = 0; i < level; i++) printf("  ");
            printf("A_WHILE start L%d\n", Lstart);
            Lend = makeLabel();
            ASTnode_Dump(n->left, st, Lend, level + 2);
            ASTnode_Dump(n->right, st, NO_LABEL, level + 2);
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
                printf("A_IDENT rval %s type %d\n", n->sym->name,
                       n->type);
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
        default:
            fatala("InternalError: Unknown AST node %d", n->op);
    }
}