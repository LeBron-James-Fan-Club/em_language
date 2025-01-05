#include "expr.h"

#include "ast.h"
#include "decl.h"
#include "defs.h"
#include "misc.h"
#include "sym.h"
#include "types.h"

static enum ASTOP arithOp(Scanner s, enum OPCODES tok);
static int precedence(Scanner s, enum OPCODES op);
static bool rightAssoc(enum OPCODES op);
static ASTnode primary(Compiler c, Scanner s, SymTable st, Token t,
                       Context ctx);
static ASTnode paren_expression(Compiler c, Scanner s, SymTable st, Token t,
                                Context ctx);

static ASTnode ASTnode_FuncCall(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx);
static ASTnode ASTnode_ArrayRef(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx, ASTnode left);

static ASTnode ASTnode_Prefix(Compiler c, Scanner s, SymTable st, Token tok,
                              Context ctx);
static ASTnode ASTnode_Postfix(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx);

static ASTnode peek_operator(Compiler c, Scanner s, SymTable st, Token tok,
                             Context ctx);
static ASTnode sizeof_operator(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx);

static ASTnode ASTnode_MemberAccess(Scanner s, SymTable st, Token tok,
                                    Context ctx, ASTnode left, bool isPtr);

// * 1:1 baby
static enum ASTOP arithOp(Scanner s, enum OPCODES tok) {
    if (tok > T_EOF && tok < T_INTLIT) return (enum ASTOP)tok;
    lfatala(s, "SyntaxError: %d", tok);
}

static int precedence(Scanner s, enum OPCODES op) {
    switch (op) {
        case T_LT:
        case T_GT:
        case T_LE:
        case T_GE:
            return 12;
        case T_EQ:
        case T_NE:
            return 11;

        case T_MODULO:
        case T_SLASH:
        case T_STAR:
            return 10;

        case T_PLUS:
        case T_MINUS:
            return 9;

        case T_LSHIFT:
        case T_RSHIFT:
            return 8;
        // BIT AND
        case T_AMPER:
            return 7;
        case T_XOR:
            return 6;
        case T_OR:
            return 5;
        case T_LOGAND:
            return 4;
        case T_LOGOR:
            return 3;
        case T_QUESTION:
            return 2;
        case T_ASSIGN:
        case T_ASPLUS:
        case T_ASMINUS:
        case T_ASSTAR:
        case T_ASSLASH:
        case T_ASMOD:
            return 1;
        default:
            lfatala(s, "InternalError: No proper precedence for operator %d",
                    op);
    }
}

static bool rightAssoc(enum OPCODES op) {
    return op >= T_ASSIGN && op <= T_ASMOD;
}

