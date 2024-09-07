#include <string.hm>

i32 strlen(string str) {
    i32 len = 0;
    while (str[len] != 0) {
        len++;
    }
    return len;
}