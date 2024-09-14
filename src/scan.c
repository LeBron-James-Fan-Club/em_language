#include "scan.h"

#include <stdlib.h>

#include "defs.h"
#include "misc.h"

Scanner Scanner_New(void) {
    Scanner n = calloc(1, sizeof(struct scanner));
    n->line = 1;
    return n;
}

void Scanner_Free(Scanner this) {
    free(this);
}

void Scanner_Scan(Scanner this, Token t) {
    t->token = T_EOF;
    t->tokstr = "";
    t->intvalue = 0;
    this->text[0] = '\0';
    this->comment[0] = '\0';
    this->commentLen = 0;
}

void Scanner_EndComment(Scanner this) {
    this->comment[this->commentLen] = '\0';
    this->commentLen = 0;
}

void Scanner_RejectToken(Scanner this, Token t) {
    lfatal(this, "InternalError: Cannot reject token\n");
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
