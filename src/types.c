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