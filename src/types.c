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