#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "flags.h"
#include "new-compiler.h"

Flags flags;

static char *read_file(const char *filename);

int main(int argc, char *argv[]) {
    printf("NOTE: The author makes evident that using\n"
           "this tool to complete work where it is not\n"
           "permitted, such as in COMP1521 activities\n"
           "and assignments, is not condoned, and takes\n"
           "no responsibility in such events.\n\n");

    int opt;
    char *output = "-";

    // Parse command line options
    while ((opt = getopt(argc, argv, "TdSo:")) != -1) {
        switch (opt) {
            case 'T':
                flags.dumpAST = true;
                break;
            case 'd':
                flags.debug = true;
                break;
            case 'S':
                flags.dumpSym = true;
                break;
            case 'o':
                output = optarg;
                break;
            case '?':
                fprintf(stderr, "Usage: %s [-T] [-d] [-S] -o output_file [input_files...]\n", argv[0]);
                exit(EXIT_FAILURE);
            default:
                abort();
        }
    }

    void *compiler = compiler_new();

    for (int i = optind; i < argc; i++) {
        compiler_accept(compiler, read_file(argv[i]));
    }

    FILE *out;

    if (strcmp(output, "-") == 0) {
        out = stdout;
    } else {
        out = fopen(output, "w");

        if (out == NULL) {
            perror("opening output");
            exit(EXIT_FAILURE);
        }
    }

    compiler_assemble(compiler, out);
    compiler_free(compiler);

    return EXIT_SUCCESS;

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

char *read_file(const char *filename) {
    char *buffer = NULL;
    size_t buffer_size = 0;

    FILE *memstream = open_memstream(&buffer, &buffer_size);

    if (memstream == NULL) {
        perror("open_memstream");
        exit(EXIT_FAILURE);
    }

    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process

        // Close the read end of the pipe in the child
        close(pipefd[STDIN_FILENO]);

        // Redirect stdout to the write end of the pipe
        dup2(pipefd[STDOUT_FILENO], STDOUT_FILENO);
        close(pipefd[STDOUT_FILENO]);

        // Preprocess
        execlp("cpp", "cpp", "-P", "-nostdinc", filename, NULL);

        // Expect cpp to exit
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process

        // Close the write end of the pipe in the parent
        close(pipefd[STDOUT_FILENO]);

        // Read from the pipe and write into the memory stream
        char tmp_buffer[4096];
        ssize_t n;

        while ((n = read(pipefd[STDIN_FILENO], tmp_buffer, sizeof(tmp_buffer))) > 0) {
            fwrite(tmp_buffer, 1, n, memstream);
        }

        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        // Close the read end of the pipe
        close(pipefd[STDIN_FILENO]);

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);

        fclose(memstream);
        return buffer;
    }
}
