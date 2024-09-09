#include <ctype.h>
#include <string.h>

#include "scan.h"
#include "defs.h"
#include "misc.h"

static char *TokStr[] = {
    "<EOF>",  "=",      "+=",       "-=",         "*=",    "/=",      "%%=",
    "?",      "||",     "&&",       "|",          "^",     "&",       "==",
    "!=",     "<",      ">",        "<=",         ">=",    "<<",      ">>",
    "+",      "-",      "*",        "/",          "%%",    "++",      "--",
    "~",      "!",      "intlit",   "void",       "i8",    "i32",     "print",
    "input",  "peek",   "poke",     "if",         "else",  "label",   "goto",
    "while",  "for",    "return",   "struct",     "union", "enum",    "typedef",
    "extern", "break",  "continue", "switch",     "case",  "default", "sizeof",
    "static", "strlit", ";",        "identifier", "{",     "}",       "(",
    ")",      "[",      "]",        ",",          ".",     "->",      ":"};

static char next(Scanner);
static void putback(Scanner, char c);
static char skip(Scanner self);

static int scanIdent(Scanner self, char c);
static int scanChr(Scanner self);
static int scanInt(Scanner, char c);
static int scanStr(Scanner self);
static int hexChar(Scanner self);
static int keyword(char *s);

static int chrpos(char *s, int c);

static char next(Scanner self) {
    char c;
    if (self->putback) {
        c = self->putback;
        self->putback = 0;
        return c;
    }
    c = fgetc(self->infile);
    if (c == '\n') self->line++;
    return c;
}

// Con: one character buffer we cant really read 3 symbol characters only 2
static void putback(Scanner self, char c) { self->putback = c; }

void Scanner_Putback(Scanner self, char c) { putback(self, c); }

// ignores whitespace
static char skip(Scanner self) {
    char c;
    c = next(self);

    // please work

    // * comments are handled by preprocessor

    while (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
           // \f not really used much anymore
           c == '\f') {
        c = next(self);
        // debug("fuck before");
    }
    return c;
}

