#include "types.h"

#include <stdio.h>

bool type_compatible(int *left, int *right, bool onlyright) {
    if (*left == *right) {
        printf("Yee Yee aaaah types\n");
        return true;
    }
    printf("checking compat\n");
    int leftsize = PrimSize(*left);
    int rightsize = PrimSize(*right);

    if (leftsize == 0 || rightsize == 0) {
        printf("Lebron James 1\n");
        return false;
    }
    if (leftsize < rightsize) {
        // prevents the int from being converted to a char
        *left = A_WIDEN;
        *right = 0;
        printf("Lebron James 2\n");
        return true;
    }

    if (rightsize < leftsize) {
        // prevents the int from being converted to a char
        if (onlyright) return false;
        *left = 0;
        *right = A_WIDEN;
        printf("Lebron James 3\n");
        return true;
    }

    // Last thing should be that it equals to each other
    *left = *right = 0;
    printf("Lebron James 4\n");
    return true;
}