#include "types.h"

#include <stdio.h>

bool type_compatible(int *left, int *right, bool onlyright) {
    if (*left == *right) {
        *left = *right = 0;
        return true;
    }

    int leftsize = PrimSize(*left);
    int rightsize = PrimSize(*right);

    if (leftsize == 0 || rightsize == 0) {
        return false;
    }
    if (leftsize < rightsize) {
        // prevents the int from being converted to a char
        *left = A_WIDEN;
        *right = 0;
        return true;
    }

    if (rightsize < leftsize) {
        // prevents the int from being converted to a char
        if (onlyright) return false;
        *left = 0;
        *right = A_WIDEN;
        return true;
    }

    // Last thing should be that it equals to each other
    *left = *right = 0;
    return true;
}

bool inttype(enum ASTPRIM type) {
    return type == P_INT || type == P_CHAR;
}

bool ptrtype(enum ASTPRIM type) {
    return type == P_VOIDPTR || type == P_INTPTR || type == P_CHARPTR;
}

ASTnode modify_type(ASTnode tree, enum ASTPRIM rtype, enum ASTOP op, bool onlyright) {
    enum ASTPRIM ltype;
    int lsize, rsize;
    ltype = tree->type;
    if (inttype(ltype) && inttype(rtype)) {
        if (ltype == rtype) return tree;
        lsize = PrimSize(ltype);
        rsize = PrimSize(rtype);
        if (lsize > rsize) {
            printf("Out\n");
            return NULL;
        } else if (lsize < rsize) {
            return ASTnode_NewUnary(A_WIDEN, ltype, tree, 0);
        }
        
    }
    if (ptrtype(rtype) && !onlyright) {
        printf("worked\n");
        if (op == A_NONE && ptrtype(rtype)) {
            printf("Out22\n");
            return tree;
        }
        // I don't know why but the types are swapped
        // Pointer -> left, Value -> right
        if (op == A_ADD || op == A_SUBTRACT) {
            printf("Worked\n");
            if (inttype(ltype) && ptrtype(rtype)) {
                rsize = PrimSize(value_at(rtype));
                if (rsize > 1) return ASTnode_NewUnary(A_SCALE, rtype, tree, rsize);
            }
        }
    }

    // Types are incompatible
    printf("Out33\n");
    return NULL;
}

enum ASTPRIM pointer_to(enum ASTPRIM type) {
    switch (type) {
        case P_VOID:
            return P_VOIDPTR;
        case P_INT:
            return P_INTPTR;
        case P_CHAR:
            return P_CHARPTR;
        default:
            fprintf(stderr, "Error: Unknown type %d for pointer\n", type);
            exit(-1);
    }
}

enum ASTPRIM value_at(enum ASTPRIM type) {
    switch (type) {
        case P_VOIDPTR:
            return P_VOID;
        case P_INTPTR:
            return P_INT;
        case P_CHARPTR:
            return P_CHAR;
        default:
            fprintf(stderr, "Error: Unknown type %d for pointer\n", type);
            exit(-1);
    }
}