#include <stdbool.h>

#include "ast.h"
#include "tokens.h"
#include "sym.h"

static void match(Scanner s, Token t, enum OPCODES op, char *tok);

static void semi(Scanner s, Token t);
static void ident(Scanner s, Token t);
static void lbrace(Scanner s, Token t);
static void rbrace(Scanner s, Token t);
static void lparen(Scanner s, Token t);
static void rparen(Scanner s, Token t);

static ASTnode print_statement(Scanner s, SymTable st, Token tok);
static void var_declare(Scanner s, SymTable st, Token tok);
static ASTnode assignment_statement(Scanner s, SymTable st,
                                    Token tok);
static ASTnode input_statement(Scanner s, SymTable st, Token tok);
static ASTnode if_statement(Scanner s, SymTable st, Token tok);

static ASTnode label_statement(Scanner s, SymTable st, Token tok);
static ASTnode goto_statement(Scanner s, SymTable st, Token tok);
static ASTnode while_statement(Scanner s, SymTable st, Token tok); 

ASTnode Compound_Statement(Scanner s, SymTable st, Token tok) {
    // Might combine assignment and declare
    // change syntax to lol : i32 = 2

    ASTnode left = NULL;
    ASTnode tree = NULL;
    printf("Token %d\n", tok->token);

    lbrace(s, tok);

    while (true) {
        switch (tok->token) {
            case T_PRINT:
                tree = print_statement(s, st, tok);
                break;
            case T_INT:
                var_declare(s, st, tok);
                tree = NULL;
                break;
            case T_IDENT:
                tree = assignment_statement(s, st, tok);
                break;
            case T_INPUT:
                tree = input_statement(s, st, tok);
                break;
            case T_IF:
                tree = if_statement(s, st, tok);
                break;
            case T_LABEL:
                tree = label_statement(s, st, tok);
                break;
            case T_GOTO:
                tree = goto_statement(s, st, tok);
                break;
            case T_WHILE:
                tree = while_statement(s, st, tok);
                break;
            case T_RBRACE:
                rbrace(s, tok);
                return left;
            default:
                fprintf(stderr,
                        "Error: Unknown statement on line %d token %d\n",
                        s->line, tok->token);
                exit(-1);
        }
        if (tree) {
            printf("tree %d\n", tree->op);
            if (left == NULL) {
                printf("Set left = true\n");
                left = tree;
            } else {
                printf("Creating GLUE\n");
                left = ASTnode_New(A_GLUE, left, NULL, tree, 0);
            }
            //left = (left == NULL) ? tree
             //                     : ASTnode_New(A_GLUE, left, NULL, tree, 0);
        }
    }
}

static ASTnode print_statement(Scanner s, SymTable st, Token tok) {
    ASTnode t;

    match(s, tok, T_PRINT, "print");

    t = ASTnode_Order(s, st, tok);
    t = ASTnode_NewUnary(A_PRINT, t, 0);

    semi(s, tok);
    return t;
}

static void var_declare(Scanner s, SymTable st, Token tok) {
    match(s, tok, T_INT, "int");

    ident(s, tok);

    SymTable_GlobAdd(st, s);
    // * .comm written is supposed to be here but it will be
    // * deferred

    semi(s, tok);
}

static ASTnode assignment_statement(Scanner s, SymTable st,
                                    Token tok) {
    ASTnode left, right, tree;
    int id;

    // Looks unnecessary but it consumes token
    ident(s, tok);

    if ((id = SymTable_GlobFind(st, s)) == -1) {
        fprintf(stderr, "Error: Undefined variable %s on line %d\n", s->text,
                s->line);
        exit(-1);
    }
    right = ASTnode_NewLeaf(A_LVIDENT, id);
    match(s, tok, T_ASSIGN, "=");

    left = ASTnode_Order(s, st, tok);

    tree = ASTnode_New(A_ASSIGN, left, NULL, right, 0);

    semi(s, tok);
    return tree;
}

static ASTnode input_statement(Scanner s, SymTable st, Token tok) {
    int id;
    match(s, tok, T_INPUT, "input");

    ident(s, tok);

    if ((id = SymTable_GlobFind(st, s)) == -1) {
        fprintf(stderr, "Error: Undefined variable %s on line %d\n", s->text,
                s->line);
        exit(-1);
    }

    ASTnode left = ASTnode_NewLeaf(A_INPUT, 0);
    ASTnode right = ASTnode_NewLeaf(A_LVIDENT, id);
    ASTnode tree = ASTnode_New(A_ASSIGN, left, NULL, right, 0);

    semi(s, tok);
    return tree;
}

static ASTnode if_statement(Scanner s, SymTable st, Token tok) {
    ASTnode condAST, trueAST, falseAST = NULL;
    match(s, tok, T_IF, "if");
    lparen(s, tok);

    printf("Making condition AST\n");

    condAST = ASTnode_Order(s, st, tok);
    printf("Finished making it\n");
    // Might remove this guard later
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }

    rparen(s, tok);

    trueAST = Compound_Statement(s, st, tok);
    if (tok->token == T_ELSE) {
        Scanner_Scan(s, tok);
        falseAST = Compound_Statement(s, st, tok);
    }

    return ASTnode_New(A_IF, condAST, trueAST, falseAST, 0);
}

static ASTnode while_statement(Scanner s, SymTable st, Token tok) {
    ASTnode condAST, bodyAST;
    match(s, tok, T_WHILE, "while");
    lparen(s, tok);

    condAST = ASTnode_Order(s, st, tok);   
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }

    rparen(s, tok);
    bodyAST = Compound_Statement(s, st, tok);

    return ASTnode_New(A_WHILE, condAST, NULL, bodyAST, 0);
}

static ASTnode label_statement(Scanner s, SymTable st, Token tok) {
    printf("WHY IS THIS HAPPENING 4 TIMES\n");
    match(s, tok, T_LABEL, "label");
    ident(s, tok);

    int id = SymTable_LabelAdd(st, s);

    ASTnode t = ASTnode_NewLeaf(A_LABEL, id);
    semi(s, tok);

    return t;
}

static ASTnode goto_statement(Scanner s, SymTable st, Token tok) {
    int id;

    match(s, tok, T_GOTO, "goto");
    // ! Might be buggy?
    ident(s, tok);
    if ((id = SymTable_LabelFind(st, s)) == -1) {
        fprintf(stderr, "Error: Undefined variable %s on line %d\n", s->text,
                s->line);
        exit(-1);
    }

    ASTnode t = ASTnode_NewLeaf(A_GOTO, id);
    semi(s, tok);

    return t;
}

static void match(Scanner s, Token t, enum OPCODES op, char *tok) {
    if (t->token == op) {
        Scanner_Scan(s, t);
    } else {
        fprintf(stderr, "Error: %s expected on line %d\n", tok, s->line);
        fprintf(stderr, "got instead %d\n", t->token);
        exit(-1);
    }
}

static void semi(Scanner s, Token t) { match(s, t, T_SEMI, ";"); }

static void ident(Scanner s, Token t) { match(s, t, T_IDENT, "identifier"); }

static void lbrace(Scanner s, Token t) { match(s, t, T_LBRACE, "{"); }

static void rbrace(Scanner s, Token t) { match(s, t, T_RBRACE, "}"); }

static void lparen(Scanner s, Token t) { match(s, t, T_LPAREN, "("); }

static void rparen(Scanner s, Token t) { match(s, t, T_RPAREN, ")"); }
