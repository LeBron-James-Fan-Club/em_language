#include "scan.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "defs.h"
#include "misc.h"

static char *TokStr[] = {
    "<EOF>", "=",          "+=",     "-=",    "*=",      "/=",      "%%=",
    "||",    "&&",         "|",      "^",     "&",       "==",      "!=",
    "<",     ">",          "<=",     ">=",    "<<",      ">>",      "+",
    "-",     "*",          "/",      "%%",    "++",      "--",      "~",
    "!",     "intlit",     "void",   "i8",    "i32",     "print",   "input",
    "peek",  "poke",       "if",     "else",  "label",   "goto",    "while",
    "for",   "return",     "struct", "union", "enum",    "typedef", "extern",
    "break", "continue",   "switch", "case",  "default", "sizeof",  "strlit",
    ";",     "identifier", "{",      "}",     "(",       ")",       "[",
    "]",     ",",          ".",      "->",    ":"};

static char next(Scanner);
static void putback(Scanner, char c);
static char skip(Scanner this);

static int scanIdent(Scanner this, char c);
static int scanChr(Scanner this);
static int scanInt(Scanner, char c);
static int scanStr(Scanner this);
static int hexChar(Scanner this);
static int keyword(char *s);

static int chrpos(char *s, int c);

Scanner Scanner_New(void) {
    Scanner n = calloc(1, sizeof(struct scanner));
    n->line = 1;
    n->putback = '\n';

    return n;
}

void Scanner_Free(Scanner this) {
    pclose(this->infile);
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

    // please work

    // * comments are handled by preprocessor

    while (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
           // \f not really used much anymore
           c == '\f') {
        c = next(this);
        // debug("fuck before");
    }
    return c;
}

void Scanner_Scan(Scanner this, Token t) {
    char c, tokenType;

    if (this->rejToken) {
        // might be buggy check later
        t->token = this->rejToken->token;
        t->intvalue = this->rejToken->intvalue;
        this->rejToken = NULL;
        return;
    }

    c = skip(this);

    debug("scanning %c", c);

    switch (c) {
        case EOF:
            t->token = T_EOF;
            debug("<EOF>");
            break;
        case ';':
            // equv to eof
            // Need to manage putback of characters
            t->token = T_SEMI;
            break;
        case '+':
            if ((c = next(this)) == '+') {
                t->token = T_INC;
            } else if (c == '=') {
                t->token = T_ASPLUS;
            } else {
                putback(this, c);
                t->token = T_PLUS;
            }
            break;
        case '-':
            if ((c = next(this)) == '-') {
                t->token = T_DEC;
            } else if (c == '=') {
                t->token = T_ASMINUS;
            } else if (c == '>') {
                t->token = T_ARROW;
            } else if (isdigit(c)) {
                t->intvalue = -scanInt(this, c);
                t->token = T_INTLIT;
            } else {
                putback(this, c);
                t->token = T_MINUS;
            }
            break;
        case '*':
            if ((c = next(this)) == '=') {
                t->token = T_ASSTAR;
            } else {
                putback(this, c);
                t->token = T_STAR;
            }
            break;
        case '/':
            if ((c = next(this)) == '=') {
                t->token = T_ASSLASH;
            } else {
                putback(this, c);
                t->token = T_SLASH;
            }
            break;
        case '%':
            if ((c = next(this)) == '=') {
                t->token = T_ASMOD;
            } else {
                putback(this, c);
                t->token = T_MODULO;
            }
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
                putback(this, c);
                t->token = T_LOGNOT;
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
            t->token = T_RPAREN;
            t->tokstr = TokStr[t->token];
            break;
        case '[':
            t->token = T_LBRACKET;
            break;
        case ']':
            t->token = T_RBRACKET;
            t->tokstr = TokStr[t->token];
            break;
        case ',':
            t->token = T_COMMA;
            t->tokstr = TokStr[t->token];
            break;
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
                lfatal(this, "SyntaxError: expected ' at end of char literal");
            }
            break;
        case '"':
            scanStr(this);
            t->token = T_STRLIT;
            break;
        case '.':
            t->token = T_DOT;
            break;
        case ':':
            t->token = T_COLON;
            break;
        default:
            if (isdigit(c)) {
                t->intvalue = scanInt(this, c);
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
            lfatala(this, "SyntaxError: Invalid character %c", c);
    }

    t->tokstr = TokStr[t->token];
}

