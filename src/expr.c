#include "expr.h"

#include "ast.h"
#include "defs.h"
#include "misc.h"
#include "sym.h"
#include "types.h"

static enum ASTOP arithOp(Scanner s, enum OPCODES tok);
static int precedence(enum ASTOP op);
static ASTnode primary(Scanner s, SymTable st, Token t, Context ctx);
static void orderOp(Scanner s, SymTable st, Token t, ASTnode *stack,
                    enum ASTOP *opStack, int *top, int *opTop);

static ASTnode ASTnode_FuncCall(Scanner s, SymTable st, Token tok, Context ctx);
static ASTnode ASTnode_ArrayRef(Scanner s, SymTable st, Token tok, Context ctx);

static ASTnode ASTnode_Prefix(Scanner s, SymTable st, Token tok, Context ctx);
static ASTnode ASTnode_Postfix(Scanner s, SymTable st, Token tok, Context ctx);

static ASTnode peek_statement(Scanner s, SymTable st, Token tok, Context ctx);
static ASTnode expression_list(Scanner s, SymTable st, Token tok, Context ctx);

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

static ASTnode primary(Scanner s, SymTable st, Token t, Context ctx) {
    SymTableEntry var;
    switch (t->token) {
        case T_STRLIT:
            var = SymTable_AddGlob(st, s, pointer_to(P_CHAR), NULL, S_VAR, 1, true);
            SymTable_SetText(st, s, var);
            return ASTnode_NewLeaf(A_STRLIT, pointer_to(P_CHAR), var, 0);
        case T_INTLIT:
            if (t->intvalue >= 0 && t->intvalue < 256) {
                return ASTnode_NewLeaf(A_INTLIT, P_CHAR, NULL, t->intvalue);
            } else {
                return ASTnode_NewLeaf(A_INTLIT, P_INT, NULL, t->intvalue);
            }
        case T_PEEK:
            // printf("PEEK\n");
            return peek_statement(s, st, t, ctx);
        case T_IDENT:
            return ASTnode_Postfix(s, st, t, ctx);
        default:
            break;
    }
    return NULL;
}

static void orderOp(Scanner s, SymTable st, Token t, ASTnode *stack,
                    enum ASTOP *opStack, int *top, int *opTop) {
    ASTnode ltemp, rtemp;
    if (*opTop < 0) {
        lfatal(s, "Syntax Error: (opTop < 0)");
    }

    enum ASTOP op = opStack[(*opTop)--];

    if (*top < 1) {
        lfatal(s, "Syntax Error: (top < 1)");
    }

    ASTnode right = stack[(*top)--];
    ASTnode left = stack[(*top)--];

    if (op == A_ASSIGN) {
        right->rvalue = 1;
        right = modify_type(right, left->type, 0);
        if (right == NULL) {
            lfatal(s, "Syntax Error: incompatible types in assignment");
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
            lfatal(s, "Syntax Error: Incompatible types in expression");
        }

        if (ltemp != NULL) left = ltemp;
        if (rtemp != NULL) right = rtemp;
    }
    stack[++(*top)] = ASTnode_New(op, left->type, left, NULL, right, NULL, 0);
}

