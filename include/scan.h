#ifndef SCAN_H
#define SCAN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "tokens.h"

#define TEXTLEN 512

struct scanner {
    // file to scan in
    FILE *infile;
    // character to put back
    char putback;
    // current line number
    int line;

    char text[TEXTLEN + 1];
};

typedef struct scanner *Scanner;

Scanner Scanner_New(char *name);
void Scanner_Free(Scanner);
bool Scanner_Scan(Scanner, Token t);
void Scanner_Putback(Scanner this, char c);

#endif