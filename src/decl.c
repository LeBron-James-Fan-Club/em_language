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

void global_declare(Compiler c, Scanner s, SymTable st, Token tok,
                    Context ctx) {
    ASTnode tree;
    enum ASTPRIM type;

    while (true) {
        type = parse_type(s, tok);
        ident(s, tok);
        if (tok->token == T_LPAREN) {
            tree = function_declare(c, s, st, tok, ctx, type);
            Compiler_Gen(c, st, ctx, tree);
        } else {
            var_declare(s, st, tok);
        }
        if (tok->token == T_EOF) break;
    }
}

ASTnode function_declare(Compiler c, Scanner s, SymTable st, Token tok,
                         Context ctx, enum ASTPRIM type) {
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
    enum ASTOP type;
    switch (tok->token) {
        case T_INT:
            type = P_INT;
            break;
        case T_CHAR:
            type = P_CHAR;
            break;
        case T_VOID:
            type = P_VOID;
            break;
        default:
            fprintf(stderr, "Error: unknown type\n");
            exit(-1);
    }

    // allows the user to do int ******a
    while (true) {
        Scanner_Scan(s, tok);
        if (tok->token != T_STAR) break;
        type = pointer_to(type);
    }
    Scanner_RejectToken(s, tok);

    return type;
}
