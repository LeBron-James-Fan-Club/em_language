#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

#include "gen.h"
#include "sym.h"
#include "scan.h"
#include "tokens.h"

#define MAX_STACK 500

static enum ASTOP arithop(Scanner s, enum OPCODES tok);

ASTnode ASTnode_New(enum ASTOP op, ASTnode left, ASTnode mid, ASTnode right,
                    int intvalue) {
    ASTnode n = calloc(1, sizeof(struct astnode));
    if (n == NULL) {
        fprintf(stderr, "Error: Unable to initialise ASTnode\n");
        exit(-1);
    }

    n->op = op;
    n->left = left;
    n->mid = mid;
    n->right = right;
    n->v.intvalue = intvalue;

    return n;
}

ASTnode ASTnode_NewLeaf(enum ASTOP op, int intvalue) {
    return ASTnode_New(op, NULL, NULL, NULL, intvalue);
}

ASTnode ASTnode_NewUnary(enum ASTOP op, ASTnode left, int intvalue) {
    return ASTnode_New(op, left, NULL, NULL, intvalue);
}

void ASTnode_Free(ASTnode this) {
    if (this->left) ASTnode_Free(this->left);
    if (this->right) ASTnode_Free(this->right);
    free(this);
}

// maybe move this to tokens.c or scan.c
static enum ASTOP arithop(Scanner s, enum OPCODES tok) {
    if (tok > T_EOF && tok < T_INTLIT) return tok;
    fprintf(stderr, "Error: Syntax error on line %d token %d\n", s->line, tok);
    exit(-1);
}

static ASTnode primary(Scanner s, Token t) {
    ASTnode n;
    if (t == NULL) {
        fprintf(stderr, "Error: Unable to initialise token\n");
        exit(-1);
    }
    char *tokens[] = {"EOF", "+", "-", "*", "/", "%", "INTLIT"};

    if (t->token == T_INTLIT) {
        n = ASTnode_NewLeaf(A_INTLIT, t->intvalue);
        Scanner_Scan(s, t);
    } else {
        fprintf(stderr, "Error: Unexpected token %s on line %d\n",
                tokens[t->token], s->line);
        exit(-1);
    }

    return n;
}

ASTnode ASTnode_Expr(Scanner s, Token t) {
    ASTnode n, left, right;
    int nodeType;

    left = primary(s, t);

    if (t->token == T_EOF) {
        return left;
    }

    nodeType = arithop(s, t->token);

    Scanner_Scan(s, t);

    right = ASTnode_Expr(s, t);

    n = ASTnode_New(nodeType, left, NULL, right, 0);

    return n;
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

// Stunting yard algo was good before
// but didnt go so well with register assignment stuff

ASTnode ASTnode_Order(Scanner s, SymTable st, Token t) {

    ASTnode *stack = calloc(MAX_STACK, sizeof(ASTnode));
    enum ASTOP *opStack = calloc(MAX_STACK, sizeof(enum ASTOP));
    enum ASTOP curOp;
    int id;

    //! SCANNER_SCAN MUST BE CALLED FIRST
    //! IN ORDER FOR THIS TO WORK!

    int top = -1, opTop = -1;
    do {
        switch (t->token) {
            case T_INTLIT:
                stack[++top] = ASTnode_NewLeaf(A_INTLIT, t->intvalue);
                break;
            case T_IDENT:
                if ((id = SymTable_GlobFind(st, s)) == -1) {
                    fprintf(stderr, "Error: Unknown variable %s on line %d\n",
                            s->text, s->line);
                    exit(-1);
                }
                // Need to fix this
                // this needs to somehow be left - fixed

                stack[++top] = ASTnode_NewLeaf(A_IDENT, id);
                break;
            // TODO: IMPLEMENT PERTHENESIS
            default:
                curOp = arithop(s, t->token);
                while (opTop != -1 &&
                       precedence(opStack[opTop]) >= precedence(curOp)) {
                    enum ASTOP op = opStack[opTop--];

                    //! This order causes the thingo
                    //! to exhaust all registers
                    //! Right should have another operation - fixed

                    ASTnode right = stack[top--];
                    ASTnode left = stack[top--];

                    stack[++top] = ASTnode_New(op, left, NULL, right, 0);
                }
                opStack[++opTop] = curOp;
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

        stack[++top] = ASTnode_New(op, left, NULL, right, 0);
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