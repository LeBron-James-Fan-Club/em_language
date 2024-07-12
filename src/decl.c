#include "decl.h"

void var_declare(Scanner s, SymTable st, Token tok, enum ASTPRIM type) {

    //* int x[2], a;
    while (true) {
        if (tok->token == T_LBRACKET) {
            Scanner_Scan(s, tok);
            if (tok->token == T_INTLIT) {
                SymTable_GlobAdd(st, s, pointer_to(type), S_ARRAY,
                                 tok->intvalue);
            }
            Scanner_Scan(s, tok);
            match(s, tok, T_RBRACKET, "]");
        } else {
            SymTable_GlobAdd(st, s, type, S_VAR, 1);
        }
        if (tok->token == T_SEMI) {
            Scanner_Scan(s, tok);
            return;
        }
        if (tok->token != T_COMMA) {
            fprintf(stderr, "Error: comma expected on line %d got %d\n",
                    s->line, tok->token);
            exit(-1);
        }
        Scanner_Scan(s, tok);
        ident(s, tok);
    }
}

void global_declare(Compiler c, Scanner s, SymTable st, Token tok, Context ctx,
                    Flags f) {
    ASTnode tree;
    enum ASTPRIM type;

    while (true) {
        type = parse_type(s, tok);
        ident(s, tok);
        if (tok->token == T_LPAREN) {
            tree = function_declare(c, s, st, tok, ctx, type);
            if (f.dumpAST) {
                ASTnode_Dump(tree, st, NO_LABEL, 0);
            }
            Compiler_Gen(c, st, ctx, tree);
            ASTnode_Free(tree);
        } else {
            var_declare(s, st, tok, type);
        }
        if (tok->token == T_EOF) break;
    }
}

ASTnode function_declare(Compiler c, Scanner s, SymTable st, Token tok,
                         Context ctx, enum ASTPRIM type) {
    // Scanner_Scan(s, tok);
    // ident(s, tok);

    // Might change later to use names instead of numbered labels
    int id = SymTable_GlobAdd(st, s, type, S_FUNC, 1);

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

enum ASTPRIM parse_type(Scanner s, Token tok) {
    enum ASTPRIM type;
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
            fprintf(stderr, "Error: !! unknown type %d\n", tok->token);
            exit(-1);
    }

    // allows the user to do int ******a
    while (true) {
        Scanner_Scan(s, tok);
        if (tok->token != T_STAR) break;
        type = pointer_to(type);
    }

    return type;
}
