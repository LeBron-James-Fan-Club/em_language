#include "stmt.h"

#include <stdbool.h>

#include "misc.h"

void match(Scanner s, Token t, enum OPCODES op, char *tok);
void semi(Scanner s, Token t);
void ident(Scanner s, Token t);
void lbrace(Scanner s, Token t);
void rbrace(Scanner s, Token t);
void lparen(Scanner s, Token t);
void rparen(Scanner s, Token t);

static ASTnode poke_statement(Scanner s, SymTable st, Token tok);

static ASTnode print_statement(Scanner s, SymTable st, Token tok);
// static ASTnode assignment_statement(Scanner s, SymTable st, Token tok);
static ASTnode input_statement(Scanner s, SymTable st, Token tok);
static ASTnode if_statement(Compiler c, Scanner s, SymTable st, Token tok,
                            Context ctx);

static ASTnode label_statement(Scanner s, SymTable st, Token tok);
static ASTnode goto_statement(Scanner s, SymTable st, Token tok);
static ASTnode while_statement(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx);

static ASTnode for_statement(Compiler c, Scanner s, SymTable st, Token tok,
                             Context ctx);
static ASTnode return_statement(Scanner s, SymTable st, Token tok, Context ctx);

static ASTnode single_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx);

ASTnode Compound_Statement(Compiler c, Scanner s, SymTable st, Token tok,
                           Context ctx) {
    // Might combine assignment and declare
    // change syntax to lol : i32 = 2

    ASTnode left = NULL;
    ASTnode tree = NULL;

    lbrace(s, tok);

    while (true) {
        // TODO:  Compiler directive will be checked here
        tree = single_statement(c, s, st, tok, ctx);

        if (tree != NULL &&
            (tree->op == A_INPUT || tree->op == A_RETURN ||
             tree->op == A_ASSIGN || tree->op == A_FUNCCALL ||
             tree->op == A_LABEL || tree->op == A_GOTO || tree->op == A_POKE)) {
            semi(s, tok);
        }

        if (tree) {
            left = (left == NULL)
                       ? tree
                       : ASTnode_New(A_GLUE, P_NONE, left, NULL, tree, 0);
        }

        if (tok->token == T_RBRACE) {
            rbrace(s, tok);
            return left;
        }
    }
}

static ASTnode single_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx) {
    int type;
    switch (tok->token) {
        case T_PRINT:
            return print_statement(s, st, tok);
        case T_CHAR:
        case T_INT:
            type = parse_type(s, tok);
            ident(s, tok);
            var_declare(c, s, st, tok, type, C_LOCAL);
            return NULL;
        case T_POKE:
            return poke_statement(s, st, tok);
        case T_INPUT:
            return input_statement(s, st, tok);
        case T_IF:
            return if_statement(c, s, st, tok, ctx);
        case T_LABEL:
            return label_statement(s, st, tok);
        case T_GOTO:
            return goto_statement(s, st, tok);
        case T_WHILE:
            return while_statement(c, s, st, tok, ctx);
        case T_FOR:
            // Basically a while loop wrapper
            return for_statement(c, s, st, tok, ctx);
        case T_RETURN:
            return return_statement(s, st, tok, ctx);
        default:
            debug("tok is %d", tok->token);
            return ASTnode_Order(s, st, tok);
    }
}

static ASTnode poke_statement(Scanner s, SymTable st, Token tok) {
    ASTnode param1, param2;
    match(s, tok, T_POKE, "poke");
    lparen(s, tok);
    param1 = ASTnode_Order(s, st, tok);
    match(s, tok, T_COMMA, ",");
    param2 = ASTnode_Order(s, st, tok);
    rparen(s, tok);
    return ASTnode_New(A_POKE, P_NONE, param2, NULL, param1, 0);
}

static ASTnode print_statement(Scanner s, SymTable st, Token tok) {
    ASTnode t;

    match(s, tok, T_PRINT, "print");

    // Might break if i add more types not compatable

    // loops til no more commas - we can do hello world now
    ASTnode parent = NULL;
    bool firstGone = false;
    lparen(s, tok);
    do {
        if (parent) {
            Scanner_Scan(s, tok);
        }
        t = ASTnode_Order(s, st, tok);

        int rightType = t->type;
        t->rvalue = true;

        // P_NONE should never occur thereotically for now
        if (rightType == P_NONE || rightType == P_VOID) {
            fprintf(stderr, "Error: Type mismatch on line %d\n", s->line);
            exit(-1);
        }

        t = ASTnode_NewUnary(A_PRINT, rightType, t, 0);

        if (firstGone) {
            // memory leak occurs here
            parent = ASTnode_New(A_GLUE, P_NONE, parent, NULL, t, 0);
        } else {
            parent = t;
            firstGone = true;
        }
    } while (tok->token == T_COMMA);
    rparen(s, tok);

    semi(s, tok);

    return parent;
}

