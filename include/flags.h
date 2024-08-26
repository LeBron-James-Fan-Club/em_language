#ifndef FLAGS_H
#define FLAGS_H

#include <stdbool.h>

struct flags {
    bool dumpAST;
    bool debug;
};

typedef struct flags Flags;

extern Flags flags;

#endif