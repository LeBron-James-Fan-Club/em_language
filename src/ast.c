#include "ast.h"

ASTnode ASTnode_New(enum ASTOP op, enum ASTPRIM type, ASTnode left, ASTnode mid,
                    ASTnode right, int intvalue) {
    ASTnode n = calloc(1, sizeof(struct astnode));
    if (n == NULL) {
        fprintf(stderr, "Error: Unable to initialise ASTnode\n");
        exit(-1);
    }

    n->op = op;
    n->type = type;
    n->left = left;
    n->mid = mid;
    n->right = right;
    n->v.intvalue = intvalue;

    return n;
}

ASTnode ASTnode_NewLeaf(enum ASTOP op, enum ASTPRIM type, int intvalue) {
    return ASTnode_New(op, type, NULL, NULL, NULL, intvalue);
}

ASTnode ASTnode_NewUnary(enum ASTOP op, enum ASTPRIM type, ASTnode left,
                         int intvalue) {
    return ASTnode_New(op, type, left, NULL, NULL, intvalue);
}

void ASTnode_Free(ASTnode this) {
    if (this->left) ASTnode_Free(this->left);
    if (this->right) ASTnode_Free(this->right);
    free(this);
}
