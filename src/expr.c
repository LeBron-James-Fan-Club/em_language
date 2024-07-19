#include "expr.h"

static enum ASTOP arithOp(Scanner s, enum OPCODES tok);
static int precedence(enum ASTOP op);
static ASTnode primary(Scanner s, SymTable st, Token t);
static void orderOp(Scanner s, SymTable st, Token t, ASTnode *stack,
                    enum ASTOP *opStack, int *top, int *opTop);

static ASTnode ASTnode_FuncCall(Scanner s, SymTable st, Token tok);
static ASTnode ASTnode_ArrayRef(Scanner s, SymTable st, Token tok);

static ASTnode ASTnode_Prefix(Scanner s, SymTable st, Token tok);
static ASTnode ASTnode_Postfix(Scanner s, SymTable st, Token tok);

static enum ASTOP arithOp(Scanner s, enum OPCODES tok) {
    if (tok > T_EOF && tok < T_INTLIT) return (enum ASTOP)tok;
    fprintf(stderr, "Error: Syntax error on line %d token %d\n", s->line, tok);
    exit(-1);
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
            return 1;
        default:
            fprintf(stderr, "Error: No proper precedence for operator %d\n",
                    op);
            exit(-1);
    }
}

static bool rightAssoc(enum ASTOP op) { return op == T_ASSIGN; }

static ASTnode primary(Scanner s, SymTable st, Token t) {
    int id;
    switch (t->token) {
        case T_STRLIT:
            id = SymTable_GlobAdd(st, s, P_CHARPTR, S_VAR, 1, true);
            SymTable_GlobSetText(st, s, id);
            return ASTnode_NewLeaf(A_STRLIT, P_CHARPTR, id);
        case T_INTLIT:
            if (t->intvalue >= 0 && t->intvalue < 256) {
                return ASTnode_NewLeaf(A_INTLIT, P_CHAR, t->intvalue);
            } else {
                return ASTnode_NewLeaf(A_INTLIT, P_INT, t->intvalue);
            }
        case T_IDENT:
            return ASTnode_Postfix(s, st, t);
        default:
            break;
    }
    return NULL;
}

static void orderOp(Scanner s, SymTable st, Token t, ASTnode *stack,
                    enum ASTOP *opStack, int *top, int *opTop) {
    ASTnode ltemp, rtemp;
    if (*opTop < 0) {
        fprintf(stderr, "Error: Syntax error on line (opTop < 0) %d\n",
                s->line);
        exit(-1);
    }

    enum ASTOP op = opStack[(*opTop)--];

    if (*top < 1) {
        fprintf(stderr, "Error: Syntax error on line %d\n", s->line);
        exit(-1);
    }

    ASTnode right = stack[(*top)--];
    ASTnode left = stack[(*top)--];

    if (op == A_ASSIGN) {
        right->rvalue = 1;
        right = modify_type(right, left->type, 0);
        if (right == NULL) {
            fprintf(stderr,
                    "Error: Syntax error on line %d, "
                    "incompatible expression\n",
                    s->line);
            exit(-1);
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
            fprintf(stderr,
                    "Error: Incompatible types in expression on "
                    "line %d\n",
                    s->line);
            exit(-1);
        }
        if (ltemp != NULL) left = ltemp;
        if (rtemp != NULL) right = rtemp;
    }
    stack[++(*top)] = ASTnode_New(op, left->type, left, NULL, right, 0);
}

ASTnode ASTnode_Order(Scanner s, SymTable st, Token t) {
    ASTnode *stack = calloc(MAX_STACK, sizeof(ASTnode));
    enum ASTOP *opStack = calloc(MAX_STACK, sizeof(enum ASTOP));
    enum ASTOP curOp;
    int parenCount = 0;

    bool expectPreOp = true;

    int top = -1, opTop = -1;
    do {
#if DEBUG
        printf("Token: %d\n", t->token);
#endif
        switch (t->token) {
            case T_SEMI:
            case T_EOF:
            case T_COMMA:
                // Breaks out of loop if parenthesis are not balanced
                goto out;
            case T_INTLIT:
            case T_IDENT:
            case T_STRLIT:
                stack[++top] = primary(s, st, t);
                expectPreOp = false;
                break;
            case T_LPAREN:
#if DEBUG
                printf("LPAREN\n");
#endif
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
                    stack[++top] = ASTnode_Prefix(s, st, t);
                    break;
                }
                curOp = arithOp(s, t->token);
                while (opTop != -1 && opStack[opTop] != A_STARTPAREN &&
                       (precedence(opStack[opTop]) >= precedence(curOp) ||
                        (rightAssoc(opStack[opTop]) &&
                         precedence(opStack[opTop]) == precedence(curOp)))) {
                    orderOp(s, st, t, stack, opStack, &top, &opTop);
                }
                opStack[++opTop] = curOp;
                expectPreOp = true;
        }
    } while (Scanner_Scan(s, t) || parenCount > 0);
out:

    while (opTop != -1) {
        orderOp(s, st, t, stack, opStack, &top, &opTop);
    }

#if DEBUG
    printf("Top: %d\n", top);
#endif

    ASTnode n = stack[top];

#if DEBUG
    printf("Dumping AST\n");
    ASTnode_Dump(n, st, NO_LABEL, 0);
#endif

    free(stack);
    free(opStack);
    return n;
}