static int keyword(char *s) {
    debug("keyword %s", s);
    switch (*s) {
        case 'b':
            if (!strcmp(s, "break")) return T_BREAK;
            break;
        case 'c':
            if (!strcmp(s, "case")) return T_CASE;
            if (!strcmp(s, "continue")) return T_CONTINUE;
            break;
        case 'd':
            if (!strcmp(s, "default")) return T_DEFAULT;
            break;
        case 'e':
            if (!strcmp(s, "else")) return T_ELSE;
            if (!strcmp(s, "enum")) return T_ENUM;
            if (!strcmp(s, "extern")) return T_EXTERN;
            break;
        case 'f':
            if (!strcmp(s, "for")) return T_FOR;
            break;
        case 'g':
            if (!strcmp(s, "goto")) return T_GOTO;
            break;
        case 'i':
            if (!strcmp(s, "input")) return T_INPUT;
            if (!strcmp(s, "i32")) return T_INT;
            if (!strcmp(s, "i8")) return T_CHAR;
            if (!strcmp(s, "if")) return T_IF;
            break;
        case 'l':
            if (!strcmp(s, "label")) return T_LABEL;
            break;
        case 's':
            if (!strcmp(s, "sizeof")) return T_SIZEOF;
            if (!strcmp(s, "struct")) return T_STRUCT;
            if (!strcmp(s, "switch")) return T_SWITCH;
            break;
        case 't':
            if (!strcmp(s, "typedef")) return T_TYPEDEF;
            break;
        case 'p':
            if (!strcmp(s, "print")) return T_PRINT;
            if (!strcmp(s, "poke")) return T_POKE;
            if (!strcmp(s, "peek")) return T_PEEK;
            break;
        case 'r':
            if (!strcmp(s, "return")) return T_RETURN;
            break;
        case 'u':
            if (!strcmp(s, "union")) return T_UNION;
            break;
        case 'v':
            if (!strcmp(s, "void")) return T_VOID;
            break;
        case 'w':
            if (!strcmp(s, "while")) return T_WHILE;
            break;
    }
    debug("not keyword %s", s);
    return 0;
}

void Scanner_RejectToken(Scanner this, Token t) {
    if (this->rejToken) {
        lfatal(this, "InternalError: Cannot reject token twice\n");
    }
    this->rejToken = t;
}

static int scanIdent(Scanner this, char c) {
    int i = 0;

    while (isalpha(c) || isdigit(c) || c == '_') {
        if (i == TEXTLEN - 1) {
            lfatal(this, "UnsupportedError: Identifier too long");
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
    int i, val = 0, base = 10;

    if (c == '0') {
        if ((c = next(this)) == 'x') {
            base = 16;
            c = next(this);
        } else {
            base = 8;
        }
    }

    while ((i = chrpos("0123456789abcdef", tolower(c))) >= 0) {
        if (i >= base) {
            lfatal(this, "SyntaxError: Invalid digit in number");
        }
        val = val * base + i;
        c = next(this);
    }

    putback(this, c);
    return val;
}

static int hexChar(Scanner this) {
    bool sawHex = false;
    int c, h, n = 0;

    while (isxdigit(c = next(this))) {
        h = chrpos("0123456789abcdef", tolower(c));
        n = n * 16 + h;
        sawHex = true;
    }
    putback(this, c);
    if (!sawHex) {
        lfatal(this, "SyntaxError: Missing digits after \\x");
    } else if (n > 255) {
        lfatal(this, "SyntaxError: Hex value out of range after \\x");
    }

    return n;
}

static int scanChr(Scanner this) {
    char c = next(this), c2;
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
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                for (int i = c2 = 0; isdigit(c) && c < '8'; c = next(this)) {
                    if (++i > 3) break;
                    c2 = c2 * 8 + (c - '0');
                }
                putback(this, c);
                return c2;
            case 'x':
                return hexChar(this);
            default:
                lfatal(this, "SyntaxError: Unknown escape sequence");
        }
    }
    return c;
}

static int scanStr(Scanner this) {
    char c;
    for (int i = 0; i < TEXTLEN - 1; i++) {
        c = next(this);
        if (c == EOF) {
            lfatal(this, "SyntaxError: EOF in string");
        }
        if (c == '"') {
            this->text[i] = '\0';
            return i;
        }
        this->text[i] = c;
    }
    lfatal(this, "UnsupportedError: String too long");
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
        lfatala(s, "SyntaxError: %s expected got instead %s", tok, t->tokstr);
    }
}

void semi(Scanner s, Token t) { match(s, t, T_SEMI, ";"); }

void ident(Scanner s, Token t) { match(s, t, T_IDENT, "identifier"); }

void lbrace(Scanner s, Token t) { match(s, t, T_LBRACE, "{"); }

void rbrace(Scanner s, Token t) { match(s, t, T_RBRACE, "}"); }

void lparen(Scanner s, Token t) { match(s, t, T_LPAREN, "("); }

void rparen(Scanner s, Token t) { match(s, t, T_RPAREN, ")"); }

void comma(Scanner s, Token t) { match(s, t, T_COMMA, ","); }