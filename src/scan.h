#ifndef SCAN_H
#define SCAN_H

#include <stdio.h>

#include "defs.h"

// May god forgive me for self
#define TEXTLEN 100000000

struct scanner {
    // file to scan in
    FILE *infile;
    char *infilename;

    // character to put back
    char _putback{'\n'};
    // current line number
    int line{1};

    char text[TEXTLEN + 1];

    Token rejToken;

    ~scanner();

    void Scanner_Scan(Token t);
    void Scanner_RejectToken(Token t);

    void match(Token t, enum OPCODES op, char *tok);
    void semi(Token t);
    void ident(Token t);
    void lbrace(Token t);
    void rbrace(Token t);
    void lparen(Token t);
    void rparen(Token t);
    void comma(Token t);
};

typedef struct scanner *Scanner;

#endif
