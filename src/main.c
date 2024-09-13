#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "flags.h"

Flags flags;

int main(int argc, char *argv[]) {
    printf("NOTE: The author makes evident that using\n"
           "this tool to complete work where it is not\n"
           "permitted, such as in COMP1521 activities\n"
           "and assignments, is not condoned, and takes\n"
           "no responsibility in such events.\n\n");

    int opt;
    char *output = "-";

    // Parse command line options
    while ((opt = getopt(argc, argv, "TdpSo:")) != -1) {
        switch (opt) {
            case 'T':
                flags.dumpAST = true;
                break;
            case 'd':
                flags.debug = true;
                break;
            case 'p':
                flags.paramFix = true;
                break;
            case 'S':
                flags.dumpSym = true;
                break;
            case 'o':
                output = optarg;
                break;
            case '?':
                fprintf(stderr, "Usage: %s [-T] [-d] [-p] [-S] -o output_file [input_files...]\n", argv[0]);
                exit(EXIT_FAILURE);
            default:
                abort();
        }
    }

    for (int i = optind; i < argc; i++) {
        printf("input: %s\n", argv[i]);
    }

    printf("output: %s\n", output);

    /*
    Scanner s = Scanner_New();

    if (s == NULL) {
        fatal("InternalError: unable to initialise scanner\n");
    }

    Compiler c = Compiler_New(o_value);
    SymTable st = SymTable_New();
    Token tok = calloc(1, sizeof(struct token));
    Context ctx = Context_New();

    MIPS_Pre(c);

    Scanner_Scan(s, tok);
    debug("global_declare 222");
    global_declare(c, s, st, tok, ctx);
    MIPS_Post(c);

    Compiler_GenData(c, st);

    if (flags.dumpSym) {
        printf("SYM TABLE\n");
        SymTable_Dump(st);
    }

    Scanner_Free(s);
    Compiler_Free(c);
    SymTable_Free(st);
    free(tok);
    Context_Free(ctx);

    exit(0);
     */
}
