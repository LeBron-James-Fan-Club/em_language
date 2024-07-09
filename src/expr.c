#include "expr.h"

static enum ASTOP arithop(Scanner s, enum OPCODES tok);
static int precedence(enum ASTOP op);
static ASTnode primary(Scanner s, SymTable st, Token t);

// maybe move this to tokens.c or scan.c
static enum ASTOP arithop(Scanner s, enum OPCODES tok) {
    if (tok > T_EOF && tok < T_INTLIT) return (enum ASTOP)tok;
    fprintf(stderr, "Error: Syntax error on line %d token %d\n", s->line, tok);
    exit(-1);
}

//* Might put this shit in a diff file

static int precedence(enum ASTOP op) {
    // TODO: Make this an array later maybe
    switch (op) {
        case T_EQ:
        case T_NE:
        case T_LT:
        case T_GT:
        case T_LE:
        case T_GE:
            return 3;

        case T_MODULO:
        case T_SLASH:
        case T_STAR:
            return 2;

        case T_PLUS:
        case T_MINUS:
            return 1;

        default:
            return 0;
    }
}

static ASTnode primary(Scanner s, SymTable st, Token t) {
    int id;
    switch (t->token) {
        case T_INTLIT:
            // any int literal is automatically casted as char 0 <= x < 256
            if (t->intvalue >= 0 && t->intvalue < 256)
                return ASTnode_NewLeaf(A_INTLIT, P_CHAR, t->intvalue);
            else
                return ASTnode_NewLeaf(A_INTLIT, P_INT, t->intvalue);
        case T_IDENT:
            Scanner_Scan(s, t);
            if (t->token == T_LPAREN) {
                return ASTnode_FuncCall(s, st, t);
            } else {
                Scanner_RejectToken(s, t);
                if ((id = SymTable_GlobFind(st, s, S_VAR)) == -1) {
                    fprintf(stderr, "Error: Unknown variable %s on line %d\n",
                            s->text, s->line);
                    exit(-1);
                }
                return ASTnode_NewLeaf(A_IDENT, st->Gsym[id].type, id);
            }
        default:
            // stfu
            break;
    }
    // Shouldn't occur
    return NULL;
}

// Stunting yard algo was good before
// but didnt go so well with register assignment stuff

ASTnode ASTnode_Order(Scanner s, SymTable st, Token t) {
    ASTnode *stack = calloc(MAX_STACK, sizeof(ASTnode));
    enum ASTOP *opStack = calloc(MAX_STACK, sizeof(enum ASTOP));
    enum ASTOP curOp;
    ASTnode ltemp, rtemp;

    bool expectPreOp = true;

    //! SCANNER_SCAN MUST BE CALLED FIRST
    //! IN ORDER FOR THIS TO WORK!

    int top = -1, opTop = -1;
    do {
        switch (t->token) {
            //! Probs bug - we need to scan
            //! T_STAR in but its also used for multiplication
            case T_INTLIT:
            case T_IDENT:
                stack[++top] = primary(s, st, t);
                expectPreOp = false;
                break;
            // TODO: IMPLEMENT PERTHENESIS
            default:
                if ((t->token == T_STAR || t->token == T_AMPER) &&
                    expectPreOp) {
                    stack[++top] = ASTnode_Prefix(s, st, t);
                    break;
                }
                curOp = arithop(s, t->token);
                while (opTop != -1 &&
                       precedence(opStack[opTop]) >= precedence(curOp)) {
                    enum ASTOP op = opStack[opTop--];

                    ASTnode right = stack[top--];
                    ASTnode left = stack[top--];

                    printf("LEF CHECK\n");
                    ltemp = modify_type(left, right->type, op, false);
                    printf("RIG CHECK\n");
                    rtemp = modify_type(right, left->type, op, false);

                    if (ltemp == NULL && rtemp == NULL) {
                        fprintf(stderr,
                                "Error: Incompatable types in expression on "
                                "line %d\n",
                                s->line);
                        exit(-1);
                    }
                    if (ltemp != NULL) left = ltemp;
                    if (rtemp != NULL) right = rtemp;

                    stack[++top] =
                        ASTnode_New(op, left->type, left, NULL, right, 0);
                }
                opStack[++opTop] = curOp;
                expectPreOp = true;
        }
    } while (Scanner_Scan(s, t));

    while (opTop != -1) {
        enum ASTOP op = opStack[opTop--];
        if (top < 1) {
            fprintf(stderr, "Error: Syntax error on line %d\n", s->line);
            exit(-1);
        }
        ASTnode right = stack[top--];
        ASTnode left = stack[top--];

        printf("LEF CHECK\n");
        ltemp = modify_type(left, right->type, op, false);
        printf("RIG CHECK\n");
        rtemp = modify_type(right, left->type, op, false);

        if (ltemp == NULL && rtemp == NULL) {
            fprintf(stderr,
                    "Error: Incompatable types in expression on line %d\n",
                    s->line);
            exit(-1);
        }
        if (ltemp != NULL) left = ltemp;
        if (rtemp != NULL) right = rtemp;

        stack[++top] = ASTnode_New(op, left->type, left, NULL, right, 0);
    }

    ASTnode n = stack[top];

    free(stack);
    free(opStack);

    return n;
}

void ASTnode_PrintTree(ASTnode n) {
    static char *ASTop[] = {"+", "-", "*", "/", "%"};

    if (n->op == A_INTLIT)
        printf("INTLIT: %d\n", n->v.id);
    else
        printf("%d Operator: %s\n", n->op, ASTop[n->op]);

    if (n->left) ASTnode_PrintTree(n->left);
    if (n->right) ASTnode_PrintTree(n->right);
}

ASTnode ASTnode_FuncCall(Scanner s, SymTable st, Token tok) {
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

    // Needed for the scanner to check for semicolon in main loop
    Scanner_RejectToken(s, tok);
    return t;
}

ASTnode ASTnode_Prefix(Scanner s, SymTable st, Token tok) {
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
                        "Error: * Operatior must be followed by an identifier "
                        "or *, occurred on line %d\n",
                        s->line);
                exit(-1);
            }
            t = ASTnode_NewUnary(A_DEREF, value_at(t->type), t, 0);
            break;
        default:
            t = primary(s, st, tok);
    }
    return t;
}