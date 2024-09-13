#include <stdio.h>

void *compiler_new();
void compiler_accept(void *compiler, char *source);
void compiler_assemble(void *compiler, FILE *output);
void compiler_free(void *compiler);
