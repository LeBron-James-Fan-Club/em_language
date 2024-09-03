#include "decl.h"

#include "defs.h"
#include "flags.h"
#include "misc.h"

static int var_declare_list(Scanner s, SymTable st, Token tok,
                            SymTableEntry funcSym, enum STORECLASS store,
                            enum OPCODES sep, enum OPCODES end);

static SymTableEntry composite_declare(Scanner s, SymTable st, Token tok,
                                       enum ASTPRIM type);

static void enum_declare(Scanner s, SymTable st, Token tok);

static enum ASTPRIM typedef_declare(Scanner s, SymTable st, Token tok,
                             SymTableEntry *cType);

static enum ASTPRIM typedef_type(Scanner s, SymTable st, Token tok, SymTableEntry *cType);

void var_declare(Scanner s, SymTable st, Token tok, enum ASTPRIM type,
                 SymTableEntry cType, enum STORECLASS store, bool ignoreEnd) {
    //* int x[2], a;

    // TODO : Support array initialisation
    // TODO : Support multi-dimensional arrays

    while (true) {
        if (tok->token == T_LBRACKET) {
            //! Unsure about this for local variables
            //! I don't think i calculated the offset for the size yet

            Scanner_Scan(s, tok);
            if (store == C_LOCAL || store == C_PARAM || store == C_MEMBER) {
                lfatal(s, "UnsupportedError: only globals can be arrays");
            }

            SymTable_AddGlob(st, s->text, pointer_to(type), cType, S_ARRAY,
                             tok->token == T_INTLIT ? tok->intvalue : 1, false);
            Scanner_Scan(s, tok);
            match(s, tok, T_RBRACKET, "]");
        } else if (tok->token == T_ASSIGN) {
            if (store == C_LOCAL || store == C_PARAM || store == C_MEMBER) {
                lfatal(s,
                       "UnsupportedError: only globals can"
                       "be initialised");
            }

            Scanner_Scan(s, tok);

            // Only for scalar types
            SymTableEntry sym =
                SymTable_AddGlob(st, s->text, type, cType, S_VAR, 0, false);

            // For now until lazy evaluation is implemented
            // we stick with singular values

            // initialisation weird with local vars - doesnt work as of yet

            //! implemented initialiation for local vars
            //! but disabled for now just in case

            if (tok->token == T_INTLIT) {
                SymTable_SetValue(st, sym, tok->intvalue);
            } else {
                lfatal(s,
                       "UnsupportedError: only integer literals are supported");
            }
            Scanner_Scan(s, tok);

            // TODO : check if there is wrong syntax?
            // TODO: NVM it does it below
        } else {
            switch (store) {
                case C_STRUCT:
                    fatala("InternalError: C_STRUCT shouldn't be here");
                case C_UNION:
                    fatala("InternalError: C_UNION shouldn't be here");
                case C_NONE:
                    fatala("InternalError: C_NONE shouldn't be here");
                case C_ENUMTYPE:
                    fatala("InternalError: C_ENUMTYPE shouldn't be here");
                case C_TYPEDEF:
                    fatala("InternalError: C_TYPEDEF shouldn't be here");
                case C_ENUMVAL:
                    fatala("InternalError: C_ENUMVAL shouldn't be here");

                
                case C_GLOBAL:
                    debug("Adding global variable %s", s->text);
                    if (SymTable_AddGlob(st, s->text, type, cType, S_VAR, 1, false) ==
                        NULL) {
                        lfatala(s,
                                "DuplicateError: Duplicate global variable %s",
                                s->text);
                    }
                    break;
                case C_PARAM:
                    debug("Adding parameter %s", s->text);
                    if (SymTable_AddParam(st, s->text, type, cType, S_VAR, 1) ==
                        NULL) {
                        lfatala(s, "DuplicateError: Duplicate parameter %s",
                                s->text);
                    }
                    break;
                case C_MEMBER:
                    debug("Adding member %s", s->text);
                    if (SymTable_AddMemb(st, s->text, type, cType, S_VAR, 1) ==
                        NULL) {
                        lfatala(s, "DuplicateError: Duplicate member %s",
                                s->text);
                    }
                    break;
                case C_LOCAL:
                    debug("Adding local variable %s", s->text);
                    if (SymTable_AddLocl(st, s->text, type, cType, S_VAR, 1) ==
                        NULL) {
                        lfatala(s,
                                "DuplicateError: Duplicate local variable %s",
                                s->text);
                    }
                    break;
            }
        }

        if (store == C_PARAM &&
            (tok->token == T_COMMA || tok->token == T_RPAREN)) {
            return;
        }

        if (tok->token == T_SEMI || tok->token == T_EOF) {
            if (!ignoreEnd) Scanner_Scan(s, tok);
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
        // debug("before type check");
        type = parse_type(s, st, tok, &cType);
        // debug("after type check");

        // debug("before ident check");
        ident(s, tok);
        // debug("after ident check");

        if (protoPtr != NULL) {
            debug("1:%d 2:%d", protoPtr->type, type);
            if (type != protoPtr->type) {
                lfatal(s,
                       "InvalidParamsError: parameter type mismatch for proto");
            }
            protoPtr = protoPtr->next;
        }

        var_declare(s, st, tok, type, cType, store, true);
        paramCnt++;

        debug("token %d", tok->token);

        if (tok->token != sep && tok->token != end) {
            lfatala(s,
                    "SyntaxError: expected comma or right parenthesis got %d",
                    tok->token);
        }
        if (tok->token == sep) {
            debug("sep met");
            Scanner_Scan(s, tok);
        }
    }

    debug("end %d", tok->token);

    // Swallow end?
    // Scanner_Scan(s, tok);
    if (funcSym != NULL && paramCnt != funcSym->nElems) {
        debug("param count is %d", paramCnt);
        debug("funcSym is %d", funcSym->nElems);
        lfatal(s, "InvalidParamsError: parameter count mismatch for proto");
    }

    return paramCnt;
}

void global_declare(Compiler c, Scanner s, SymTable st, Token tok,
                    Context ctx) {
    ASTnode tree;
    enum ASTPRIM type;
    SymTableEntry cType;

    while (true) {
        // SymTable_ResetLocls(st);

        debug("token 1 %d", tok->token);

        type = parse_type(s, st, tok, &cType);

        if (type == -1) {
            semi(s, tok);
            continue;
        }

        debug("is ident? %d", tok->token == T_IDENT);
        ident(s, tok);

        debug("token 2 %d", tok->token);

        if (tok->token == T_LPAREN) {
            debug("type is %d", type);
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

            SymTable_FreeLocls(st);
            Context_SetFunctionId(ctx, NULL);

        } else {
            var_declare(s, st, tok, type, cType, C_GLOBAL, false);
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
        newFuncSym = SymTable_AddGlob(st, s->text, type, NULL, S_FUNC, 1, false);
    }

    lparen(s, tok);
    paramCnt =
        var_declare_list(s, st, tok, oldFuncSym, C_PARAM, T_COMMA, T_RPAREN);
    rparen(s, tok);

    if (newFuncSym) {
        newFuncSym->nElems = paramCnt;
        newFuncSym->member = st->paramHead;
        oldFuncSym = newFuncSym;
    } else {
        SymTable_FreeParams(st);
    }

    st->paramHead = st->paramTail = NULL;

    // This is only a proto
    if (tok->token == T_SEMI) {
        debug("proto");
        Scanner_Scan(s, tok);
        return NULL;
    }

    ctx->functionId = oldFuncSym;

    tree = Compound_Statement(c, s, st, tok, ctx);
    if (type != P_VOID) {
        finalstmt = tree->op == A_GLUE ? tree->right : tree;
        if (finalstmt == NULL || finalstmt->op != A_RETURN) {
            fatal("NoReturnError: non-void function must return a value\n");
        }
    }

    return ASTnode_NewUnary(A_FUNCTION, P_VOID, tree, NULL, 0);
}

enum ASTPRIM parse_type(Scanner s, SymTable st, Token tok,
                        SymTableEntry *ctype) {
    debug("parse_type");
    enum ASTPRIM type;
    switch (tok->token) {
        case T_INT:
            debug("type is int");
            type = P_INT;
            debug("before scan");
            Scanner_Scan(s, tok);
            debug("after scan");
            break;
        case T_CHAR:
            debug("type is char");
            type = P_CHAR;
            Scanner_Scan(s, tok);
            break;
        case T_VOID:
            debug("type is void");
            type = P_VOID;
            Scanner_Scan(s, tok);
            break;
        case T_STRUCT:
            debug("type is struct");
            type = P_STRUCT;
            *ctype = composite_declare(s, st, tok, P_STRUCT);
            break;
        case T_UNION:
            debug("type is union");
            type = P_UNION;
            *ctype = composite_declare(s, st, tok, P_UNION);
            break;
        case T_ENUM:
            type = P_INT;
            enum_declare(s, st, tok);
            // if after ;, theres no type
            if (tok->token == T_SEMI) type = -1;
            break;
        case T_TYPEDEF:
            debug("type is typedef");
            type = typedef_declare(s, st, tok, ctype);
            if (tok->token == T_SEMI) type = -1;
            break;
        case T_IDENT: // typedef
            debug("type is ident (maybe a typedef)");
            type = typedef_type(s, st, tok, ctype);
            break;  
        default:
            fatala("InternalError: unknown type %d", tok->token);
    }

    // allows the user to do int ******a
    while (true) {
        if (tok->token != T_STAR) break;
        type = pointer_to(type);
        Scanner_Scan(s, tok);
    }

    return type;
}

static SymTableEntry composite_declare(Scanner s, SymTable st, Token tok,
                                       enum ASTPRIM type) {
    SymTableEntry cType = NULL;
    SymTableEntry memb;

    int offset;
    Scanner_Scan(s, tok);

    if (tok->token == T_IDENT) {
        if (type == P_STRUCT) {
            cType = SymTable_FindStruct(st, s);
        } else if (type == P_UNION) {
            cType = SymTable_FindUnion(st, s);
        } else {
            fatal("InternalError: Unknown composite type");
        }
        Scanner_Scan(s, tok);
    }

    if (tok->token != T_LBRACE) {
        if (cType == NULL) {
            lfatala(s, "UndefinedError: struct/union %s is not defined",
                    s->text);
        }
        return cType;
    }

    if (cType) {
        lfatal(s, "DuplicateError: struct/union already defined");
    }

    if (type == P_STRUCT) {
        cType = SymTable_AddStruct(st, s->text, P_STRUCT, NULL, S_VAR, 0);
    } else if (type == P_UNION) {
        cType = SymTable_AddUnion(st, s->text, P_UNION, NULL, S_VAR, 0);
    } else {
        fatal("InternalError: Unknown composite type");
    }

    debug("Declaring a struct: %s", s->text);

    Scanner_Scan(s, tok);

    var_declare_list(s, st, tok, NULL, C_MEMBER, T_SEMI, T_RBRACE);
    rbrace(s, tok);
    // semi(s, tok);

    cType->member = st->membHead;
    st->membHead = st->membTail = NULL;

    memb = cType->member;
    memb->offset = 0;
    offset = type_size(memb->type, memb->ctype);

    // for union
    int maxSize = 0;

    for (memb = memb->next; memb != NULL; memb = memb->next) {
        if (type == P_STRUCT) {
            memb->offset = MIPS_Align(memb->type, offset, 1);
            // Calculates offset of next free byte
            offset += type_size(memb->type, memb->ctype);
        } else {
            memb->offset = 0;
            int size = type_size(memb->type, memb->ctype);
            debug("Size of %s: %d", memb->name, size);
            // We only store the largest byte size
            if (size > maxSize) {
                maxSize = size;
            }
        }
    }

    debug("Size of struct %d", offset);

    cType->size = type == P_STRUCT ? offset : maxSize;

    return cType;
}

static void enum_declare(Scanner s, SymTable st, Token tok) {
    SymTableEntry eType = NULL;
    char *name = NULL;
    int intVal = 0;

    Scanner_Scan(s, tok);

    if (tok->token == T_IDENT) {
        eType = SymTable_FindEnumType(st, s);
        name = strdup(s->text);
        Scanner_Scan(s, tok);
    }

    if (tok->token != T_LBRACE) {
        if (eType == NULL) {
            lfatala(s, "UndefinedError: enum %s is not defined", name);
        }
        if (name) free(name);
        return;
    }

    Scanner_Scan(s, tok);

    if (eType != NULL) {
        lfatala(s, "DuplicateError: enum %s already defined", eType->name);
    }

    eType = SymTable_AddEnum(st, name, C_ENUMTYPE, 0);
    
    if (name) free(name);

    while (true) {
        ident(s, tok);
        name = strdup(s->text);

        eType = SymTable_FindEnumVal(st, s);
        if (eType != NULL) {
            lfatala(s, "DuplicateError: enum value %s already defined", name);
        }

        if (tok->token == T_ASSIGN) {
            Scanner_Scan(s, tok);
            if (tok->token != T_INTLIT) {
                lfatal(s, "SyntaxError: expected integer literal");
            }

            intVal = tok->intvalue;
            Scanner_Scan(s, tok);
        }

        eType = SymTable_AddEnum(st, name, C_ENUMVAL, intVal++);
        free(name);

        if (tok->token == T_RBRACE) break;

        comma(s, tok);
    }

    // Consume the }
    Scanner_Scan(s, tok);
}

static enum ASTPRIM typedef_declare(Scanner s, SymTable st, Token tok,
                             SymTableEntry *cType) {
    enum ASTPRIM type;
    
    // typedef consumed
    Scanner_Scan(s, tok);
    // a ident should be here

    type = parse_type(s, st, tok, cType);

    if (SymTable_FindTypeDef(st, s) != NULL) {
        lfatala(s, "DuplicateError: typedef %s already defined", s->text);
    }

    SymTable_AddTypeDef(st, s->text, type, *cType, S_VAR, 0);
    Scanner_Scan(s, tok);

    return type;
}

static enum ASTPRIM typedef_type(Scanner s, SymTable st, Token tok, SymTableEntry *cType) {
    SymTableEntry type;

    type = SymTable_FindTypeDef(st, s);
    if (type == NULL) {
        lfatala(s, "UndefinedError: typedef %s is not defined", s->text);
    }

    Scanner_Scan(s, tok);
    *cType = type->ctype;
    return type->type;
}