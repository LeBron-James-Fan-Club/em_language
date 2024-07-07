#include "decl.h"

void var_declare(Scanner s, SymTable st, Token tok) {
    enum ASTPRIM type = parse_type(s, tok);

    Scanner_Scan(s, tok);
    ident(s, tok);

    SymTable_GlobAdd(st, s, type, S_VAR);
    semi(s, tok);
    // * .comm written is supposed to be here but it will be
    // * deferred
}

ASTnode function_declare(Compiler c, Scanner s, SymTable st, Token tok,
                         Context ctx) {

    int type = parse_type(s, tok);
    Scanner_Scan(s, tok);
    ident(s, tok);

    // Might change later to use names instead of numbered labels
    int id = SymTable_GlobAdd(st, s, type, S_FUNC);

    Context_SetFunctionId(ctx, id);

    ASTnode tree, finalstmt;

    lparen(s, tok);
    rparen(s, tok);

    tree = Compound_Statement(s, st, tok, ctx);
    if (type != P_VOID) {
        finalstmt = (tree->op == A_GLUE) ? tree->right : tree;
        if (finalstmt == NULL || finalstmt->op != A_RETURN) {
            fprintf(stderr, "Error: non-void function must return a value\n");
            exit(-1);
        }
    }

    return ASTnode_NewUnary(A_FUNCTION, P_VOID, tree, id);
}

enum ASTOP parse_type(Scanner s, Token tok) {
    switch (tok->token) {
        case T_INT:
            return P_INT;
        case T_CHAR:
            return P_CHAR;
        case T_VOID:
            return P_VOID;
        default:
            fprintf(stderr, "Error: unknown type\n");
            exit(-1);
    }
}