static ASTnode primary(Compiler c, Scanner s, SymTable st, Token t,
                       Context ctx) {
    ASTnode n;
    SymTableEntry var;
    SymTableEntry enumPtr;

    switch (t->token) {
        case T_STRLIT:
            // Compiler being schizo about name for some reason
            // cause name is declared as soon after case
            {
                char *name = SymTableEntry_MakeAnon(st, NULL);
                bool customName = false;

                char *text = strdup(s->text);
                bool madeNew = false;

                while (true) {
                    Scanner_Scan(s, t);
                    if (t->token != T_STRLIT) break;
                    Scanner_RejectToken(s, t);

                    char *otherText = s->text;
                    char *new = malloc(strlen(text) + strlen(otherText) + 1);
                    strcpy(new, text);
                    strcat(new, otherText);

                    free(text);
                    free(otherText);

                    text = new;
                    madeNew = true;
                    Scanner_Scan(s, t);
                }

                if (t->token == T_LBRACKET) {
                    Scanner_Scan(s, t);
                    customName = true;
                    if (t->token != T_IDENT)
                        lfatal(
                            s,
                            "SyntaxError: Expected identifier in custom label");
                    free(name);
                    name = s->text;
                    Scanner_Scan(s, t);
                    rbracket(s, t);
                }

                var = SymTable_AddGlob(st, name, pointer_to(P_CHAR), NULL,
                                       S_VAR, C_GLOBAL, NULL, 1, 0);

                if (!customName) free(name);
                SymTable_SetText(st, text, var);
                free(text);

                if (madeNew) free(text);
                Scanner_RejectToken(s, t);
                debug("finished making annoymous string");
                n = ASTnode_NewLeaf(A_STRLIT, pointer_to(P_CHAR), NULL, var, 0);
            }
            break;
        case T_INTLIT:
            if (t->intvalue >= 0 && t->intvalue < 256) {
                n = ASTnode_NewLeaf(A_INTLIT, P_CHAR, NULL, NULL, t->intvalue);
            } else {
                n = ASTnode_NewLeaf(A_INTLIT, P_INT, NULL, NULL, t->intvalue);
            }
            break;
        case T_PEEK:
            // printf("PEEK\n");
            n = peek_operator(c, s, st, t, ctx);
            break;
        case T_SIZEOF:
            n = sizeof_operator(c, s, st, t, ctx);
            break;
        case T_IDENT:
            if ((enumPtr = SymTable_FindEnumVal(st, s)) != NULL) {
                return ASTnode_NewLeaf(A_INTLIT, P_INT, NULL, NULL,
                                       enumPtr->posn);
            }
            if ((var = SymTable_FindSymbol(st, s, ctx)) == NULL) {
                lfatala(s, "UndefinedError: Undefined variable %s", s->text);
            }

            switch (var->stype) {
                case S_VAR:
                    n = ASTnode_NewLeaf(A_IDENT, var->type, var->ctype, var, 0);
                    break;
                case S_ARRAY:
                    n = ASTnode_NewLeaf(A_ADDR, var->type, var->ctype, var, 0);
                    n->rvalue = 1;
                    break;
                case S_FUNC:
                    Scanner_Scan(s, t);
                    if (t->token == T_LPAREN) {
                        n = ASTnode_FuncCall(c, s, st, t, ctx);
                    } else {
                        lfatala(s,
                                "SyntaxError: Expected '(' after function %s",
                                s->text);
                    }
                    return n;
                default:
                    lfatala(s,
                            "SyntaxError: identifier not a scalar or array, %s",
                            s->text);
            }
            break;
        case T_LPAREN:
            return paren_expression(c, s, st, t, ctx);
        default:
            lfatala(s, "SyntaxError: Expecting primary expression got %s",
                    t->tokstr);
    }

    Scanner_Scan(s, t);

    return n;
}

static ASTnode paren_expression(Compiler c, Scanner s, SymTable st, Token t,
                                Context ctx) {
    ASTnode n;
    enum ASTPRIM type = P_NONE;
    SymTableEntry cType = NULL;

    Scanner_Scan(s, t);

    switch (t->token) {
        case T_IDENT:
            if (SymTable_FindTypeDef(st, s) == NULL) {
                n = ASTnode_Order(c, s, st, t, ctx, 0);
                break;
            }
        case T_VOID:
        case T_CHAR:
        case T_INT:
        case T_STRUCT:
        case T_UNION:
        case T_ENUM:
            debug("fell here");
            type = parse_cast(c, s, st, t, ctx, &cType);
            rparen(s, t);
        default:
            // * rest of the expression (int)b <- this
            // * Please work with the shunting algorithm
            // * I beg you
            debug("falling INTO THE FUCKING VOID");
            n = ASTnode_Order(c, s, st, t, ctx, 0);
            debug("token now %s", t->tokstr);
    }

    // Its a left parenthesis eat it
    if (type == P_NONE) {
        rparen(s, t);
    } else {
        n = ASTnode_NewUnary(A_CAST, type, n, cType, NULL, 0);
    }

    return n;
}