static ASTnode ASTnode_FuncCall(Scanner s, SymTable st, Token tok) {
    ASTnode t;
    int id;
    if ((id = SymTable_GlobFind(st, s, S_FUNC)) == -1) {
        fprintf(stderr, "Error: Undefined function %s on line %d\n", s->text,
                s->line);
        exit(-1);
    }
    lparen(s, tok);

    t = ASTnode_Order(s, st, tok);
    t = ASTnode_NewUnary(A_FUNCCALL, st->Gsym[id].type, t, id);

    rparen(s, tok);

    Scanner_RejectToken(s, tok);
    return t;
}

static ASTnode ASTnode_ArrayRef(Scanner s, SymTable st, Token tok) {
    //! BUG: For some reason intlit becomes top of stack
    //! only copying the right value
    ASTnode left, right;
    int id;
    if ((id = SymTable_GlobFind(st, s, S_ARRAY)) == -1) {
        fprintf(stderr, "Error: Undeclared array %s on line %d\n", s->text,
                s->line);
        exit(-1);
    }
    left = ASTnode_NewLeaf(A_ADDR, st->Gsym[id].type, id);
    Scanner_Scan(s, tok);
    right = ASTnode_Order(s, st, tok);
    right->rvalue = 1;

    match(s, tok, T_RBRACKET, "]");
#if DEBUG
    printf("Consumed right bracket\n");
#endif
    if (!inttype(right->type)) {
        fprintf(stderr, "Error: Array index must be an integer on line %d\n",
                s->line);
        exit(-1);
    }
    right = modify_type(right, left->type, A_ADD);

    left = ASTnode_New(A_ADD, st->Gsym[id].type, left, NULL, right, 0);

    Scanner_RejectToken(s, tok);
    return ASTnode_NewUnary(A_DEREF, value_at(left->type), left, 0);
}

static ASTnode ASTnode_Prefix(Scanner s, SymTable st, Token tok) {
    ASTnode t;
    switch (tok->token) {
        case T_AMPER:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(s, st, tok);
            if (t->op != A_IDENT) {
                fprintf(stderr, "Error: Expected identifier on line %d\n",
                        s->line);
                exit(-1);
            }
            t->op = A_ADDR;
            t->type = pointer_to(t->type);
            break;
        case T_STAR:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(s, st, tok);
            if (t->op != A_IDENT && t->op != A_DEREF) {
                fprintf(stderr,
                        "Error: * Operator must be followed by an identifier "
                        "or *, occurred on line %d\n",
                        s->line);
                exit(-1);
            }
            t = ASTnode_NewUnary(A_DEREF, value_at(t->type), t, 0);
            break;
        case T_INC:
            Scanner_Scan(s, tok);
#if DEBUG
            printf("INC\n");
#endif
            t = ASTnode_Postfix(s, st, tok);
#if DEBUG
            printf("%p\n", t);
#endif
            // temp check - cause inc also sets the rvalue
            if (t->op != A_IDENT) {
                fprintf(
                    stderr,
                    "Error: ++ must be followed by an identifier on line %d\n",
                    s->line);
                exit(-1);
            }
            t = ASTnode_NewUnary(A_PREINC, t->type, t, 0);
            break;
        case T_MINUS:
            Scanner_Scan(s, tok);
            // for ---a
            t = ASTnode_Prefix(s, st, tok);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_NEGATE, t->type, t, 0);
            break;
        case T_INVERT:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(s, st, tok);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_INVERT, t->type, t, 0);
            break;
        case T_LOGNOT:
            Scanner_Scan(s, tok);
            t = ASTnode_Prefix(s, st, tok);
            t->rvalue = 1;
            t = ASTnode_NewUnary(A_LOGNOT, t->type, t, 0);
            break;
        case T_DEC:
            Scanner_Scan(s, tok);
            t = ASTnode_Postfix(s, st, tok);
            if (t->op != A_IDENT) {
                fprintf(
                    stderr,
                    "Error: -- must be followed by an identifier on line %d\n",
                    s->line);
                exit(-1);
            }
            t = ASTnode_NewUnary(A_PREDEC, t->type, t, 0);
            break;
        default:
            t = primary(s, st, tok);
    }
    return t;
}

static ASTnode ASTnode_Postfix(Scanner s, SymTable st, Token tok) {
    int id;

    Scanner_Scan(s, tok);
    if (tok->token == T_LPAREN) return ASTnode_FuncCall(s, st, tok);
    printf("Checking for array ref\n");
    if (tok->token == T_LBRACKET) return ASTnode_ArrayRef(s, st, tok);
    Scanner_RejectToken(s, tok);

    if ((id = SymTable_GlobFind(st, s, S_VAR)) == -1) {
        fprintf(stderr, "Error: Unknown variable %s on line %d\n", s->text,
                s->line);
        exit(-1);
    }

    switch (tok->token) {
        case T_INC:
            Scanner_Scan(s, tok);
            return ASTnode_NewLeaf(A_POSTINC, st->Gsym[id].type, id);
        case T_DEC:
            Scanner_Scan(s, tok);
            return ASTnode_NewLeaf(A_POSTDEC, st->Gsym[id].type, id);
        default:
            return ASTnode_NewLeaf(A_IDENT, st->Gsym[id].type, id);
    }
}