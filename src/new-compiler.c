#include <stdlib.h>
#include "new-compiler.h"

// debugging compiler
void *compiler_new() {
    return NULL;
}

void compiler_accept(void *compiler, char *source) {
    printf("Parsing source in %s", source);
    free(source);
}

void compiler_assemble(void *compiler, FILE *output) {
    fprintf(output, "# todo: assemble for %p\n", compiler);
}

void compiler_free(void *compiler) {
}
