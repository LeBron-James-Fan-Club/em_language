#include "expr.h"

#include "ast.h"
#include "decl.h"
#include "defs.h"
#include "misc.h"
#include "sym.h"
#include "types.h"

static enum ASTOP arithOp(Scanner s, enum OPCODES tok);
static int precedence(enum ASTOP op);
static ASTnode primary(Compiler c, Scanner s, SymTable st, Token t,
                       Context ctx);
static void orderOp(Scanner s, ASTnode *stack, enum ASTOP *opStack, int *top,
                    int *opTop);

static ASTnode ASTnode_FuncCall(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx);
static ASTnode ASTnode_ArrayRef(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx);

static ASTnode ASTnode_Prefix(Compiler c, Scanner s, SymTable st, Token tok,
                              Context ctx);
static ASTnode ASTnode_Postfix(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx);

static ASTnode peek_statement(Compiler c, Scanner s, SymTable st, Token tok,
                              Context ctx);

static ASTnode ASTnode_MemberAccess(Scanner s, SymTable st, Token tok,
                                    Context ctx, bool isPtr);

// * 1:1 baby
static enum ASTOP arithOp(Scanner s, enum OPCODES tok) {
    if ((tok > T_EOF && tok < T_INTLIT) ||
        (tok >= T_ASSIGNADD && tok <= T_ASSIGNMOD))
        return (enum ASTOP)tok;
    lfatala(s, "SyntaxError: %d", tok);
}

static int precedence(enum ASTOP op) {
    switch (op) {
        case T_LT:
        case T_GT:
        case T_LE:
        case T_GE:
            return 9;
        case T_EQ:
        case T_NE:
            return 8;

        case T_MODULO:
        case T_SLASH:
        case T_STAR:
            return 7;

        case T_PLUS:
        case T_MINUS:
            return 6;

        case T_LSHIFT:
        case T_RSHIFT:
            return 5;
        // BIT AND
        case T_AMPER:
            return 4;
        case T_XOR:
            return 3;
        case T_OR:
            return 2;
        case T_ASSIGN:
        case T_ASSIGNADD:
        case T_ASSIGNSUB:
        case T_ASSIGNMUL:
        case T_ASSIGNDIV:
        case T_ASSIGNMOD:
            return 1;
        default:
            fatala("InternalError: No proper precedence for operator %d", op);
    }
}

static bool rightAssoc(enum ASTOP op) {
    return op == T_ASSIGN || op == T_ASSIGNADD || op == T_ASSIGNSUB ||
           op == T_ASSIGNMOD || op == T_ASSIGNDIV || op == T_ASSIGNMUL;
}

static ASTnode primary(Compiler c, Scanner s, SymTable st, Token t,
                       Context ctx) {
    SymTableEntry var;
    ASTnode n;
    enum ASTPRIM type = P_NONE;

    switch (t->token) {
        case T_STRLIT:
            // Compiler being schizo about name for some reason
            // cause name is declared as soon after case
            {
                char *name = SymTableEntry_MakeAnon(st, NULL);
                var = SymTable_AddGlob(st, name, pointer_to(P_CHAR), NULL,
                                       S_VAR, C_GLOBAL, 1, 0);
                free(name);
                SymTable_SetText(st, s, var);
                n = ASTnode_NewLeaf(A_STRLIT, pointer_to(P_CHAR), var, 0);
            }
            break;
        case T_INTLIT:
            if (t->intvalue >= 0 && t->intvalue < 256) {
                n = ASTnode_NewLeaf(A_INTLIT, P_CHAR, NULL, t->intvalue);
            } else {
                n = ASTnode_NewLeaf(A_INTLIT, P_INT, NULL, t->intvalue);
            }
            break;
        case T_PEEK:
            // printf("PEEK\n");
            n = peek_statement(c, s, st, t, ctx);
            break;
        case T_IDENT:
            return ASTnode_Postfix(c, s, st, t, ctx);
        case T_LPAREN:
            // eat (
            Scanner_Scan(s, t);

            switch (t->token) {
                case T_IDENT:
                    if (SymTable_FindTypeDef(st, s) == NULL) {
                        n = ASTnode_Order(c, s, st, t, ctx);
                        break;
                    }
                case T_VOID:
                case T_CHAR:
                case T_INT:
                case T_STRUCT:
                case T_UNION:
                case T_ENUM:
                    debug("fell here");
                    type = parse_cast(c, s, st, t, ctx);
                    rparen(s, t);
                default:
                    // * rest of the expression (int)b <- this
                    // * Please work with the shunting algorithm
                    // * I beg you
                    debug("falling INTO THE FUCKING VOID");
                    n = ASTnode_Order(c, s, st, t, ctx);
                    debug("token now %s", t->tokstr);
            }

            // Its a left parenthesis eat it
            if (type == P_NONE) {
                rparen(s, t);
            } else {
                debug("type %d", type);
                debug("tree %p", n);
                ASTnode_Dump(n, st, 0, 0);
                n = ASTnode_NewUnary(A_CAST, type, n, NULL, 0);
            }

            // If shit was scanned in before
            switch (t->token) {
                case T_SEMI:
                case T_EOF:
                case T_COMMA:
                case T_COLON:
                case T_RPAREN:
                case T_RBRACKET:
                    Scanner_RejectToken(s, t);
            }

            debug("leaving primary now");
            return n;

        default:
            lfatala(s, "SyntaxError: Expected primary expression got %s",
                    t->tokstr);
    }

    return n;
}

