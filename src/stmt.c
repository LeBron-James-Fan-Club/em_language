#include "stmt.h"

#include <stdbool.h>

void match(Scanner s, Token t, enum OPCODES op, char *tok);
void semi(Scanner s, Token t);
void ident(Scanner s, Token t);
void lbrace(Scanner s, Token t);
void rbrace(Scanner s, Token t);
void lparen(Scanner s, Token t);
void rparen(Scanner s, Token t);

static ASTnode print_statement(Scanner s, SymTable st, Token tok);
static ASTnode assignment_statement(Scanner s, SymTable st, Token tok);
static ASTnode input_statement(Scanner s, SymTable st, Token tok);
static ASTnode if_statement(Scanner s, SymTable st, Token tok);

static ASTnode label_statement(Scanner s, SymTable st, Token tok);
static ASTnode goto_statement(Scanner s, SymTable st, Token tok);
static ASTnode while_statement(Scanner s, SymTable st, Token tok);

static ASTnode for_statement(Scanner s, SymTable st, Token tok);

static ASTnode single_statement(Scanner s, SymTable st, Token tok);

ASTnode Compound_Statement(Scanner s, SymTable st, Token tok) {
    // Might combine assignment and declare
    // change syntax to lol : i32 = 2

    ASTnode left = NULL;
    ASTnode tree = NULL;
    printf("Token %d\n", tok->token);

    lbrace(s, tok);

    while (true) {
        tree = single_statement(s, st, tok);

        // Might be weird here:
        if (tree != NULL && (tree->op == A_PRINT || tree->op == A_INPUT ||
                             tree->op == A_ASSIGN || tree->op == A_LABEL ||
                             tree->op == A_GOTO )) {
            semi(s, tok);
        }

        if (tree) {
            left = (left == NULL) ? tree
                                  : ASTnode_New(A_GLUE, left, NULL, tree, 0);
        }

        if (tok->token == T_RBRACE) {
            rbrace(s, tok);
            return left;
        }
    }
}

static ASTnode single_statement(Scanner s, SymTable st, Token tok) {
    switch (tok->token) {
        case T_PRINT:
            return print_statement(s, st, tok);
        case T_INT:
            var_declare(s, st, tok);
            return NULL;
        case T_IDENT:
            return assignment_statement(s, st, tok);
        case T_INPUT:
            return input_statement(s, st, tok);
        case T_IF:
            return if_statement(s, st, tok);
        case T_LABEL:
            return label_statement(s, st, tok);
        case T_GOTO:
            return goto_statement(s, st, tok);
        case T_WHILE:
            return while_statement(s, st, tok);
        case T_FOR:
            // Basically a while loop wrapper
            return for_statement(s, st, tok);
        default:
            fprintf(stderr, "Error: Unknown statement on line %d\n", s->line);
            exit(-1);
    }
}

static ASTnode print_statement(Scanner s, SymTable st, Token tok) {
    ASTnode t;

    match(s, tok, T_PRINT, "print");

    t = ASTnode_Order(s, st, tok);
    t = ASTnode_NewUnary(A_PRINT, t, 0);

    return t;
}

static ASTnode assignment_statement(Scanner s, SymTable st, Token tok) {
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

static ASTnode for_statement(Scanner s, SymTable st, Token tok) {
    ASTnode condAST, bodyAST;
    ASTnode preopAST, postopAST;
    ASTnode t;
    match(s, tok, T_FOR, "for");
    lparen(s, tok);

    preopAST = single_statement(s, st, tok);
    semi(s, tok);

    condAST = ASTnode_Order(s, st, tok);
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }
    semi(s, tok);

    postopAST = single_statement(s, st, tok);
    rparen(s, tok);

    bodyAST = Compound_Statement(s, st, tok);

    t = ASTnode_New(A_GLUE, bodyAST, NULL, postopAST, 0);
    t = ASTnode_New(A_WHILE, condAST, NULL, t, 0);
    return ASTnode_New(A_GLUE, preopAST, NULL, t, 0);
}

static ASTnode label_statement(Scanner s, SymTable st, Token tok) {
    printf("WHY IS THIS HAPPENING 4 TIMES\n");
    match(s, tok, T_LABEL, "label");
    ident(s, tok);

    int id = SymTable_LabelAdd(st, s);

    ASTnode t = ASTnode_NewLeaf(A_LABEL, id);

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

    return t;
}

void match(Scanner s, Token t, enum OPCODES op, char *tok) {
    if (t->token == op) {
        Scanner_Scan(s, t);
    } else {
        fprintf(stderr, "Error: %s expected on line %d\n", tok, s->line);
        fprintf(stderr, "got instead %d\n", t->token);
        exit(-1);
    }
}

void semi(Scanner s, Token t) { match(s, t, T_SEMI, ";"); }

void ident(Scanner s, Token t) { match(s, t, T_IDENT, "identifier"); }

void lbrace(Scanner s, Token t) { match(s, t, T_LBRACE, "{"); }

void rbrace(Scanner s, Token t) { match(s, t, T_RBRACE, "}"); }

void lparen(Scanner s, Token t) { match(s, t, T_LPAREN, "("); }

void rparen(Scanner s, Token t) { match(s, t, T_RPAREN, ")"); }