ASTnode ASTnode_Order(Compiler c, Scanner s, SymTable st, Token t, Context ctx,
                      int prePreced) {
    ASTnode left, right;
    ASTnode lTemp, rTemp;
    enum OPCODES tokenType;
    enum ASTOP op;

    left = ASTnode_Prefix(c, s, st, t, ctx);
    tokenType = t->token;
    debug("tokenType %s", t->tokstr);
    if (tokenType == T_SEMI || tokenType == T_RPAREN ||
        tokenType == T_RBRACKET || tokenType == T_COMMA ||
        tokenType == T_COLON || tokenType == T_RBRACE) {
        left->rvalue = 1;
        return left;
    }

    while (precedence(s, tokenType) > prePreced ||
           (rightAssoc(tokenType) && precedence(s, tokenType) == prePreced)) {
        Scanner_Scan(s, t);

        right = ASTnode_Order(c, s, st, t, ctx, precedence(s, tokenType));

        op = arithOp(s, tokenType);

        if (op == A_TERNARY) {
            debug("Ternary operator");
            match(s, t, T_COLON, ":");
            lTemp = ASTnode_Order(c, s, st, t, ctx, 0);
            return ASTnode_New(A_TERNARY, right->type, left, right, lTemp,
                               right->ctype, NULL, 0);

        } else if (op == A_ASSIGN) {
            right->rvalue = 1;
            debug("The assign called it");
            right = modify_type(right, left->type, left->ctype, A_NONE);
            if (right == NULL) {
                lfatal(s, "SyntaxError: incompatible types in assignment");
            }
            lTemp = left;
            left = right;
            right = lTemp;
        } else {
            debug("The assign did not call it");
            debug("op %d", op);
            right->rvalue = 1;
            left->rvalue = 1;
            lTemp = modify_type(left, right->type, right->ctype, op);
            rTemp = modify_type(right, left->type, left->ctype, op);

            if (lTemp == NULL && rTemp == NULL) {
                lfatal(s, "SyntaxError: Incompatible types in expression");
            }

            if (lTemp != NULL) left = lTemp;
            if (rTemp != NULL) right = rTemp;
        }

        left = ASTnode_New(op, left->type, left, NULL, right, left->ctype, NULL,
                           0);

        switch (op) {
            case A_LOGOR:
            case A_LOGAND:
            case A_EQ:
            case A_NE:
            case A_LT:
            case A_GT:
            case A_LE:
            case A_GE:
                left->type = P_INT;
        }

        tokenType = t->token;
        if (tokenType == T_SEMI || tokenType == T_RPAREN ||
            tokenType == T_RBRACKET || tokenType == T_COMMA ||
            tokenType == T_COLON || tokenType == T_RBRACE) {
            left->rvalue = 1;
            return left;
        }
    }

    left->rvalue = 1;
    return left;
}

static ASTnode ASTnode_FuncCall(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx) {
    ASTnode t;
    SymTableEntry var;

    if ((var = SymTable_FindSymbol(st, s, ctx)) == NULL) {
        lfatala(s, "UndefinedError: Undefined function %s", s->text);
    }

    lparen(s, tok);

    t = expression_list(c, s, st, tok, ctx, T_RPAREN);

    t = ASTnode_NewUnary(A_FUNCCALL, var->type, t, var->ctype, var, 0);

    rparen(s, tok);

    return t;
}

ASTnode expression_list(Compiler c, Scanner s, SymTable st, Token tok,
                        Context ctx, enum OPCODES endToken) {
    ASTnode tree = NULL, child = NULL;
    int exprCount = 0;
    while (tok->token != endToken) {
        child = ASTnode_Order(c, s, st, tok, ctx, 0);
        exprCount++;

        debug("expression generation %d", exprCount);
        debug("child %p", child);

        tree = ASTnode_New(A_GLUE, P_NONE, tree, NULL, child, NULL, NULL,
                           exprCount);

        if (tok->token == endToken) break;

        match(s, tok, T_COMMA, ",");
    }
    return tree;
}

