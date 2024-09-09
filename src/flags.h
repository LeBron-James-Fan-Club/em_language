#ifndef FLAGS_H
#define FLAGS_H

struct flags {
    bool dumpAST;
    bool debug;
    bool dumpSym;
    // Stick to 4 params (we use $a0-$a3)
    bool paramFix;
};

typedef struct flags Flags;

extern Flags flags;

#endif