#include "scan.h"

#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include "defs.h"
#include "misc.h"

static char *TokStr[] = {
    "<EOF>",   "=",      "+=",     "-=",       "*=",         "/=",    "%=",
    "?",       "||",     "&&",     "|",        "^",          "&",     "==",
    "!=",      "<",      ">",      "<=",       ">=",         "<<",    ">>",
    "+",       "-",      "*",      "/",        "%",          "++",    "--",
    "~",       "!",      "intlit", "void",     "i8",         "i32",   "print",
    "input",   "peek",   "poke",   "exit",     "if",         "else",  "label",
    "goto",    "while",  "for",    "return",   "struct",     "union", "enum",
    "typedef", "extern", "break",  "continue", "switch",     "case",  "default",
    "sizeof",  "static", "strlit", ";",        "identifier", "{",     "}",
    "(",       ")",      "[",      "]",        ",",          ".",     "->",
    ":"};

Scanner Scanner_New(void) {
    Scanner n = calloc(1, sizeof(struct scanner));
    n->line = 1;
    n->putback = '\n';

    return n;
}

void Scanner_Free(Scanner this) {
    em_scanner_free(this->em_scanner);
    free(this);
}

void Scanner_Scan(Scanner this, Token t) {
    if (this->hasRejectedToken) {
        // might be buggy check later
        *t = this->rejToken;
        this->hasRejectedToken = false;
        return;
    }

    *t = em_scanner_next(this->em_scanner);

    if (t->token == T_IDENT || t->token == T_STRLIT) {
        strcpy(this->text, t->tokstr);
    }

    if (t->token == T_SEMI) {
        this->comment[this->commentLen] = '\0';
        this->commentLen = 0;
    } else {
        for (int i = 0; i < strlen(t->tokstr); i++) {
            this->comment[this->commentLen++] = t->tokstr[i];
        }
        this->comment[this->commentLen++] = ' ';
    }

    debug("\t\t\t\t\t scan %d ('%s'): '%s' ('%s')", t->token, TokStr[t->token], t->tokstr, this->text);
}

void Scanner_EndComment(Scanner this) {
    this->comment[this->commentLen] = '\0';
    this->commentLen = 0;
}

void Scanner_RejectToken(Scanner this, Token t) {
    if (this->hasRejectedToken) {
        lfatal(this, "InternalError: Cannot reject token twice\n");
    }
    this->rejToken = *t;
    this->hasRejectedToken = true;
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

void lbracket(Scanner s, Token t) { match(s, t, T_LBRACKET, "["); }

void rbracket(Scanner s, Token t) { match(s, t, T_RBRACKET, "]"); }

void comma(Scanner s, Token t) { match(s, t, T_COMMA, ","); }

int decode_char_literal(const char *input) {
    if (input[2] == '\'') {
        return input[1];
    }

    switch (input[2]) {
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
        case 'a':
            return '\a';
        case 'v':
            return '\v';
        case '\\':
            return '\\';
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        case '?':
            return '\?';
        case '0' ... '7': {
            int value = 0;

            for (int i = 1; input[i] != '\''; i++) {
                value *= 8;
                value += input[i] - '0';
            }

            return value;
        }
        case 'x': {
            int value = 0;

            for (int i = 2; isxdigit(input[i]); i++) {
                value *= 16;
                value += isdigit(input[i]) ? (input[i] - '0') : (tolower(input[i]) - 'a' + 10);
            }

            return value;
        }
        default:
            return '\0'; // invalid
    }
}