static void orderOp(Scanner s, ASTnode *stack, enum ASTOP *opStack, int *top,
                    int *opTop) {
    ASTnode ltemp, rtemp;
    if (*opTop < 0) {
        lfatal(s, "SyntaxError: (opTop < 0)");
    }

    enum ASTOP op = opStack[(*opTop)--];

    if (*top < 1) {
        lfatal(s, "SyntaxError: (top < 1)");
    }

    ASTnode right = stack[(*top)--];
    ASTnode left = stack[(*top)--];

    if (op == A_ASSIGN) {
        right->rvalue = 1;
        right = modify_type(right, left->type, 0);
        if (right == NULL) {
            lfatal(s, "SyntaxError: incompatible types in assignment");
        }
        ltemp = left;
        left = right;
        right = ltemp;
    } else {
        right->rvalue = 1;
        left->rvalue = 1;
        ltemp = modify_type(left, right->type, op);
        rtemp = modify_type(right, left->type, op);

        if (ltemp == NULL && rtemp == NULL) {
            lfatal(s, "SyntaxError: Incompatible types in expression");
        }

        if (ltemp != NULL) left = ltemp;
        if (rtemp != NULL) right = rtemp;
    }
    stack[++(*top)] = ASTnode_New(op, left->type, left, NULL, right, NULL, 0);
}

ASTnode ASTnode_Order(Compiler c, Scanner s, SymTable st, Token t,
                      Context ctx) {
    ASTnode *stack = calloc(MAX_STACK, sizeof(ASTnode));
    enum ASTOP *opStack = calloc(MAX_STACK, sizeof(enum ASTOP));
    enum ASTOP curOp;

    bool expectPreOp = true;

    int top = -1, opTop = -1;

    debug("GET IN ORDER");

    do {
        switch (t->token) {
            case T_SEMI:
            case T_EOF:
            case T_COMMA:
            case T_COLON:
            case T_RPAREN:
            case T_RBRACKET:
                // Breaks out of loop if parenthesis are not balanced
                debug("(breaking out) top %d token %s", top, t->tokstr);
                debug("GTFO OUT OF ORDER NOW");
                goto out;
            case T_LPAREN:
            case T_INTLIT:
            case T_IDENT:
            case T_STRLIT:
            case T_PEEK:
                stack[++top] = primary(c, s, st, t, ctx);
                expectPreOp = false;
                break;
            default:
                if ((t->token == T_STAR || t->token == T_AMPER ||
                     t->token == T_INC || t->token == T_DEC) &&
                    expectPreOp) {
                    // need to somehow add rvalue here
                    stack[++top] = ASTnode_Prefix(c, s, st, t, ctx);
                    break;
                }
                curOp = arithOp(s, t->token);
                while (opTop != -1 &&
                       (precedence(opStack[opTop]) >= precedence(curOp) ||
                        (rightAssoc(opStack[opTop]) &&
                         precedence(opStack[opTop]) == precedence(curOp)))) {
                    orderOp(s, stack, opStack, &top, &opTop);
                }

                //! THE 1:1 THING HAPPENS HERE

                // TODO: Handle the other assignment ops

                // The last node added
                // Should be a identifier
                // If not, then it's a syntax error
                if (curOp >= T_ASSIGNADD && curOp <= T_ASSIGNMOD) {
                    if (stack[top] == NULL) {
                        fatal("InternalError: stack[top] is NULL");
                    }

                    if (stack[top]->op != A_IDENT) {
                        lfatal(s, "SyntaxError: Expected identifier");
                    }

                    opStack[++opTop] = A_ASSIGN;
                    SymTableEntry sym = stack[top]->sym;
                    enum ASTPRIM type = stack[top]->type;
                    stack[++top] = ASTnode_NewLeaf(A_IDENT, type, sym, 0);
                }

                switch (curOp) {
                    case T_ASSIGNADD:
                        opStack[++opTop] = A_ADD;
                        break;
                    case T_ASSIGNSUB:
                        opStack[++opTop] = A_SUBTRACT;
                        break;
                    case T_ASSIGNMUL:
                        opStack[++opTop] = A_MULTIPLY;
                        break;
                    case T_ASSIGNDIV:
                        opStack[++opTop] = A_DIVIDE;
                        break;
                    case T_ASSIGNMOD:
                        opStack[++opTop] = A_MODULO;
                        break;
                    default:
                        opStack[++opTop] = curOp;
                }
                expectPreOp = true;
        }

        Scanner_Scan(s, t);
    } while (true);
out:

    while (opTop != -1) {
        orderOp(s, stack, opStack, &top, &opTop);
    }

    debug("Top: %d line %d", top, s->line);

    if (top != 0) {
        lfatal(s, "SyntaxError: Invalid expression");
    }

    ASTnode n = stack[top];

    free(stack);
    free(opStack);

    // Might break it?
    n->rvalue = 1;
    return n;
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

    t = ASTnode_NewUnary(A_FUNCCALL, var->type, t, var, 0);

    rparen(s, tok);

    Scanner_RejectToken(s, tok);
    return t;
}

