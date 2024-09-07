i32 pow(i32 base, i32 exp) {
    i32 result = 1;
    for (i32 i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}