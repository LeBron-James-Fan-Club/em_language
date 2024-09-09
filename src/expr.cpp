#include <stdlib.h>
#include <string.h>

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
static ASTnode paren_expression(Compiler c, Scanner s, SymTable st, Token t,
                                Context ctx);
static void orderOp(Compiler c, Scanner s, SymTable st, Token t, Context ctx,
                    ASTnode *stack, enum ASTOP *opStack, int *top, int *opTop);

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

static int precedence(enum ASTOP op) {
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
            fatala("InternalError: No proper precedence for operator %d", op);
    }
}

static bool rightAssoc(enum ASTOP op) {
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
                char *name = st->SymTableEntry_MakeAnon(nullptr);
                var = st->SymTable_AddGlob(name, pointer_to(P_CHAR), nullptr,
                                       S_VAR, C_GLOBAL, 1, 0);
                free(name);
                st->SymTable_SetText(s, var);
                n = ASTnode_NewLeaf(A_STRLIT, pointer_to(P_CHAR), nullptr, var, 0);
            }
            break;
        case T_INTLIT:
            if (t->intvalue >= 0 && t->intvalue < 256) {
                n = ASTnode_NewLeaf(A_INTLIT, P_CHAR, nullptr, nullptr, t->intvalue);
            } else {
                n = ASTnode_NewLeaf(A_INTLIT, P_INT, nullptr, nullptr, t->intvalue);
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
            if ((enumPtr = st->SymTable_FindEnumVal(s)) != nullptr) {
                return ASTnode_NewLeaf(A_INTLIT, P_INT, nullptr, nullptr,
                                       enumPtr->posn);
            }
            if ((var = st->SymTable_FindSymbol(s, ctx)) == nullptr) {
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

    return n;
}

static ASTnode paren_expression(Compiler c, Scanner s, SymTable st, Token t,
                                Context ctx) {
    ASTnode n;
    enum ASTPRIM type = P_NONE;
    SymTableEntry cType = nullptr;

    Scanner_Scan(s, t);

    switch (t->token) {
        case T_IDENT:
            if (st->SymTable_FindTypeDef(s) == nullptr) {
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
            type = parse_cast(c, s, st, t, ctx, &cType);
            rparen(s, t);
        default:
            // * rest of the expression (int)b <- self
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
        n = ASTnode_NewUnary(A_CAST, type, n, cType, nullptr, 0);
    }

    // If shit was scanned in before
    Scanner_RejectToken(s, t);

    return n;
}

static void orderOp(Compiler c, Scanner s, SymTable st, Token t, Context ctx,
                    ASTnode *stack, enum ASTOP *opStack, int *top, int *opTop) {
    ASTnode ltemp, rtemp;
    if (*opTop < 0) {
        lfatal(s, "SyntaxError: (opTop < 0)");
    }

    enum ASTOP op = opStack[(*opTop)--];

    if (*top < 1) {
        lfatal(s, "SyntaxError: (top < 1)");
    }

    debug("%d top", *top);
    ASTnode right = stack[(*top)--];
    ASTnode left = stack[(*top)--];

    // ! THIS MIGHT CAUSE BUG
    if (op == A_TERNARY) {
        debug("Ternary operator");
        match(s, t, T_COLON, ":");
        ltemp = ASTnode_Order(c, s, st, t, ctx);
        stack[++(*top)] = ASTnode_New(A_TERNARY, right->type, left, right,
                                      ltemp, right->ctype, nullptr, 0);
        return;

    } else if (op == A_ASSIGN) {
        right->rvalue = 1;
        debug("The assign called it");
        right = modify_type(right, left->type, left->ctype, A_NONE);
        if (right == nullptr) {
            lfatal(s, "SyntaxError: incompatible types in assignment");
        }
        ltemp = left;
        left = right;
        right = ltemp;
    } else {
        debug("The assign did not call it");
        debug("op %d", op);
        right->rvalue = 1;
        left->rvalue = 1;
        ltemp = modify_type(left, right->type, right->ctype, op);
        rtemp = modify_type(right, left->type, left->ctype, op);

        if (ltemp == nullptr && rtemp == nullptr) {
            lfatal(s, "SyntaxError: Incompatible types in expression");
        }

        if (ltemp != nullptr) left = ltemp;
        if (rtemp != nullptr) right = rtemp;
    }
    stack[++(*top)] =
        ASTnode_New(op, left->type, left, nullptr, right, left->ctype, nullptr, 0);
    debug("%d top", *top);
}

ASTnode ASTnode_Order(Compiler c, Scanner s, SymTable st, Token t,
                      Context ctx) {
    ASTnode *stack = new ASTnode[MAX_STACK];
    enum ASTOP *opStack = new enum ASTOP[MAX_STACK];
    enum ASTOP curOp;

    bool expectPreOp = true;

    int top = -1, opTop = -1;

    debug("GET IN ORDER");

    do {
        debug("token %s", t->tokstr);

        switch (t->token) {
            case T_SEMI:
            case T_EOF:
            case T_COMMA:
            case T_COLON:
            case T_RPAREN:
            case T_RBRACKET:
            case T_RBRACE:
                // Breaks out of loop if parenthesis are not balanced
                debug("(breaking out) top %d token %s", top, t->tokstr);
                debug("GTFO OUT OF ORDER NOW");
                goto out;
            case T_LPAREN:
            case T_INTLIT:
            case T_IDENT:
            case T_STRLIT:
            case T_PEEK:
            case T_SIZEOF:
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

                //! THE 1:1 THING HAPPENS HERE

                // TODO: Handle the other assignment ops

                // The last node added
                // Should be a identifier
                // If not, then it's a syntax error

                curOp = arithOp(s, t->token);
                debug("curOp %d %s", curOp, t->tokstr);
                while (opTop != -1 &&
                       (precedence(opStack[opTop]) >= precedence(curOp) ||
                        (rightAssoc(opStack[opTop]) &&
                         precedence(opStack[opTop]) == precedence(curOp)))) {
                    orderOp(c, s, st, t, ctx, stack, opStack, &top, &opTop);
                }
                opStack[++opTop] = curOp;

                expectPreOp = true;
        }

        Scanner_Scan(s, t);
    } while (true);
out:

    debug("Top op: %d", stack[top]->op);
    while (opTop != -1) {
        orderOp(c, s, st, t, ctx, stack, opStack, &top, &opTop);
    }

    debug("Top op: %d", stack[top]->op);
    if (stack[top]->left) {
        debug("Left %d", stack[top]->left->op);
    }
    if (stack[top]->right) {
        debug("Right %d", stack[top]->right->op);
    }

    if (top != 0) {
        lfatal(s, "SyntaxError: Invalid expression");
    }

    ASTnode n = stack[top];

    delete[] stack;
    delete[] opStack;

    // Might break it?
    n->rvalue = 1;
    return n;
}

static ASTnode ASTnode_FuncCall(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx) {
    ASTnode t;
    SymTableEntry var;

    if ((var = st->SymTable_FindSymbol(s, ctx)) == nullptr) {
        lfatala(s, "UndefinedError: Undefined function %s", s->text);
    }

    lparen(s, tok);

    t = expression_list(c, s, st, tok, ctx, T_RPAREN);

    t = ASTnode_NewUnary(A_FUNCCALL, var->type, t, var->ctype, var, 0);

    rparen(s, tok);

    Scanner_RejectToken(s, tok);
    return t;
}

ASTnode expression_list(Compiler c, Scanner s, SymTable st, Token tok,
                        Context ctx, enum OPCODES endToken) {
    ASTnode tree = nullptr, child = nullptr;
    int exprCount = 0;
    while (tok->token != endToken) {
        child = ASTnode_Order(c, s, st, tok, ctx);
        exprCount++;

        debug("expression generation %d", exprCount);
        debug("child %p", child);

        tree = ASTnode_New(A_GLUE, P_NONE, tree, nullptr, child, nullptr, nullptr,
                           exprCount);

        if (tok->token == endToken) break;

        match(s, tok, T_COMMA, ",");
    }
    return tree;
}

static ASTnode ASTnode_ArrayRef(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx, ASTnode left) {
    ASTnode right;
    SymTableEntry var;

    if (!ptrtype(left->type)) {
        lfatala(s, "TypeError: Array index must be a pointer or array, %s",
                s->text);
    }

    // eat [
    Scanner_Scan(s, tok);

    right = ASTnode_Order(c, s, st, tok, ctx);

    match(s, tok, T_RBRACKET, "]");

    if (!inttype(right->type)) {
        fatal("TypeError: Array index must be an integer");
    }

    left->rvalue = 1;

    right = modify_type(right, left->type, left->ctype, A_ADD);

    left =
        ASTnode_New(A_ADD, left->type, left, nullptr, right, left->ctype, nullptr, 0);
    Scanner_RejectToken(s, tok);

    return ASTnode_NewUnary(A_DEREF, value_at(left->type), left, left->ctype,
                            nullptr, 0);
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
            t = ASTnode_NewUnary(A_DEREF, value_at(t->type), t, t->ctype, nullptr,
                                 0);
            break;
        case T_INC:
            Scanner_Scan(s, tok);
            t = ASTnode_Postfix(c, s, st, tok, ctx);

            // * temp check - cause inc also sets the rvalue
            if (t->op != A_IDENT) {
                lfatal(s, "SyntaxError: ++ must be followed by an identifier");
            }
            debug("pre inc");
            t = ASTnode_NewUnary(A_PREINC, t->type, t, t->ctype, nullptr, 0);
            break;
        case T_MINUS:
            Scanner_Scan(s, tok);
            // for ---a
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_NEGATE, t->type, t, t->ctype, nullptr, 0);
            break;
        case T_INVERT:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_INVERT, t->type, t, t->ctype, nullptr, 0);
            break;
        case T_LOGNOT:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(c, s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_LOGNOT, t->type, t, t->ctype, nullptr, 0);
            break;
        case T_DEC:
            Scanner_Scan(s, tok);
            t = ASTnode_Postfix(c, s, st, tok, ctx);
            if (t->op != A_IDENT) {
                lfatal(s, "SyntaxError: -- must be followed by an identifier");
            }
            t = ASTnode_NewUnary(A_PREDEC, t->type, t, t->ctype, nullptr, 0);
            break;
        default:
            t = ASTnode_Postfix(c, s, st, tok, ctx);
    }
    return t;
}