ASTnode ASTnode_Order(Scanner s, SymTable st, Token t, Context ctx) {
    ASTnode *stack = calloc(MAX_STACK, sizeof(ASTnode));
    enum ASTOP *opStack = calloc(MAX_STACK, sizeof(enum ASTOP));
    enum ASTOP curOp;
    int parenCount = 0;

    bool expectPreOp = true;

    int top = -1, opTop = -1;
    do {
        debug("Token %d", t->token);

        switch (t->token) {
            case T_SEMI:
            case T_EOF:
            case T_COMMA:
                // Breaks out of loop if parenthesis are not balanced
                goto out;
            case T_INTLIT:
            case T_IDENT:
            case T_STRLIT:
            case T_PEEK:
                stack[++top] = primary(s, st, t, ctx);
                expectPreOp = false;
                break;
            case T_LPAREN:
                opStack[++opTop] = A_STARTPAREN;
                parenCount++;
                break;
            case T_RPAREN:
                while (opTop != -1 && opStack[opTop] != A_STARTPAREN) {
                    orderOp(s, st, t, stack, opStack, &top, &opTop);
                }
                opTop--;  // Delete the left parenthesis
                parenCount--;
                break;
            default:
                if ((t->token == T_STAR || t->token == T_AMPER ||
                     t->token == T_INC || t->token == T_DEC) &&
                    expectPreOp) {
                    // need to somehow add rvalue here
                    stack[++top] = ASTnode_Prefix(s, st, t, ctx);
                    break;
                }
                curOp = arithOp(s, t->token);
                while (opTop != -1 && opStack[opTop] != A_STARTPAREN &&
                       (precedence(opStack[opTop]) >= precedence(curOp) ||
                        (rightAssoc(opStack[opTop]) &&
                         precedence(opStack[opTop]) == precedence(curOp)))) {
                    orderOp(s, st, t, stack, opStack, &top, &opTop);
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
    } while (Scanner_Scan(s, t) || parenCount > 0);
out:

    while (opTop != -1) {
        orderOp(s, st, t, stack, opStack, &top, &opTop);
    }

    debug("Top: %d", top);

    ASTnode n = stack[top];

    free(stack);
    free(opStack);
    return n;
}

static ASTnode ASTnode_FuncCall(Scanner s, SymTable st, Token tok,
                                Context ctx) {
    ASTnode t;
    SymTableEntry var;
    if ((var = SymTable_FindSymbol(st, s, ctx)) == NULL) {
        lfatala(s, "UndefinedError: Undefined function %s", s->text);
    }

    lparen(s, tok);

    t = expression_list(s, st, tok, ctx);

    t = ASTnode_NewUnary(A_FUNCCALL, var->type, t, var, 0);

    rparen(s, tok);

    Scanner_RejectToken(s, tok);
    return t;
}

static ASTnode expression_list(Scanner s, SymTable st, Token tok, Context ctx) {
    ASTnode tree = NULL, child = NULL;
    int exprCount = 0;
    while (tok->token != T_RPAREN) {
        child = ASTnode_Order(s, st, tok, ctx);
        exprCount++;

        tree = ASTnode_New(A_GLUE, P_NONE, tree, NULL, child, NULL, exprCount);

        switch (tok->token) {
            case T_COMMA:
                Scanner_Scan(s, tok);
                break;
            case T_RPAREN:
                break;
            default:
                lfatal(s, "SyntaxError: Expected comma or right parenthesis");
        }
    }
    return tree;
}

static ASTnode ASTnode_ArrayRef(Scanner s, SymTable st, Token tok,
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
    right = ASTnode_Order(s, st, tok, ctx);
    right->rvalue = 1;

    match(s, tok, T_RBRACKET, "]");

    if (!inttype(right->type)) {
        fatal("TypeError: Array index must be an integer");
    }
    right = modify_type(right, left->type, A_ADD);

    left = ASTnode_New(A_ADD, var->type, left, NULL, right, NULL, 0);

    Scanner_RejectToken(s, tok);
    return ASTnode_NewUnary(A_DEREF, value_at(left->type), left, NULL, 0);
}

static ASTnode ASTnode_Prefix(Scanner s, SymTable st, Token tok, Context ctx) {
    ASTnode t;
    switch (tok->token) {
        case T_AMPER:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(s, st, tok, ctx);
            if (t->op != A_IDENT) {
                lfatal(s, "SyntaxError: Expected identifier");
            }
            t->op = A_ADDR;
            t->type = pointer_to(t->type);
            break;
        case T_STAR:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(s, st, tok, ctx);
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
            t = ASTnode_Postfix(s, st, tok, ctx);

            // * temp check - cause inc also sets the rvalue
            if (t->op != A_IDENT) {
                lfatal(s, "SyntaxError: ++ must be followed by an identifier");
            }
            t = ASTnode_NewUnary(A_PREINC, t->type, t, NULL, 0);
            break;
        case T_MINUS:
            Scanner_Scan(s, tok);
            // for ---a
            t = ASTnode_Prefix(s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_NEGATE, t->type, t, NULL, 0);
            break;
        case T_INVERT:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_INVERT, t->type, t, NULL, 0);
            break;
        case T_LOGNOT:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(s, st, tok, ctx);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_LOGNOT, t->type, t, NULL, 0);
            break;
        case T_DEC:
            Scanner_Scan(s, tok);
            t = ASTnode_Postfix(s, st, tok, ctx);
            if (t->op != A_IDENT) {
                lfatal(s, "SyntaxError: -- must be followed by an identifier");
            }
            t = ASTnode_NewUnary(A_PREDEC, t->type, t, NULL, 0);
            break;
        default:
            t = primary(s, st, tok, ctx);
    }
    return t;
}

static ASTnode ASTnode_Postfix(Scanner s, SymTable st, Token tok, Context ctx) {
    SymTableEntry var;

    Scanner_Scan(s, tok);
    if (tok->token == T_LPAREN) return ASTnode_FuncCall(s, st, tok, ctx);
    if (tok->token == T_LBRACKET) return ASTnode_ArrayRef(s, st, tok, ctx);
    Scanner_RejectToken(s, tok);

    if ((var = SymTable_FindSymbol(st, s, ctx)) == NULL) {
        lfatala(s, "UndefinedError: Undefined variable %s", s->text);
    }

    switch (tok->token) {
        case T_INC:
            Scanner_Scan(s, tok);
            return ASTnode_NewLeaf(A_POSTINC, var->type, var, 0);
        case T_DEC:
            Scanner_Scan(s, tok);
            return ASTnode_NewLeaf(A_POSTDEC, var->type, var, 0);
        default:
            return ASTnode_NewLeaf(A_IDENT, var->type, var, 0);
    }
}

static ASTnode peek_statement(Scanner s, SymTable st, Token tok, Context ctx) {
    ASTnode t;
    match(s, tok, T_PEEK, "peek");
    lparen(s, tok);
    t = ASTnode_Order(s, st, tok, ctx);
    t->rvalue = true;
    rparen(s, tok);
    Scanner_RejectToken(s, tok);
    return ASTnode_NewUnary(A_PEEK, P_INT, t, NULL, 0);
}