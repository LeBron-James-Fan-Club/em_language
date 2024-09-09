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
    char putback{'\n'};
    // current line number
    int line{1};

    char text[TEXTLEN + 1];

    Token rejToken;

    ~scanner();
};

typedef struct scanner *Scanner;

void Scanner_Scan(Scanner self, Token t);
void Scanner_Putback(Scanner self, char c);
void Scanner_RejectToken(Scanner self, Token t);

void match(Scanner s, Token t, enum OPCODES op, char *tok);
void semi(Scanner s, Token t);
void ident(Scanner s, Token t);
void lbrace(Scanner s, Token t);
void rbrace(Scanner s, Token t);
void lparen(Scanner s, Token t);
void rparen(Scanner s, Token t);
void comma(Scanner s, Token t);

#endif