static ASTnode ASTnode_Postfix(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {

    ASTnode n = primary(c, s, st, tok, ctx);

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
    SymTableEntry m = nullptr;

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

    for (m = typePtr->member; m != nullptr; m = m->next) {
        if (!strcmp(m->name, s->text)) break;
    }

    if (m == nullptr) fatala("UndefinedError: Undefined member %s", s->text);

    left->rvalue = 1;

    right = ASTnode_NewLeaf(A_INTLIT, P_INT, nullptr, nullptr, m->posn);

    left = ASTnode_New(A_ADD, pointer_to(m->type), left,  nullptr, right,
                       m->ctype,nullptr, 0);
    left = ASTnode_NewUnary(A_DEREF, m->type, left, m->ctype, nullptr, 0);

    Scanner_RejectToken(s, tok);

    return left;
}

static ASTnode peek_operator(Compiler c, Scanner s, SymTable st, Token tok,
                             Context ctx) {
    ASTnode n;
    match(s, tok, T_PEEK, "peek");
    lparen(s, tok);
    n = ASTnode_Order(c, s, st, tok, ctx);
    n->rvalue = true;
    rparen(s, tok);
    Scanner_RejectToken(s, tok);
    return ASTnode_NewUnary(A_PEEK, P_INT, n, nullptr, nullptr, 0);
}

static ASTnode sizeof_operator(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {
    enum ASTPRIM type;
    enum STORECLASS _class;
    int size;
    SymTableEntry cType;

    debug("sizeof");
    match(s, tok, T_SIZEOF, "sizeof");
    lparen(s, tok);
    type = static_cast<enum ASTPRIM>(parse_stars(s, tok, parse_type(c, s, st, tok, ctx, &cType, &_class)));
    debug("cType %p", cType);
    size = type_size(type, cType);
    rparen(s, tok);
    Scanner_RejectToken(s, tok);

    return ASTnode_NewLeaf(A_INTLIT, P_INT, nullptr, nullptr, size);
}