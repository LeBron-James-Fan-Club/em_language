#include "scan.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "tokens.h"

static char next(Scanner);
static void putback(Scanner, char c);
static char skip(Scanner this);

static int scanIdent(Scanner this, int c);
static int scanInt(Scanner, char c);
static int keyword(char *s);

static int chrpos(char *s, int c);

Scanner Scanner_New(char *name) {
    Scanner n = calloc(1, sizeof(struct scanner));
    n->line = 1;
    n->putback = '\n';
    n->infile = fopen(name, "r");
    if (n->infile == NULL) {
        fprintf(stderr, "Error: Unable to open file %s.\n", name);
        exit(-1);
    }
    return n;
}

void Scanner_Free(Scanner this) {
    fclose(this->infile);
    free(this);
}

static char next(Scanner this) {
    char c;
    if (this->putback) {
        c = this->putback;
        this->putback = 0;
        return c;
    }
    c = fgetc(this->infile);
    if (c == '\n') this->line++;
    return c;
}

// Con: one character buffer we cant really read 3 symbol characters only 2
static void putback(Scanner this, char c) { this->putback = c; }

void Scanner_Putback(Scanner this, char c) { putback(this, c); }

// ignores whitespace
static char skip(Scanner this) {
    char c;
    c = next(this);
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
           // \f not really used much anymore
           c == '\f') {
        c = next(this);
    }
    return c;
}

bool Scanner_Scan(Scanner this, Token t) {
    char c, tokenType;
    c = skip(this);

    switch (c) {
        case EOF:
            t->token = T_EOF;
            return false;
        case ';':
            // equv to eof
            // Need to manage putback of characters
            t->token = T_SEMI;
            // putback(this, c);
            return false;
        case '+':
            t->token = T_PLUS;
            break;
        case '-':
            t->token = T_MINUS;
            break;
        case '*':
            t->token = T_STAR;
            break;
        case '/':
            t->token = T_SLASH;
            break;
        case '%':
            t->token = T_MODULO;
            break;
        case '=':
            if ((c = next(this)) == '=') {
                t->token = T_EQ;
            } else {
                putback(this, c);
                t->token = T_ASSIGN;
            }
            break;
        case '!':
            if ((c = next(this)) == '=') {
                t->token = T_NE;
            } else {
                fprintf(stderr, "Error: Invalid character %c on line %d\n", c,
                        this->line);
                exit(-1);
            }
            break;
        case '<':
            if ((c = next(this)) == '=') {
                t->token = T_LE;
            } else {
                putback(this, c);
                t->token = T_LT;
            }
            break;
        case '>':
            if ((c = next(this)) == '=') {
                t->token = T_GE;
            } else {
                putback(this, c);
                t->token = T_GT;
            }
            break;
        case '{':
            printf("Found brace\n");
            t->token = T_LBRACE;
            break;
        case '}':
            t->token = T_RBRACE;
            break;
        case '(':
            t->token = T_LPAREN;
            break;
        case ')':
            // TODO NEED TO CHANGE THIS LATER ON
            // TODO WHEN WE USE PARENS
            t->token = T_RPAREN;
            return false;
        default:
            if (isdigit(c)) {
                t->intvalue = scanInt(this, c);
                t->token = T_INTLIT;
                break;
            } else if (isalpha(c) || c == '_') {
                scanIdent(this, c);

                if ((tokenType = keyword(this->text))) {
                    if (tokenType == T_GOTO) printf("Found goto\n");
                    t->token = tokenType;
                    break;
                }

                t->token = T_IDENT;
                break;
            }

            // occurs only probs when unicode
            fprintf(stderr, "Error: Invalid character %c on line %d\n", c,
                    this->line);
            exit(-1);
    }
    return true;
}

static int keyword(char *s) {
    switch (*s) {
        case 'e':
            if (!strcmp(s, "else")) return T_ELSE;
        case 'g':
            if (!strcmp(s, "goto")) return T_GOTO;
        case 'i':
            if (!strcmp(s, "input")) return T_INPUT;
            if (!strcmp(s, "int")) return T_INT;
            if (!strcmp(s, "if")) return T_IF;
        case 'l':
            if (!strcmp(s, "label")) return T_LABEL;
        case 'p':
            if (!strcmp(s, "print")) return T_PRINT;
        case 'w':
            if (!strcmp(s, "while")) return T_WHILE;
        }
    return 0;
}

static int scanIdent(Scanner this, int c) {
    int i = 0;

    while (isalpha(c) || isdigit(c) || c == '_') {
        if (i == TEXTLEN - 1) {
            fprintf(stderr, "Error: Identifier too long on line %d\n",
                    this->line);
            exit(-1);
        } else if (i < TEXTLEN - 1) {
            this->text[i++] = c;
        }
        c = next(this);
    }

    putback(this, c);
    this->text[i] = '\0';

    return i;
}

static int scanInt(Scanner this, char c) {
    int i, val = 0;

    while ((i = chrpos("0123456789", c)) >= 0) {
        val = val * 10 + i;
        c = next(this);
    }

    putback(this, c);
    return val;
}

static int chrpos(char *s, int c) {
    char *p;

    p = strchr(s, c);
    return p ? p - s : -1;
}