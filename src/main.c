#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "context.h"
#include "decl.h"
#include "defs.h"
#include "gen.h"
#include "scan.h"
#include "stmt.h"
#include "sym.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: [file] [outfile]\n");
        exit(-1);
    }

    Scanner s = Scanner_New(argv[1]);
    if (s == NULL) {
        fprintf(stderr, "Error: unable to initialise scanner\n");
        exit(-1);
    }

    Compiler c = Compiler_New(argv[2]);
    SymTable st = SymTable_New();
    Token tok = calloc(1, sizeof(struct token));
    Context ctx = Context_New();

    // MIPS_Pre(c);
    Scanner_Scan(s, tok);

    ASTnode t;
    while (true) {
        t = function_declare(c, s, st, tok,
                             ctx);  // Compound_Statement(s, st, tok);
        Compiler_Gen(c, st, ctx, t);
        if (tok->token == T_EOF) break;
    }

    // Compiler_Gen(c, st, t);

    MIPS_Post(c);

    Compiler_GenData(c, st);

    Scanner_Free(s);
    Compiler_Free(c);
    SymTable_Free(st);
    free(tok);
    Context_Free(ctx);

    printf("Success!\n");

    exit(0);
}