static ASTnode ASTnode_ArrayRef(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx, ASTnode left) {
    ASTnode right = NULL;

    if (!ptrtype(left->type)) {
        lfatala(s, "TypeError: Array index must be a pointer or array, %s",
                s->text);
    }

    ArrayDim dims = left->sym->dims;
    do {
        // Possible problem? - no I dont think so
        if (right && dims == NULL)
            lfatal(s, "TypeError: Dimension mismatch for variable.");
        // eat [ - Yummy
        Scanner_Scan(s, tok);

        right = ASTnode_Order(c, s, st, tok, ctx, 0);

        match(s, tok, T_RBRACKET, "]");

        left->rvalue = 1;

        // Scales by sizeof(type) - this can be factorised out
        // e.g. (i * size y * sizeof(int)) + ( y * sizeof(int))
        // -> ((i * size y) + y) * sizeof(int)
        // TODO: optimise this later
        right = modify_type(right, left->type, left->ctype, A_ADD);

        // Multiplcation of dimensions needs to happen here I think

        if (dims) {
            // We skip the first size cause only other
            // other sizes are multiplied
            int scale = 1;
            for (ArrayDim currentDim = dims->next; currentDim != NULL;
                 currentDim = currentDim->next) {
                scale *= dims->nElems;
            }
            left = ASTnode_NewUnary(A_SCALE, left->type, left, left->ctype,
                                    NULL, scale);
            dims = dims->next;
        }

        left = ASTnode_New(A_ADD, left->type, left, NULL, right, left->ctype,
                           NULL, 0);

        // Peek to see if we can continue
        debug("token arrayref: %s", tok->tokstr);
    } while (tok->token == T_LBRACKET);
    //Scanner_RejectToken(s, tok);
    debug("WE OUT OF DA ARRAY REF >:)");

    /*if (!inttype(right->type)) {
        fatal("TypeError: Array index must be an integer");
    }*/

    return ASTnode_NewUnary(A_DEREF, value_at(left->type), left, left->ctype,
                            NULL, 0);
}

static ASTnode ASTnode_Prefix(Compiler c, Scanner s, SymTable st, Token tok,
                              Context ctx) {
    ASTnode t;
    switch (tok->token) {
        case T_AMPER:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            if (t->op != A_IDENT) {
                lfatal(s, "SyntaxError: Expected identifier");
            }
            t->op = A_ADDR;
            t->type = pointer_to(t->type);
            break;
        case T_STAR:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            if (t->op != A_IDENT && t->op != A_DEREF) {
                lfatal(
                    s,
                    "SyntaxError: * Operator must be followed by an identifier "
                    "or *");
            }
            t = ASTnode_NewUnary(A_DEREF, value_at(t->type), t, t->ctype, NULL,
                                 0);
            break;
        case T_INC:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(c, s, st, tok, ctx);

            // * temp check - cause inc also sets the rvalue
            if (t->op != A_IDENT) {
                lfatal(s, "SyntaxError: ++ must be followed by an identifier");
            }
            debug("pre inc");
            t = ASTnode_NewUnary(A_PREINC, t->type, t, t->ctype, NULL, 0);
            break;
        case T_MINUS:
            Scanner_Scan(s, tok);
            // for ---a
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_NEGATE, t->type, t, t->ctype, NULL, 0);
            break;
        case T_INVERT:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_INVERT, t->type, t, t->ctype, NULL, 0);
            break;
        case T_LOGNOT:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_LOGNOT, t->type, t, t->ctype, NULL, 0);
            break;
        case T_DEC:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            if (t->op != A_IDENT) {
                lfatal(s, "SyntaxError: -- must be followed by an identifier");
            }
            t = ASTnode_NewUnary(A_PREDEC, t->type, t, t->ctype, NULL, 0);
            break;
        default:
            t = ASTnode_Postfix(c, s, st, tok, ctx);
    }
    return t;
}

