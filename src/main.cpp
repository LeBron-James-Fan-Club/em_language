#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "decl.h"
#include "defs.h"
#include "flags.h"
#include "gen.h"
#include "misc.h"
#include "scan.h"
#include "sym.h"

#define CPPCMD "cpp -P -nostdinc -isystem"
// declared in makeifle
#ifndef INCDIR
#define INCDIR "./bin/include"
#endif

Flags flags;

static void usage(char *path);
static int argParse(int argc, char *argv[]);
static void preprocess(Scanner s, char *filename);

static void usage(char *path) {
    printf("Usage: %s [file] [outfile]\n", path);
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

    if ((s->infile = popen(cmd, "r")) == nullptr) {
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

    if (s == nullptr) {
        fatal("InternalError: unable to initialise scanner\n");
    }

    Compiler c = Compiler_New(argv[i + 1]);
    SymTable st = SymTable_New();
    Token tok = new token;
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
    delete tok;
    Context_Free(ctx);

    printf(
        "\033[32m"
        "Success!"
        "\033[0m\n");

    exit(0);
}