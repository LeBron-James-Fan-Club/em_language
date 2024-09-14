#include <stdlib.h>
#include "new-compiler.h"
#include "strong_ast.h"

struct source_ast_pair {
    char *source;
    AstNode root;
};

struct new_compiler {
    struct source_ast_pair *inputs;
    int input_count;
};

// debugging compiler
NewCompiler compiler_new() {
    NewCompiler compiler = calloc(1, sizeof(*compiler));

    if (compiler == NULL) {
        perror("NewCompiler");
        exit(EXIT_FAILURE);
    }

    return compiler;
}

void compiler_accept(NewCompiler compiler, char *source) {
    AstNode root = ast_parse(source);

    int inputs = compiler->input_count;
    struct source_ast_pair *new_list = reallocarray(compiler->inputs, inputs + 1, sizeof(struct source_ast_pair));

    if (new_list == NULL) {
        perror("could not allocate enough space");
        exit(EXIT_FAILURE);
    }

    new_list[inputs] = (struct source_ast_pair) {
        .source = source,
        .root = root,
    };
    compiler->inputs = new_list;
    compiler->input_count++;
}

void compiler_assemble(NewCompiler compiler, FILE *output) {
    fprintf(output, "# todo: assemble for %p\n", compiler);
}

void compiler_free(NewCompiler compiler) {
    for (int i = 0; i < compiler->input_count; ++i) {
        free(compiler->inputs[i].source);
        ast_free(compiler->inputs[i].root);
    }

    free(compiler->inputs);
    free(compiler);
}
