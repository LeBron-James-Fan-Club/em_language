#include "misc.h"

#include <stdio.h>
#include <stdarg.h>

#include "scan.h"
#include "flags.h"

static inline void start_red() {
    printf("\033[1;31m");
}

static inline void end_red() {
    printf("\033[0m");
}


noreturn void lfatal(Scanner s, char *msg) {
    start_red();
    fprintf(stderr, "%s on line %d\n", msg, s->line);
    end_red();
    exit(EXIT_FAILURE);
}

noreturn void lfatala(Scanner s, char *msg, ...) {
    va_list args;
    
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    start_red();
    fprintf(stderr, "%s on line %d\n", msg, s->line);
    end_red();

    exit(EXIT_FAILURE);
}

noreturn void fatal(char *msg) {
    
    start_red();
    fprintf(stderr, "%s\n", msg);
    end_red();

    exit(EXIT_FAILURE);
}

noreturn void fatala(char *msg, ...) {
    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    start_red();
    fprintf(stderr, "%s\n", msg);
    end_red();

    exit(EXIT_FAILURE);
}

void debug(char *msg, ...) {
    if (!flags.debug) return;
    va_list args;

    va_start(args, msg);
    fprintf(stderr, "DEBUG: ");
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
}