static ASTnode ASTnode_Postfix(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {
    ASTnode n = primary(c, s, st, tok, ctx);

    debug("out of primary");

    // Converts enum to a specific int

    while (1) {
        switch (tok->token) {
            case T_LBRACKET:
                n = ASTnode_ArrayRef(c, s, st, tok, ctx, n);
                break;
            case T_DOT:
                n = ASTnode_MemberAccess(s, st, tok, ctx, n, false);
                break;
            case T_ARROW:
                n = ASTnode_MemberAccess(s, st, tok, ctx, n, true);
                break;
            case T_INC:
                if (n->rvalue == 1) {
                    lfatala(s, "SyntaxError: Cannot ++ on rvalue, %s", s->text);
                }
                Scanner_Scan(s, tok);
                if (n->op == A_POSTINC || n->op == A_POSTDEC) {
                    lfatala(
                        s,
                        "SyntaxError: Cannot ++ and/or -- more than once, %s",
                        s->text);
                }
                n->op = A_POSTINC;
                break;
            case T_DEC:
                if (n->rvalue == 1) {
                    lfatala(s, "SyntaxError: Cannot -- on rvalue, %s", s->text);
                }
                Scanner_Scan(s, tok);
                if (n->op == A_POSTINC || n->op == A_POSTDEC) {
                    lfatala(
                        s,
                        "SyntaxError: Cannot ++ and/or -- more than once, %s",
                        s->text);
                }
                n->op = A_POSTDEC;
                break;
            default:
                return n;
        }
    }
}

static ASTnode ASTnode_MemberAccess(Scanner s, SymTable st, Token tok,
                                    Context ctx, ASTnode left, bool isPtr) {
    ASTnode right;
    // the struct
    SymTableEntry typePtr;

    // the member
    SymTableEntry m = NULL;

    if (isPtr && left->type != pointer_to(P_STRUCT) &&
        left->type != pointer_to(P_UNION)) {
        fatala("TypeError: %s is not a pointer to a struct/union", s->text);
    }
    if (!isPtr && left->type != P_STRUCT && left->type != P_UNION) {
        fatala("TypeError: %s is not a struct/union", s->text);
    }

    if (!isPtr) {
        if (left->type == P_STRUCT || left->type == P_UNION) {
            left->op = A_ADDR;
        } else {
            lfatala(s, "TypeError: %s is not a struct/union", s->text);
        }
    }

    typePtr = left->ctype;

    // Consume . or ->
    Scanner_Scan(s, tok);

    ident(s, tok);

    for (m = typePtr->member; m != NULL; m = m->next) {
        if (!strcmp(m->name, s->text)) break;
    }

    if (m == NULL) fatala("UndefinedError: Undefined member %s", s->text);

    left->rvalue = 1;

    right = ASTnode_NewLeaf(A_INTLIT, P_INT, NULL, NULL, m->posn);

    left = ASTnode_New(A_ADD, pointer_to(m->type), left, NULL, right, m->ctype,
                       NULL, 0);
    left = ASTnode_NewUnary(A_DEREF, m->type, left, m->ctype, NULL, 0);

    return left;
}

static ASTnode peek_operator(Compiler c, Scanner s, SymTable st, Token tok,
                             Context ctx) {
    ASTnode n;
    match(s, tok, T_PEEK, "peek");
    lparen(s, tok);
    n = ASTnode_Order(c, s, st, tok, ctx, 0);
    n->rvalue = true;
    rparen(s, tok);
    return ASTnode_NewUnary(A_PEEK, P_INT, n, NULL, NULL, 0);
}

// sizeof on variables dont work yet
static ASTnode sizeof_operator(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {
    enum ASTPRIM type;
    enum STORECLASS class;
    int size;
    SymTableEntry cType;

    debug("sizeof");
    match(s, tok, T_SIZEOF, "sizeof");
    lparen(s, tok);
    type = parse_stars(s, tok, parse_type(c, s, st, tok, ctx, &cType, &class));
    debug("cType %p", cType);
    size = type_size(type, cType);
    rparen(s, tok);

    return ASTnode_NewLeaf(A_INTLIT, P_INT, NULL, NULL, size);
}