ASTnode expression_list(Compiler c, Scanner s, SymTable st, Token tok,
                        Context ctx, enum OPCODES endToken) {
    ASTnode tree = NULL, child = NULL;
    int exprCount = 0;
    while (tok->token != endToken) {
        child = ASTnode_Order(c, s, st, tok, ctx);
        exprCount++;

        debug("expression generation %d", exprCount);
        debug("child %p", child);

        tree = ASTnode_New(A_GLUE, P_NONE, tree, NULL, child, NULL, exprCount);

        if (tok->token == endToken) break;

        match(s, tok, T_COMMA, ",");
    }
    return tree;
}

static ASTnode ASTnode_ArrayRef(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx) {
    //! BUG: For some reason intlit becomes top of stack
    //! only copying the right value
    ASTnode left, right;
    SymTableEntry var;
    if ((var = SymTable_FindSymbol(st, s, ctx)) == NULL) {
        fatala("UndefinedError: Undefined array %s", s->text);
    }
    left = ASTnode_NewLeaf(A_ADDR, var->type, var, 0);
    Scanner_Scan(s, tok);
    right = ASTnode_Order(c, s, st, tok, ctx);
    right->rvalue = 1;

    match(s, tok, T_RBRACKET, "]");

    debug("outta array");

    if (!inttype(right->type)) {
        fatal("TypeError: Array index must be an integer");
    }
    right = modify_type(right, left->type, A_ADD);

    left = ASTnode_New(A_ADD, var->type, left, NULL, right, NULL, 0);

    Scanner_RejectToken(s, tok);
    return ASTnode_NewUnary(A_DEREF, value_at(left->type), left, NULL, 0);
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
            t = ASTnode_NewUnary(A_DEREF, value_at(t->type), t, NULL, 0);
            break;
        case T_INC:
            Scanner_Scan(s, tok);
            t = ASTnode_Postfix(c, s, st, tok, ctx);

            // * temp check - cause inc also sets the rvalue
            if (t->op != A_IDENT) {
                lfatal(s, "SyntaxError: ++ must be followed by an identifier");
            }
            debug("pre inc");
            t = ASTnode_NewUnary(A_PREINC, t->type, t, NULL, 0);
            break;
        case T_MINUS:
            Scanner_Scan(s, tok);
            // for ---a
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_NEGATE, t->type, t, NULL, 0);
            break;
        case T_INVERT:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_INVERT, t->type, t, NULL, 0);
            break;
        case T_LOGNOT:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_LOGNOT, t->type, t, NULL, 0);
            break;
        case T_DEC:
            Scanner_Scan(s, tok);
            t = ASTnode_Postfix(c, s, st, tok, ctx);
            if (t->op != A_IDENT) {
                lfatal(s, "SyntaxError: -- must be followed by an identifier");
            }
            t = ASTnode_NewUnary(A_PREDEC, t->type, t, NULL, 0);
            break;
        default:
            t = primary(c, s, st, tok, ctx);
    }
    return t;
}