static ASTnode input_statement(Scanner s, SymTable st, Token tok) {
    int id;
    match(s, tok, T_INPUT, "input");
    lparen(s, tok);

    ident(s, tok);

    if ((id = SymTable_Find(st, s, S_VAR)) == -1) {
        fprintf(stderr, "Error: Undefined variable %s on line %d\n", s->text,
                s->line);
        exit(-1);
    }

    ASTnode left = ASTnode_NewLeaf(A_INPUT, P_INT, 0);
    ASTnode right = ASTnode_NewLeaf(A_IDENT, st->Gsym[id].type, id);
    right->rvalue = 0;  // We do not need to load the value of the variable

    ASTnode tree = modify_type(right, left->type, A_NONE);

    tree = ASTnode_New(A_ASSIGN, P_NONE, left, NULL, tree, 0);
    rparen(s, tok);

    return tree;
}

static ASTnode if_statement(Compiler c, Scanner s, SymTable st, Token tok,
                            Context ctx) {
    ASTnode condAST, trueAST, falseAST = NULL;

    match(s, tok, T_IF, "if");
    lparen(s, tok);

    condAST = ASTnode_Order(s, st, tok);

    // Might remove this guard later
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        condAST = ASTnode_NewUnary(A_TOBOOL, condAST->type, condAST, 0);
    }

    rparen(s, tok);

    trueAST = Compound_Statement(c, s, st, tok, ctx);

    if (tok->token == T_ELSE) {
        Scanner_Scan(s, tok);
        falseAST = Compound_Statement(c, s, st, tok, ctx);
    }

    return ASTnode_New(A_IF, P_NONE, condAST, trueAST, falseAST, 0);
}

static ASTnode while_statement(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {
    ASTnode condAST, bodyAST;

    match(s, tok, T_WHILE, "while");
    lparen(s, tok);

    condAST = ASTnode_Order(s, st, tok);
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }

    rparen(s, tok);
    bodyAST = Compound_Statement(c, s, st, tok, ctx);

    return ASTnode_New(A_WHILE, P_NONE, condAST, NULL, bodyAST, 0);
}

static ASTnode for_statement(Compiler c, Scanner s, SymTable st, Token tok,
                             Context ctx) {
    ASTnode condAST, bodyAST;
    ASTnode preopAST, postopAST;
    ASTnode t;

    match(s, tok, T_FOR, "for");
    lparen(s, tok);

    preopAST = single_statement(c, s, st, tok, ctx);
    semi(s, tok);

    condAST = ASTnode_Order(s, st, tok);
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }
    semi(s, tok);

    postopAST = single_statement(c, s, st, tok, ctx);
    rparen(s, tok);

    bodyAST = Compound_Statement(c, s, st, tok, ctx);

    t = ASTnode_New(A_GLUE, P_NONE, bodyAST, NULL, postopAST, 0);
    t = ASTnode_New(A_WHILE, P_NONE, condAST, NULL, t, 0);
    return ASTnode_New(A_GLUE, P_NONE, preopAST, NULL, t, 0);
}

static ASTnode label_statement(Scanner s, SymTable st, Token tok) {
    match(s, tok, T_LABEL, "label");
    ident(s, tok);

    int id = SymTable_Add(st, s, P_NONE, S_LABEL, C_GLOBAL, 0, false);

    ASTnode t = ASTnode_NewLeaf(A_LABEL, P_NONE, id);

    return t;
}

static ASTnode goto_statement(Scanner s, SymTable st, Token tok) {
    int id;

    match(s, tok, T_GOTO, "goto");
    // ! Might be buggy?
    ident(s, tok);
    if ((id = SymTable_Find(st, s, S_LABEL)) == -1) {
        fprintf(stderr, "Error: Undefined variable %s on line %d\n", s->text,
                s->line);
        exit(-1);
    }

    ASTnode t = ASTnode_NewLeaf(A_GOTO, P_NONE, id);

    return t;
}

static ASTnode return_statement(Scanner s, SymTable st, Token tok,
                                Context ctx) {
    ASTnode t;
    int funcId = Context_GetFunctionId(ctx);
    if (st->Gsym[funcId].name == NULL) {
        fprintf(stderr, "Error: Return outside of function on line %d\n",
                s->line);
        exit(-1);
    }

    if (st->Gsym[funcId].type == P_VOID) {
        fprintf(stderr, "Error: Return in void function on line %d\n", s->line);
        exit(-1);
    }

    match(s, tok, T_RETURN, "return");

    t = ASTnode_Order(s, st, tok);
    t->rvalue = 1;

    t = modify_type(t, st->Gsym[funcId].type, A_NONE);
    if (t == NULL) {
        fprintf(stderr, "Error: Type mismatch in return on line %d\n", s->line);
        exit(-1);
    }
    return ASTnode_NewUnary(A_RETURN, P_NONE, t, 0);
}
