#ifndef FLAGS_H
#define FLAGS_H

#include <stdbool.h>

struct flags {
    bool dumpAST;
};

typedef struct flags Flags;

#endif