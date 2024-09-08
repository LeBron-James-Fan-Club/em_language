#include "ast.h"

static ASTnode opt_fold2(ASTnode n);
static ASTnode opt_fold1(ASTnode n);
static ASTnode opt_fold(ASTnode n);

// Fold 2 children
static ASTnode opt_fold2(ASTnode n) {
    int val, lval, rval;
    lval = n->left->intvalue;
    rval = n->right->intvalue;

    switch (n->op) {
        case A_ADD:
            val = lval + rval;
            break;
        case A_SUBTRACT:
            val = lval - rval;
            break;
        case A_MULTIPLY:
            val = lval * rval;
            break;
        case A_DIVIDE:
            if (rval == 0) {
                fatal("Divide by zero");
            }
            val = lval / rval;
            break;
        default:
            return n;
    }

    enum ASTPRIM type = n->type;
    ASTnode_Free(n);

    return ASTnode_NewLeaf(A_INTLIT, type, NULL, val);
}

// Fold for 1 child
static ASTnode opt_fold1(ASTnode n) {
    int val, lval;
    lval = n->left->intvalue;

    switch (n->op) {
        case A_WIDEN:
            val = lval;
            break;
        case A_INVERT:
            val = ~lval;
            break;
        case A_LOGNOT:
            val = !lval;
            break;
        default:
            return n;
    }

    enum ASTPRIM type = n->type;
    ASTnode_Free(n);

    return ASTnode_NewLeaf(A_INTLIT, type, NULL, val);
}

static ASTnode opt_fold(ASTnode n) {
    if (n == NULL) return NULL;

    n->left = opt_fold(n->left);
    n->right = opt_fold(n->right);

    if (n->left && n->left->op == A_INTLIT) {
        if (n->right && n->right->op == A_INTLIT) {
            return opt_fold2(n);
        } else {
            return opt_fold1(n);
        }
    }
    
    return n;
}

ASTnode Optimise(ASTnode n) {
    return opt_fold(n);
}