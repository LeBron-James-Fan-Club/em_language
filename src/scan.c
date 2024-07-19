#include "scan.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static char next(Scanner);
static void putback(Scanner, char c);
static char skip(Scanner this);

static int scanIdent(Scanner this, char c);
static int scanChr(Scanner this);
static int scanInt(Scanner, char c);
static int scanHex(Scanner, char c);
static int scanStr(Scanner this);
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

    if (this->rejToken) {
        // might be buggy check later
        t->token = this->rejToken->token;
        t->intvalue = this->rejToken->intvalue;
        this->rejToken = NULL;
        if (t->token == T_EOF || t->token == T_SEMI || t->token == T_RPAREN ||
            t->token == T_RBRACKET)
            return false;
        return true;
    }

    c = skip(this);

#if DEBUG
    printf("Scanning: %c\n", c);
#endif
    switch (c) {
        case EOF:
            t->token = T_EOF;
            return false;
        case ';':
            // equv to eof
            // Need to manage putback of characters
            t->token = T_SEMI;
            return false;
        case '+':
            if ((c = next(this)) == '+') {
                t->token = T_INC;
            } else {
                putback(this, c);
                t->token = T_PLUS;
            }
            break;
        case '-':
            if ((c = next(this)) == '-') {
                t->token = T_DEC;
            } else {
                putback(this, c);
                t->token = T_MINUS;
            }
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
            } else if (c == '<') {
                t->token = T_LSHIFT;
            } else {
                putback(this, c);
                t->token = T_LT;
            }
            break;
        case '>':
            if ((c = next(this)) == '=') {
                t->token = T_GE;
            } else if (c == '>') {
                t->token = T_RSHIFT;
            } else {
                putback(this, c);
                t->token = T_GT;
            }
            break;
        case '{':
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
            // TODO WHEN WE USE PARENS - fixed? (its kinda shitty tho)
            t->token = T_RPAREN;
            return false;
        case '[':
            t->token = T_LBRACKET;
            break;
        case ']':
#if DEBUG
            printf("RBRACKET\n");
#endif
            t->token = T_RBRACKET;
            return false;
        case ',':
            t->token = T_COMMA;
            return false;
        case '&':
            if ((c = next(this)) == '&') {
                t->token = T_LOGAND;
            } else {
                putback(this, c);
                t->token = T_AMPER;
            }
            break;
        case '^':
            t->token = T_XOR;
            break;
        case '|':
            if ((c = next(this)) == '|') {
                t->token = T_LOGOR;
            } else {
                putback(this, c);
                t->token = T_OR;
            }
            break;
        case '\'':
            t->intvalue = scanChr(this);
            t->token = T_INTLIT;
            if (next(this) != '\'') {
                fprintf(
                    stderr,
                    "Error: Expected \' at end of char literal on line %d\n",
                    this->line);
                exit(-1);
            }
            break;
        case '"':
            scanStr(this);
            t->token = T_STRLIT;
            break;
        default:
            if (isdigit(c)) {
                if (c == '0') {
                    c = next(this);
                    if (c == 'x') {
                        c = next(this);
                        t->intvalue = scanHex(this, c);
                    } else {
                        putback(this, c);
                        t->intvalue = scanInt(this, c);
                    }
                } else {
                    t->intvalue = scanInt(this, c);
                }
                t->token = T_INTLIT;
                break;
            } else if (isalpha(c) || c == '_') {
                scanIdent(this, c);

                if ((tokenType = keyword(this->text))) {
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
        case 'c':
            if (!strcmp(s, "char")) return T_CHAR;
        case 'f':
            if (!strcmp(s, "for")) return T_FOR;
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
            if (!strcmp(s, "poke")) return T_POKE;
            if (!strcmp(s, "peek")) return T_PEEK;
        case 'r':
            if (!strcmp(s, "return")) return T_RETURN;
        case 'v':
            if (!strcmp(s, "void")) return T_VOID;
        case 'w':
            if (!strcmp(s, "while")) return T_WHILE;
    }
    return 0;
}

void Scanner_RejectToken(Scanner this, Token t) {
    if (this->rejToken) {
        fprintf(stderr,
                "Error: Cannot reject token twice, occured on line %d\n",
                this->line);
        exit(-1);
    }
    this->rejToken = t;
}

static int scanIdent(Scanner this, char c) {
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

static int scanHex(Scanner this, char c) {
    int i, val = 0;

    while ((i = chrpos("0123456789ABCDEF", toupper(c))) >= 0) {
        val = val * 16 + i;
        c = next(this);
    }

    putback(this, c);
    return val;
}

static int scanChr(Scanner this) {
    char c = next(this);
    if (c == '\\') {
        switch (c = next(this)) {
            case 'n':
                return '\n';
            case 't':
                return '\t';
            case 'r':
                return '\r';
            case 'f':
                return '\f';
            case 'b':
                return '\b';
            case '\\':
                return '\\';
            case '\'':
                return '\'';
            case '\"':
                return '\"';
            default:
                fprintf(stderr,
                        "Error: Unknown escape sequence %c on line %d\n", c,
                        this->line);
                exit(-1);
        }
    }
    return c;
}

static int scanStr(Scanner this) {
    char c;
    for (int i = 0; i < TEXTLEN - 1; i++) {
        c = next(this);
        if (c == EOF) {
            fprintf(stderr, "Error: EOF in string on line %d\n", this->line);
            exit(-1);
        }
        if (c == '"') {
            this->text[i] = '\0';
            return i;
        }
        this->text[i] = c;
    }
    fprintf(stderr, "Error: String too long on line %d\n", this->line);
    exit(-1);
}

static int chrpos(char *s, int c) {
    char *p;

    p = strchr(s, c);
    return p ? p - s : -1;
}

void match(Scanner s, Token t, enum OPCODES op, char *tok) {
    if (t->token == op) {
        Scanner_Scan(s, t);
    } else {
        fprintf(stderr, "Error: %s expected on line %d\n", tok, s->line);
        fprintf(stderr, "got instead %d\n", t->token);
        exit(-1);
    }
}

void semi(Scanner s, Token t) { match(s, t, T_SEMI, ";"); }

void ident(Scanner s, Token t) { match(s, t, T_IDENT, "identifier"); }

void lbrace(Scanner s, Token t) { match(s, t, T_LBRACE, "{"); }

void rbrace(Scanner s, Token t) { match(s, t, T_RBRACE, "}"); }

void lparen(Scanner s, Token t) { match(s, t, T_LPAREN, "("); }

void rparen(Scanner s, Token t) { match(s, t, T_RPAREN, ")"); }