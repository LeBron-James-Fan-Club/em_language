#include "decl.h"

static int param_declare(Compiler c, Scanner s, SymTable st, Token tok);

void var_declare(Compiler c, Scanner s, SymTable st, Token tok,
                 enum ASTPRIM type, enum STORECLASS store) {
    //* int x[2], a;
    // TODO : Support array initialisation

    while (true) {
        if (tok->token == T_LBRACKET) {
            Scanner_Scan(s, tok);
            if (tok->token == T_INTLIT) {
                SymTable_Add(st, c, s, pointer_to(type), S_ARRAY, store,
                             tok->intvalue, false);
            } else {
                SymTable_Add(st, NULL, s, pointer_to(type), S_VAR, store, 1,
                             false);
            }
            Scanner_Scan(s, tok);
            match(s, tok, T_RBRACKET, "]");
        } else if (tok->token == T_ASSIGN) {
            if (store == C_LOCAL || store == C_PARAM) {
                fprintf(stderr,
                        "Error: local variables and parameters cannot be initialised\n");
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
            SymTable_Add(st, c, s, type, S_VAR, store, 1, false);
        }
        
        if (store == C_PARAM && (tok->token == T_COMMA || tok->token == T_RPAREN)) {
            return;
        }

        if (tok->token == T_SEMI || tok->token == T_EOF) {
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
            SymTable_ResetLocls(st);

        } else {
            var_declare(c, s, st, tok, type, C_GLOBAL);
        }
        if (tok->token == T_EOF) break;
    }
}

ASTnode function_declare(Compiler c, Scanner s, SymTable st, Token tok,
                         Context ctx, enum ASTPRIM type) {
    int id = SymTable_Add(st, NULL, s, type, S_FUNC, C_GLOBAL, 1, false);

    Context_SetFunctionId(ctx, id);

    ASTnode tree, finalstmt;

    lparen(s, tok);
    int paramCount = param_declare(c, s, st, tok);
    st->Gsym[id].nElems = paramCount;
    rparen(s, tok);

    printf("after params\n");

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

static int param_declare(Compiler c, Scanner s, SymTable st, Token tok) {
    enum ASTPRIM type;
    int paramCount = 0;

    while (tok->token != T_RPAREN) {
        type = parse_type(s, tok);
        ident(s, tok);
        var_declare(c, s, st, tok, type, C_PARAM);
        paramCount++;
        switch (tok->token) {
            case T_COMMA:
                Scanner_Scan(s, tok);
                break;
            case T_RPAREN:
                break;
            default:
                fprintf(stderr, "Error: expected comma or right parenthesis\n");
                exit(-1);
        }
        printf("param count %d\n", paramCount);
    }
    return paramCount;
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
