#include "decl.h"

void var_declare(Compiler c, Scanner s, SymTable st, Token tok, enum ASTPRIM type, bool isLocal) {
    //* int x[2], a;
    // TODO : Support array initialisation
    enum STORECLASS store = isLocal ? C_LOCAL : C_GLOBAL;

    while (true) {
        if (tok->token == T_LBRACKET) {
            Scanner_Scan(s, tok);
            if (tok->token == T_INTLIT) {
                SymTable_Add(st, c, s, pointer_to(type), S_ARRAY, store,
                                 tok->intvalue, false);
            } else {
                SymTable_Add(st, NULL, s, pointer_to(type), S_VAR, store, 1, false);
            }
            Scanner_Scan(s, tok);
            match(s, tok, T_RBRACKET, "]");
        } else if (tok->token == T_ASSIGN) {
            if (isLocal) {
                fprintf(stderr, "Error: local variables cannot be initialised\n");
                exit(-1);
            }

            Scanner_Scan(s, tok);

            // Only for scalar types
            int id = SymTable_Add(st, NULL, s, type, S_VAR, store, 1, false);
            // For now until lazy evaluation is implemented
            // we stick with singular values

            // initialisation weird with local vars - doesnt work as of yet
            if (tok->token == T_INTLIT) {
                SymTable_SetValue(st, id, tok->intvalue);
            } else {
                fprintf(stderr, "Error: only integer literals are supported\n");
                exit(-1);
            }
            Scanner_Scan(s, tok);
        } else {
            SymTable_Add(st,c,  s, type, S_VAR, store, 1, false);
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
            var_declare(c, s, st, tok, type, false);
        }
        if (tok->token == T_EOF) break;
    }
}

ASTnode function_declare(Compiler c, Scanner s, SymTable st, Token tok,
                         Context ctx, enum ASTPRIM type) {
    int id = SymTable_Add(st, NULL, s, type, S_FUNC, C_GLOBAL, 1, false);

    Context_SetFunctionId(ctx, id);

    ASTnode tree, finalstmt;

    Compiler_ResetLocals(c);

    lparen(s, tok);
    rparen(s, tok);

    tree = Compound_Statement(c, s, st, tok, ctx);
    printf("the local offset is %d\n", c->localOffset);
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
