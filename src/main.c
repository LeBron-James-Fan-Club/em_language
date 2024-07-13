#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "context.h"
#include "decl.h"
#include "defs.h"
#include "flags.h"
#include "gen.h"
#include "scan.h"
#include "stmt.h"
#include "sym.h"

static void usage(char *path) {
    printf("Usage: %s [file] [outfile]\n", path);
    exit(-1);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        usage(argv[0]);
    }

    Flags f = {
        .dumpAST = false,
    };

    int i;

    for (i = 1; i < argc; i++) {
#if DEBUG
        printf("argv[%d] = %s\n", i, argv[i]);
#endif
        if (*argv[i] != '-') {
            break;
        }
        for (int j = 1; argv[i][j] != '\0'; j++) {
            switch (argv[i][j]) {
                case 'T':
                    f.dumpAST = true;
                    break;
                default:
                    usage(argv[0]);
            }
        }
    }

    Scanner s = Scanner_New(argv[i]);
    if (s == NULL) {
        fprintf(stderr, "Error: unable to initialise scanner\n");
        exit(-1);
    }

    Compiler c = Compiler_New(argv[i + 1]);
    SymTable st = SymTable_New();
    Token tok = calloc(1, sizeof(struct token));
    Context ctx = Context_New();

    Scanner_Scan(s, tok);
    global_declare(c, s, st, tok, ctx, f);
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