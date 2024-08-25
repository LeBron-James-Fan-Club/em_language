#include "decl.h"

#include "flags.h"
#include "misc.h"

static int var_declare_list(Scanner s, SymTable st, Token tok,
                            SymTableEntry funcSym, enum STORECLASS store,
                            enum OPCODES sep, enum OPCODES end);

void var_declare(Scanner s, SymTable st, Token tok, enum ASTPRIM type,
                 SymTableEntry ctype, enum STORECLASS store) {
    //* int x[2], a;

    // TODO : Support array initialisation
    // TODO : Support multi-dimensional arrays

    while (true) {
        if (tok->token == T_LBRACKET) {
            //! Unsure about this for local variables
            //! I don't think i calculated the offset for the size yet

            Scanner_Scan(s, tok);
            if (store == C_LOCAL || store == C_PARAM || store == C_MEMBER) {
                lfatal(s,
                       "UnsupportedError: local, params or member variables "
                       "cannot be arrays");
            }
            SymTable_AddGlob(st, s, pointer_to(type), S_ARRAY, ctype,
                             (tok->token == T_INTLIT) ? tok->intvalue : 1,
                             false);
            Scanner_Scan(s, tok);
            match(s, tok, T_RBRACKET, "]");
        } else if (tok->token == T_ASSIGN) {
            if (store == C_LOCAL || store == C_PARAM || store == C_MEMBER) {
                lfatal(s,
                       "UnsupportedError: local variables, parameters and "
                       "member variables "
                       "cannot be initialised");
            }

            Scanner_Scan(s, tok);

            // Only for scalar types
            SymTableEntry e =
                SymTable_AddGlob(st, s, type, S_VAR, ctype, 1, false);
            // For now until lazy evaluation is implemented
            // we stick with singular values

            // initialisation weird with local vars - doesnt work as of yet

            //! implemented initialiation for local vars
            //! but disabled for now just in case

            if (tok->token == T_INTLIT) {
                SymTable_SetValue(st, e, tok->intvalue);
            } else {
                lfatal(s,
                       "UnsupportedError: only integer literals are supported");
            }
            Scanner_Scan(s, tok);

            // TODO : check if there is wrong syntax?
            // TODO: NVM it does it below
        } else {
            if (store == C_LOCAL) {
                debug("Adding local variable %s", s->text);
                if (SymTable_FindLocl(st, s) != NULL) {
                    lfatala(s, "DuplicateError: Duplicate local variable %s",
                            s->text);
                }
                SymTable_AddLocl(st, s, type, S_VAR, ctype, 1, false);
            } else if (store == C_MEMBER) {
                debug("Adding member variable %s", s->text);
                if (SymTable_FindMember(st, s) != NULL) {
                    lfatala(s, "DuplicateError: Duplicate member variable %s",
                            s->text);
                }
                SymTable_AddMemb(st, s, type, S_VAR, ctype, 1, false);
            } else if (store == C_PARAM) {
                debug("Adding parameter %s", s->text);
                if (SymTable_FindParam(st, s) != NULL) {
                    lfatala(s, "DuplicateError: Duplicate parameter %s",
                            s->text);
                }
                SymTable_AddParam(st, s, type, S_VAR, ctype, 1, false);
            } else {
                debug("Adding global variable %s", s->text);
                if (SymTable_FindGlob(st, s) != NULL) {
                    lfatala(s, "DuplicateError: Duplicate global variable %s",
                            s->text);
                }
                SymTable_AddGlob(st, s, type, S_VAR, ctype, 1, false);
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

static int var_declare_list(Scanner s, SymTable st, Token tok,
                            SymTableEntry funcSym, enum STORECLASS store,
                            enum OPCODES sep, enum OPCODES end) {
    enum ASTPRIM type;
    int paramCnt = 0;

    SymTableEntry protoPtr = NULL;
    SymTableEntry cType;

    // * if a proto, member exists

    if (funcSym != NULL) {
        protoPtr = funcSym->member;
    }

    while (tok->token != end) {
        type = parse_type(s, tok, &cType);
        ident(s, tok);

        if (protoPtr != NULL) {
            if (type != protoPtr->type) {
                lfatal(s,
                       "InvalidParamsError: parameter type mismatch for proto");
            }
            protoPtr = protoPtr->next;
        }

        var_declare(s, st, tok, type, cType, store);
        paramCnt++;

        if (tok->token != sep && tok->token != end) {
            lfatala(s,
                    "SyntaxError: expected comma or right parenthesis got %d",
                    tok->token);
        }
        if (tok->token == sep) {
            Scanner_Scan(s, tok);
        }
    }

    if (funcSym != NULL && paramCnt != funcSym->nElems) {
        lfatal(s, "InvalidParamsError: parameter count mismatch for proto");
    }

    return paramCnt;
}

void global_declare(Compiler c, Scanner s, SymTable st, Token tok,
                    Context ctx) {
    ASTnode tree;
    SymTableEntry cType;
    enum ASTPRIM type;

    while (true) {
        SymTable_ResetLocls(st);

        type = parse_type(s, tok, &cType);
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
            var_declare(s, st, tok, type, cType, C_GLOBAL);
        }
        if (tok->token == T_EOF) break;
    }
}

ASTnode function_declare(Compiler c, Scanner s, SymTable st, Token tok,
                         Context ctx, enum ASTPRIM type) {
    int paramCnt;

    SymTableEntry oldFuncSym, newFuncSym = NULL;

    ASTnode tree, finalstmt;

    if ((oldFuncSym = SymTable_FindSymbol(st, s, ctx)) != NULL) {
        if (oldFuncSym->stype != S_FUNC) {
            oldFuncSym = NULL;
        }
    }

    if (oldFuncSym == NULL) {
        newFuncSym = SymTable_AddGlob(st, s, type, S_FUNC, NULL, 1);
    }

    lparen(s, tok);
    paramCnt = var_declare_list(s, st, tok, C_PARAM, T_COMMA, T_RPAREN);
    // printf("param count is %d\n", paramCnt);
    rparen(s, tok);

    if (newFuncSym) {
        newFuncSym->nElems = paramCnt;
        newFuncSym->member = st->paramHead;
        oldFuncSym = newFuncSym;
    }

    st->paramHead = st->paramTail = NULL;

    // This is only a proto
    if (tok->token == T_SEMI) {
        //        printf("proto\n");
        Scanner_Scan(s, tok);
        return NULL;
    }

    ctx->functionId = oldFuncSym;

    // printf("after params\n");

    tree = Compound_Statement(c, s, st, tok, ctx);
    // printf("the local offset is %d\n", c->localOffset);
    if (type != P_VOID) {
        finalstmt = (tree->op == A_GLUE) ? tree->right : tree;
        if (finalstmt == NULL || finalstmt->op != A_RETURN) {
            fatal("NoReturnError: non-void function must return a value\n");
        }
    }

    return ASTnode_NewUnary(A_FUNCTION, P_VOID, tree, oldFuncSym);
}

enum ASTPRIM parse_type(Scanner s, Token tok, SymTableEntry *ctype) {
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
        case T_STRUCT:
            type = P_STRUCT;
            *ctype = struct_declare(s, tok);
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
