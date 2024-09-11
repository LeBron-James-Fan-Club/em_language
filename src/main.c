#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ast.h"
#include "context.h"
#include "decl.h"
#include "defs.h"
#include "flags.h"
#include "gen.h"
#include "misc.h"
#include "scan.h"
#include "stmt.h"
#include "sym.h"

Flags flags;

static void usage(char *path);
static int argParse(int argc, char *argv[]);
static void preprocess(Scanner s, char *filename);

static void usage(char *path) {
    printf("Options:\n");
    printf("  -T: Dump AST\n");
    printf("  -d: Debug\n");
    printf("  -p: Fix parameters to 4\n");
    printf("  -S: Dump symbol table\n");
    printf("Usage: %s [options] [file] [outfile]\n", path);
    exit(-1);
}

static int argParse(int argc, char *argv[]) {
    if (argc < 3) {
        usage(argv[0]);
    }

    flags = (Flags){
        .dumpAST = false, .debug = false, .paramFix = false, .dumpSym = false};

    int i;

    for (i = 1; i < argc; i++) {
        if (*argv[i] != '-') {
            break;
        }
        for (int j = 1; argv[i][j] != '\0'; j++) {
            switch (argv[i][j]) {
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
                default:
                    usage(argv[0]);
            }
        }
    }

    return i;
}

static void preprocess(Scanner s, char *filename) {
    char *cmd;
    asprintf(&cmd, "%s %s %s", CPPCMD, INCDIR, filename);

    if ((s->infile = popen(cmd, "r")) == NULL) {
        fatala("OSError: Unable to open pipe to cpp %s, error: %s", filename,
               strerror(errno));
    }
    s->infilename = filename;

    debug("Preprocessing %s", filename);

    free(cmd);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        usage(argv[0]);
    }

    argParse(argc, argv);

    int i = argParse(argc, argv);

    Scanner s = Scanner_New();
    preprocess(s, argv[i]);

    if (s == NULL) {
        fatal("InternalError: unable to initialise scanner\n");
    }

    Compiler c = Compiler_New(argv[i + 1]);
    SymTable st = SymTable_New();
    Token tok = calloc(1, sizeof(struct token));
    Context ctx = Context_New();

    MIPS_Pre(c);

    Scanner_Scan(s, tok);
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

    printf(
        "\033[32m"
        "Success!"
        "\033[0m\n");

    exit(0);
}