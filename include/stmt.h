#ifndef STMT_H
#define STMT_H

#include "scan.h"
#include "gen.h"
#include "sym.h"
#include "tokens.h"
#include "decl.h"

ASTnode Compound_Statement(Scanner s, SymTable st, Token tok);

void match(Scanner s, Token t, enum OPCODES op, char *tok);
void semi(Scanner s, Token t);
void ident(Scanner s, Token t);
void lbrace(Scanner s, Token t);
void rbrace(Scanner s, Token t);
void lparen(Scanner s, Token t);
void rparen(Scanner s, Token t);

#endif