static ASTnode ASTnode_Postfix(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {
    SymTableEntry enumPtr;

    // Converts enum to a specific int
    if ((enumPtr = SymTable_FindEnumVal(st, s)) != NULL) {
        // Scanner_Scan(s, tok);
        debug("hit a enum word");
        // ! bug: for some reason a extra token is consumed i think
        //! semi colon is ignored
        return ASTnode_NewLeaf(A_INTLIT, P_INT, NULL, enumPtr->posn);
    }

    SymTableEntry var;

    Scanner_Scan(s, tok);
    if (tok->token == T_LPAREN) return ASTnode_FuncCall(c, s, st, tok, ctx);
    if (tok->token == T_LBRACKET) return ASTnode_ArrayRef(c, s, st, tok, ctx);
    if (tok->token == T_DOT)
        return ASTnode_MemberAccess(s, st, tok, ctx, false);
    if (tok->token == T_ARROW)
        return ASTnode_MemberAccess(s, st, tok, ctx, true);

    // TODO: cant remember why i need to reject the token
    Scanner_RejectToken(s, tok);

    if ((var = SymTable_FindSymbol(st, s, ctx)) == NULL ||
        var->stype != S_VAR) {
        debug("fuck :(");
        lfatala(s, "UndefinedError: Undefined variable %s", s->text);
    }

    debug("Variable %p", var);

    // * Doesn't work for a++++ i think

    switch (tok->token) {
        case T_INC:
            Scanner_Scan(s, tok);
            debug("inc");
            return ASTnode_NewLeaf(A_POSTINC, var->type, var, 0);
        case T_DEC:
            Scanner_Scan(s, tok);
            return ASTnode_NewLeaf(A_POSTDEC, var->type, var, 0);
        default:
            debug("variable %d %p", var->type, var);
            return ASTnode_NewLeaf(A_IDENT, var->type, var, 0);
    }
}

static ASTnode ASTnode_MemberAccess(Scanner s, SymTable st, Token tok,
                                    Context ctx, bool isPtr) {
    ASTnode left, right;
    // the struct
    SymTableEntry compVar;
    SymTableEntry typePtr;

    // the member
    SymTableEntry m = NULL;

    if ((compVar = SymTable_FindSymbol(st, s, ctx)) == NULL) {
        fatala("UndefinedError: Undefined variable %s", s->text);
    }
    if (isPtr && compVar->type != pointer_to(P_STRUCT) &&
        compVar->type != pointer_to(P_UNION)) {
        fatala("TypeError: %s is not a pointer to a struct/union", s->text);
    }
    if (!isPtr && compVar->type != P_STRUCT && compVar->type != P_UNION) {
        fatala("TypeError: %s is not a struct/union", s->text);
    }

    if (isPtr) {
        left = ASTnode_NewLeaf(A_IDENT, pointer_to(P_STRUCT), compVar, 0);
    } else {
        left = ASTnode_NewLeaf(A_ADDR, P_STRUCT, compVar, 0);
    }

    left->rvalue = 1;

    typePtr = compVar->ctype;

    Scanner_Scan(s, tok);
    ident(s, tok);

    for (m = typePtr->member; m != NULL; m = m->next) {
        if (strcmp(m->name, s->text) == 0) break;
    }

    debug("Member %s", s->text);

    if (m == NULL) fatala("UndefinedError: Undefined member %s", s->text);

    right = ASTnode_NewLeaf(A_INTLIT, P_INT, NULL, m->posn);

    left = ASTnode_New(A_ADD, pointer_to(m->type), left, NULL, right, NULL, 0);
    left = ASTnode_NewUnary(A_DEREF, m->type, left, NULL, 0);

    debug("left struct access");
    Scanner_RejectToken(s, tok);
    return left;
}

static ASTnode peek_statement(Compiler c, Scanner s, SymTable st, Token tok,
                              Context ctx) {
    ASTnode t;
    match(s, tok, T_PEEK, "peek");
    lparen(s, tok);
    t = ASTnode_Order(c, s, st, tok, ctx);
    t->rvalue = true;
    rparen(s, tok);
    Scanner_RejectToken(s, tok);
    return ASTnode_NewUnary(A_PEEK, P_INT, t, NULL, 0);
}