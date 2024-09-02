#ifndef FLAGS_H
#define FLAGS_H

#include <stdbool.h>

struct flags {
    bool dumpAST;
    bool debug;
    // Stick to 4 params (we use $a0-$a3)
    bool paramFix;
};

typedef struct flags Flags;

extern Flags flags;

#endif