void Scanner_Scan(Scanner self, Token t) {
    char c, tokenType;

    if (self->rejToken) {
        // might be buggy check later
        t->token = self->rejToken->token;
        t->intvalue = self->rejToken->intvalue;
        self->rejToken = nullptr;
        return;
    }

    c = skip(self);

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
            if ((c = next(self)) == '+') {
                t->token = T_INC;
            } else if (c == '=') {
                t->token = T_ASPLUS;
            } else {
                putback(self, c);
                t->token = T_PLUS;
            }
            break;
        case '-':
            if ((c = next(self)) == '-') {
                t->token = T_DEC;
            } else if (c == '=') {
                t->token = T_ASMINUS;
            } else if (c == '>') {
                t->token = T_ARROW;
            } else if (isdigit(c)) {
                t->intvalue = -scanInt(self, c);
                t->token = T_INTLIT;
            } else {
                putback(self, c);
                t->token = T_MINUS;
            }
            break;
        case '*':
            if ((c = next(self)) == '=') {
                t->token = T_ASSTAR;
            } else {
                putback(self, c);
                t->token = T_STAR;
            }
            break;
        case '/':
            if ((c = next(self)) == '=') {
                t->token = T_ASSLASH;
            } else {
                putback(self, c);
                t->token = T_SLASH;
            }
            break;
        case '%':
            if ((c = next(self)) == '=') {
                t->token = T_ASMOD;
            } else {
                putback(self, c);
                t->token = T_MODULO;
            }
            break;
        case '=':
            if ((c = next(self)) == '=') {
                t->token = T_EQ;
            } else {
                putback(self, c);
                t->token = T_ASSIGN;
            }
            break;
        case '!':
            if ((c = next(self)) == '=') {
                t->token = T_NE;
            } else {
                putback(self, c);
                t->token = T_LOGNOT;
            }
            break;
        case '<':
            if ((c = next(self)) == '=') {
                t->token = T_LE;
            } else if (c == '<') {
                t->token = T_LSHIFT;
            } else {
                putback(self, c);
                t->token = T_LT;
            }
            break;
        case '>':
            if ((c = next(self)) == '=') {
                t->token = T_GE;
            } else if (c == '>') {
                t->token = T_RSHIFT;
            } else {
                putback(self, c);
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
            if ((c = next(self)) == '&') {
                t->token = T_LOGAND;
            } else {
                putback(self, c);
                t->token = T_AMPER;
            }
            break;
        case '^':
            t->token = T_XOR;
            break;
        case '|':
            if ((c = next(self)) == '|') {
                t->token = T_LOGOR;
            } else {
                putback(self, c);
                t->token = T_OR;
            }
            break;
        case '\'':
            t->intvalue = scanChr(self);
            t->token = T_INTLIT;
            if (next(self) != '\'') {
                lfatal(self, "SyntaxError: expected ' at end of char literal");
            }
            break;
        case '"':
            scanStr(self);
            t->token = T_STRLIT;
            break;
        case '.':
            t->token = T_DOT;
            break;
        case ':':
            t->token = T_COLON;
            break;
        case '?':
            t->token = T_QUESTION;
            break;
        default:
            if (isdigit(c)) {
                t->intvalue = scanInt(self, c);
                t->token = T_INTLIT;
                break;
            } else if (isalpha(c) || c == '_') {
                scanIdent(self, c);

                if ((tokenType = keyword(self->text))) {
                    t->token = static_cast<enum OPCODES>(tokenType);
                    break;
                }

                t->token = T_IDENT;
                break;
            }

            // occurs only probs when unicode
            lfatala(self, "SyntaxError: Invalid character %c", c);
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
            if (!strcmp(s, "static")) return T_STATIC;
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

void Scanner_RejectToken(Scanner self, Token t) {
    if (self->rejToken) {
        lfatal(self, "InternalError: Cannot reject token twice\n");
    }
    self->rejToken = t;
}

static int scanIdent(Scanner self, char c) {
    int i = 0;

    while (isalpha(c) || isdigit(c) || c == '_') {
        if (i == TEXTLEN - 1) {
            lfatal(self, "UnsupportedError: Identifier too long");
        } else if (i < TEXTLEN - 1) {
            self->text[i++] = c;
        }
        c = next(self);
    }

    putback(self, c);
    self->text[i] = '\0';

    return i;
}

static int scanInt(Scanner self, char c) {
    int i, val = 0, base = 10;

    if (c == '0') {
        if ((c = next(self)) == 'x') {
            base = 16;
            c = next(self);
        } else {
            base = 8;
        }
    }

    while ((i = chrpos("0123456789abcdef", tolower(c))) >= 0) {
        if (i >= base) {
            lfatal(self, "SyntaxError: Invalid digit in number");
        }
        val = val * base + i;
        c = next(self);
    }

    putback(self, c);
    return val;
}

static int hexChar(Scanner self) {
    bool sawHex = false;
    int c, h, n = 0;

    while (isxdigit(c = next(self))) {
        h = chrpos("0123456789abcdef", tolower(c));
        n = n * 16 + h;
        sawHex = true;
    }
    putback(self, c);
    if (!sawHex) {
        lfatal(self, "SyntaxError: Missing digits after \\x");
    } else if (n > 255) {
        lfatal(self, "SyntaxError: Hex value out of range after \\x");
    }

    return n;
}

static int scanChr(Scanner self) {
    char c = next(self), c2;
    if (c == '\\') {
        switch (c = next(self)) {
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
                for (int i = c2 = 0; isdigit(c) && c < '8'; c = next(self)) {
                    if (++i > 3) break;
                    c2 = c2 * 8 + (c - '0');
                }
                putback(self, c);
                return c2;
            case 'x':
                return hexChar(self);
            default:
                lfatal(self, "SyntaxError: Unknown escape sequence");
        }
    }
    return c;
}

static int scanStr(Scanner self) {
    char c;
    for (int i = 0; i < TEXTLEN - 1; i++) {
        c = next(self);
        if (c == EOF) {
            lfatal(self, "SyntaxError: EOF in string");
        }
        if (c == '"') {
            self->text[i] = '\0';
            return i;
        }
        self->text[i] = c;
    }
    lfatal(self, "UnsupportedError: String too long");
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

scanner::~scanner() {
    pclose(this->infile);
}
