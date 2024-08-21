#include "decl.h"

#include "flags.h"
#include "misc.h"


static int param_declare(Scanner s, SymTable st, Token tok, int id);

void var_declare(Scanner s, SymTable st, Token tok,
                 enum ASTPRIM type, enum STORECLASS store) {
    //* int x[2], a;

    // TODO : Support array initialisation
    // TODO : Support multi-dimensional arrays

    while (true) {
        if (tok->token == T_LBRACKET) {
            //! Unsure about this for local variables
            //! I don't think i calculated the offset for the size yet

            Scanner_Scan(s, tok);
            if (tok->token == T_INTLIT) {
                // temp til implemented
                if (store == C_LOCAL) {
                    lfatal(s, "UnsupportedError: local variables cannot be arrays");
                }

                SymTable_Add(st, s, pointer_to(type), S_ARRAY, store,
                             tok->intvalue, false);
            } else {
                SymTable_Add(st, s, pointer_to(type), S_VAR, store, 1,
                             false);
            }
            Scanner_Scan(s, tok);
            match(s, tok, T_RBRACKET, "]");
        } else if (tok->token == T_ASSIGN) {
            if (store == C_LOCAL || store == C_PARAM) {
                lfatal(s, "UnsupportedError: local variables and parameters cannot be initialised");
            }

            Scanner_Scan(s, tok);

            // Only for scalar types
            int id = SymTable_Add(st, s, type, S_VAR, store, 1, false);
            // For now until lazy evaluation is implemented
            // we stick with singular values

            // initialisation weird with local vars - doesnt work as of yet

            //! implemented initialiation for local vars
            //! but disabled for now just in case

            if (tok->token == T_INTLIT) {
                SymTable_SetValue(st, id, tok->intvalue);
            } else {
                lfatal(s, "UnsupportedError: only integer literals are supported");
            }
            Scanner_Scan(s, tok);

            // TODO : check if there is wrong syntax?
            // TODO: NVM it does it below
        } else {
            if (store == C_LOCAL) {
                debug("Adding local variable %s", s->text);
                if (SymTable_Add(st, s, type, S_VAR, store, 1, false) ==
                    -1) {
                    lfatala(s, "DuplicateError: Duplicate local variable %s", s->text);
                }
            } else {
                SymTable_Add(st, s, type, S_VAR, store, 1, false);
            }
        }

        if (store == C_PARAM &&
            (tok->token == T_COMMA || tok->token == T_RPAREN)) {
            return;
        }

        if (tok->token == T_SEMI || tok->token == T_EOF) {
            Scanner_Scan(s, tok);
            return;
        }
        if (tok->token != T_COMMA) {
            lfatala(s, "SyntaxError: expected comma got %d", tok->token);
        }
        Scanner_Scan(s, tok);
        ident(s, tok);
    }
}

static int param_declare(Scanner s, SymTable st, Token tok,
                         int id) {
    enum ASTPRIM type;
    int paramId;
    int origParamCnt;
    int paramCnt = 0;

    paramId = id + 1;
    if (paramId) origParamCnt = st->Gsym[id].nElems;

    while (tok->token != T_RPAREN) {
        type = parse_type(s, tok);
        ident(s, tok);

        if (paramId) {
            if (type != st->Gsym[id].type) {
                lfatal(s, "InvalidParamsError: parameter type mismatch for proto");
            }
            paramId++;
        }

        var_declare(s, st, tok, type, C_PARAM);
        paramCnt++;

        switch (tok->token) {
            case T_COMMA:
                Scanner_Scan(s, tok);
                break;
            case T_RPAREN:
                break;
            default:
                lfatal(s, "SyntaxError: expected comma or right parenthesis");
        }
    }

    if (id != -1 && paramCnt != origParamCnt) {
        lfatal(s, "InvalidParamsError: parameter count mismatch for proto");
    }

    return paramCnt;
}

void global_declare(Compiler c, Scanner s, SymTable st, Token tok, Context ctx) {
    ASTnode tree;
    enum ASTPRIM type;

    while (true) {
        SymTable_ResetLocls(st);

        type = parse_type(s, tok);
        ident(s, tok);
        if (tok->token == T_LPAREN) {
            tree = function_declare(c, s, st, tok, ctx, type);

            // Proto is a no go
            if (tree == NULL) {
                continue;
            }

            if (flags.dumpAST) {
                ASTnode_Dump(tree, st, NO_LABEL, 0);
            }

            Compiler_Gen(c, st, ctx, tree);
            ASTnode_Free(tree);

        } else {
            var_declare(s, st, tok, type, C_GLOBAL);
        }
        if (tok->token == T_EOF) break;
    }
}

ASTnode function_declare(Compiler c, Scanner s, SymTable st, Token tok,
                         Context ctx, enum ASTPRIM type) {

    int id, nameSlot, paramCnt;
    ASTnode tree, finalstmt;

    if ((id = SymTable_Find(st, s, S_FUNC)) != -1) {
        if (st->Gsym[id].stype != S_FUNC) {
            id = -1;
        }
    }

    if (id == -1) {
        nameSlot = SymTable_Add(st, s, type, S_FUNC, C_GLOBAL, 1, false);
    }

    lparen(s, tok);
    paramCnt = param_declare(s, st, tok, id);
    //printf("param count is %d\n", paramCnt);
    rparen(s, tok);

    if (id == -1) {
        st->Gsym[nameSlot].nElems = paramCnt;
    }

    // This is only a proto
    if (tok->token == T_SEMI) {
//        printf("proto\n");
        Scanner_Scan(s, tok);
        return NULL;
    }

    if (id == -1) {
        id = nameSlot;
    }

    ctx->functionId = id;

    SymTable_CopyFuncParams(st, id);

    // printf("after params\n");

    tree = Compound_Statement(c, s, st, tok, ctx);
    // printf("the local offset is %d\n", c->localOffset);
    if (type != P_VOID) {
        finalstmt = (tree->op == A_GLUE) ? tree->right : tree;
        if (finalstmt == NULL || finalstmt->op != A_RETURN) {
            fatal("NoReturnError: non-void function must return a value\n");
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
            fatala("InternalError: unknown type %d", tok->token);
    }

    // allows the user to do int ******a
    while (true) {
        Scanner_Scan(s, tok);
        if (tok->token != T_STAR) break;
        type = pointer_to(type);
    }

    return type;
}
