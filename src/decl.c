#include "decl.h"

void var_declare(Scanner s, SymTable st, Token tok) {
    match(s, tok, T_INT, "int");

    ident(s, tok);

    SymTable_GlobAdd(st, s);
    semi(s, tok);
    // * .comm written is supposed to be here but it will be
    // * deferred
}

ASTnode function_declare(Scanner s, SymTable st, Token tok) {
    match(s, tok, T_VOID, "void");

    ident(s, tok);
    int id = SymTable_GlobAdd(st, s);
    SymTable_GlobSetFunc(st, id);
    
    lparen(s, tok);
    rparen(s, tok);

    ASTnode tree = Compound_Statement(s, st, tok);
    return ASTnode_NewUnary(A_FUNCTION, tree, id);
}