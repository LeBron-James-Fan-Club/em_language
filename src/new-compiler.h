#include <stdio.h>

typedef struct new_compiler *NewCompiler;

NewCompiler compiler_new();
void compiler_accept(NewCompiler compiler, char *source);
void compiler_assemble(NewCompiler compiler, FILE *output);
void compiler_free(NewCompiler compiler);
