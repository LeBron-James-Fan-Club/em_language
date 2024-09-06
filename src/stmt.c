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

static ASTnode poke_statement(Scanner s, SymTable st, Token tok, Context ctx);

static ASTnode print_statement(Scanner s, SymTable st, Token tok, Context ctx);
static ASTnode input_statement(Scanner s, SymTable st, Token tok, Context ctx);
static ASTnode if_statement(Compiler c, Scanner s, SymTable st, Token tok,
                            Context ctx);

static ASTnode label_statement(Scanner s, SymTable st, Token tok);
static ASTnode goto_statement(Scanner s, SymTable st, Token tok);
static ASTnode while_statement(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx);

static ASTnode for_statement(Compiler c, Scanner s, SymTable st, Token tok,
                             Context ctx);
static ASTnode return_statement(Scanner s, SymTable st, Token tok, Context ctx);

static ASTnode break_statement(Scanner s, SymTable st, Token tok, Context ctx);
static ASTnode continue_statement(Scanner s, SymTable st, Token tok,
                                  Context ctx);
static ASTnode switch_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx);

static ASTnode single_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx);

ASTnode Compound_Statement(Compiler c, Scanner s, SymTable st, Token tok,
                           Context ctx) {
    // Might combine assignment and declare
    // change syntax to lol : i32 = 2

    ASTnode left = NULL;
    ASTnode tree = NULL;

    lbrace(s, tok);

    // Edge case: its empty
    if (tok->token == T_RBRACE) {
        rbrace(s, tok);
        return NULL;
    }

    while (true) {
        // TODO:  Compiler directive will be checked here
        tree = single_statement(c, s, st, tok, ctx);

        if (tree != NULL) {
            debug("op %d", tree->op);
        }
        if (tree != NULL && ((tree->op >= A_ASSIGN && tree->op <= A_IDENT) ||
                             tree->op == A_INPUT || tree->op == A_RETURN ||
                             tree->op == A_FUNCCALL || tree->op == A_LABEL ||
                             tree->op == A_GOTO || tree->op == A_POKE ||
                             tree->op == A_BREAK || tree->op == A_CONTINUE)) {
            debug("consume semi");
            semi(s, tok);
        }

        if (tree) {
            left = (left == NULL)
                       ? tree
                       : ASTnode_New(A_GLUE, P_NONE, left, NULL, tree, NULL, 0);
        }

        if (tok->token == T_RBRACE) {
            rbrace(s, tok);
            return left;
        }
    }
}

static ASTnode single_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx) {
    enum ASTPRIM type;
    enum STORECLASS class = C_LOCAL;

    SymTableEntry cType;

    switch (tok->token) {
        case T_PRINT:
            return print_statement(s, st, tok, ctx);
        case T_IDENT:
            if (SymTable_FindTypeDef(st, s) == NULL) {
                debug("Went through here");
                return ASTnode_Order(s, st, tok, ctx);
            }
            debug("pass through");
        case T_CHAR:
        case T_INT:
        case T_STRUCT:
        case T_UNION:
        case T_ENUM:
        case T_TYPEDEF:
            type = parse_type(s, st, tok, &cType, &class);
            ident(s, tok);
            debug("class %d", class);
            var_declare(s, st, tok, type, cType, class, false);
            // ! check if this fucks up anything
            debug("before semi consume");
            semi(s, tok);
            debug("out of here");
            return NULL;
        case T_POKE:
            return poke_statement(s, st, tok, ctx);
        case T_INPUT:
            return input_statement(s, st, tok, ctx);
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
        case T_BREAK:
            return break_statement(s, st, tok, ctx);
        case T_CONTINUE:
            return continue_statement(s, st, tok, ctx);
        case T_SWITCH:
            return switch_statement(c, s, st, tok, ctx);
        default:
            debug("in here 2, token %d", tok->token);
            return ASTnode_Order(s, st, tok, ctx);
    }
}

static ASTnode poke_statement(Scanner s, SymTable st, Token tok, Context ctx) {
    ASTnode param1, param2;
    match(s, tok, T_POKE, "poke");
    lparen(s, tok);
    param1 = ASTnode_Order(s, st, tok, ctx);
    match(s, tok, T_COMMA, ",");
    param2 = ASTnode_Order(s, st, tok, ctx);
    rparen(s, tok);
    return ASTnode_New(A_POKE, P_NONE, param2, NULL, param1, NULL, 0);
}

static ASTnode print_statement(Scanner s, SymTable st, Token tok, Context ctx) {
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
        t = ASTnode_Order(s, st, tok, ctx);

        int rightType = t->type;
        t->rvalue = true;

        // P_NONE should never occur thereotically for now
        if (rightType == P_NONE || rightType == P_VOID) {
            fprintf(stderr, "Error: Type mismatch on line %d\n", s->line);
            exit(-1);
        }

        t = ASTnode_NewUnary(A_PRINT, rightType, t, NULL, 0);

        if (firstGone) {
            // memory leak occurs here
            parent = ASTnode_New(A_GLUE, P_NONE, parent, NULL, t, NULL, 0);
        } else {
            parent = t;
            firstGone = true;
        }
    } while (tok->token == T_COMMA);
    rparen(s, tok);

    semi(s, tok);

    return parent;
}

static ASTnode input_statement(Scanner s, SymTable st, Token tok, Context ctx) {
    SymTableEntry var;

    match(s, tok, T_INPUT, "input");
    lparen(s, tok);

    ident(s, tok);

    if ((var = SymTable_FindSymbol(st, s, ctx)) == NULL) {
        lfatala(s, "UndefinedError: Undefined variable %s", s->text);
    }

    ASTnode left = ASTnode_NewLeaf(A_INPUT, P_INT, NULL, 0);
    ASTnode right = ASTnode_NewLeaf(A_IDENT, var->type, var, 0);
    right->rvalue = 0;  // We do not need to load the value of the variable

    ASTnode tree = modify_type(right, left->type, A_NONE);

    tree = ASTnode_New(A_ASSIGN, P_NONE, left, NULL, tree, NULL, 0);
    rparen(s, tok);

    return tree;
}

static ASTnode if_statement(Compiler c, Scanner s, SymTable st, Token tok,
                            Context ctx) {
    ASTnode condAST, trueAST, falseAST = NULL;

    match(s, tok, T_IF, "if");
    lparen(s, tok);

    condAST = ASTnode_Order(s, st, tok, ctx);

    // Might remove this guard later
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        condAST = ASTnode_NewUnary(A_TOBOOL, condAST->type, condAST, NULL, 0);
    }

    rparen(s, tok);

    trueAST = Compound_Statement(c, s, st, tok, ctx);

    if (tok->token == T_ELSE) {
        Scanner_Scan(s, tok);
        falseAST = Compound_Statement(c, s, st, tok, ctx);
    }

    return ASTnode_New(A_IF, P_NONE, condAST, trueAST, falseAST, NULL, 0);
}

static ASTnode while_statement(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {
    ASTnode condAST, bodyAST;

    match(s, tok, T_WHILE, "while");
    lparen(s, tok);

    condAST = ASTnode_Order(s, st, tok, ctx);
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }

    rparen(s, tok);

    Context_IncLoopLevel(ctx);
    bodyAST = Compound_Statement(c, s, st, tok, ctx);
    Context_DecLoopLevel(ctx);

    return ASTnode_New(A_WHILE, P_NONE, condAST, NULL, bodyAST, NULL, 0);
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

    condAST = ASTnode_Order(s, st, tok, ctx);
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }
    semi(s, tok);

    postopAST = single_statement(c, s, st, tok, ctx);
    rparen(s, tok);

    Context_IncLoopLevel(ctx);
    bodyAST = Compound_Statement(c, s, st, tok, ctx);
    Context_DecLoopLevel(ctx);

    t = ASTnode_New(A_GLUE, P_NONE, bodyAST, NULL, postopAST, NULL, 0);
    t = ASTnode_New(A_WHILE, P_NONE, condAST, NULL, t, NULL, 0);
    return ASTnode_New(A_GLUE, P_NONE, preopAST, NULL, t, NULL, 0);
}

static ASTnode label_statement(Scanner s, SymTable st, Token tok) {
    match(s, tok, T_LABEL, "label");
    ident(s, tok);

    SymTableEntry var = SymTable_AddGlob(st, s->text, P_NONE, NULL, S_LABEL,
                                         C_GLOBAL, 0, false);

    ASTnode t = ASTnode_NewLeaf(A_LABEL, P_NONE, var, 0);

    return t;
}

static ASTnode goto_statement(Scanner s, SymTable st, Token tok) {
    SymTableEntry var;

    match(s, tok, T_GOTO, "goto");
    // ! Might be buggy?
    ident(s, tok);
    if ((var = SymTable_FindGlob(st, s)) == NULL) {
        lfatala(s, "UndefinedError: Undefined label %s", s->text);
    }

    ASTnode t = ASTnode_NewLeaf(A_GOTO, P_NONE, var, 0);

    return t;
}

static ASTnode return_statement(Scanner s, SymTable st, Token tok,
                                Context ctx) {
    ASTnode t;
    SymTableEntry func = Context_GetFunctionId(ctx);
    if (func == NULL || func->name == NULL) {
        fprintf(stderr, "Error: Return outside of function on line %d\n",
                s->line);
        exit(-1);
    }

    if (func->type == P_VOID) {
        fprintf(stderr, "Error: Return in void function on line %d\n", s->line);
        exit(-1);
    }

    match(s, tok, T_RETURN, "return");

    t = ASTnode_Order(s, st, tok, ctx);
    t->rvalue = 1;

    debug("func type %d, t type %d", func->type, t->type);

    t = modify_type(t, func->type, A_NONE);
    if (t == NULL) {
        fprintf(stderr, "Error: Type mismatch in return on line %d\n", s->line);
        exit(-1);
    }
    return ASTnode_NewUnary(A_RETURN, P_NONE, t, NULL, 0);
}

static ASTnode break_statement(Scanner s, SymTable st, Token tok, Context ctx) {
    if (Context_GetLoopLevel(ctx) == 0) {
        lfatal(s, "SyntaxError: break outside of loop");
    }
    Scanner_Scan(s, tok);
    return ASTnode_NewLeaf(A_BREAK, P_NONE, NULL, 0);
}

static ASTnode continue_statement(Scanner s, SymTable st, Token tok,
                                  Context ctx) {
    if (Context_GetLoopLevel(ctx) == 0) {
        lfatal(s, "SyntaxError: continue outside of loop");
    }
    Scanner_Scan(s, tok);
    return ASTnode_NewLeaf(A_CONTINUE, P_NONE, NULL, 0);
}

static ASTnode switch_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx) {
    // n = node, ca = case
    ASTnode left, n, ca, caseTree = NULL, caseTail;
    bool inLoop = true, seenDefault = false;
    int caseCount = 0;
    enum ASTOP op;
    int caseValue;

    Scanner_Scan(s, tok);

    lparen(s, tok);
    left = ASTnode_Order(s, st, tok, ctx);
    rparen(s, tok);

    lbrace(s, tok);

    debug("after lbrace");

    if (!inttype(left->type)) {
        fatal("TypeError: switch expression must be an integer");
    }

    n = ASTnode_NewUnary(A_SWITCH, P_NONE, left, NULL, 0);

    Context_IncSwitchLevel(ctx);
    while (inLoop) {
        switch (tok->token) {
            case T_RBRACE:
                if (caseCount == 0) {
                    lfatal(s, "SyntaxError: switch statement with no cases");
                }
                debug("break out");
                inLoop = false;
                break;
            case T_CASE:
            case T_DEFAULT:
                if (seenDefault) {
                    lfatal(
                        s,
                        "SyntaxError: case or default after existing default");
                }


                if (tok->token == T_DEFAULT) {
                    debug("A DEFAULT");

                    op = A_DEFAULT;
                    seenDefault = true;
                    Scanner_Scan(s, tok);
                } else {
                    debug("A CASE");
                    op = A_CASE;
                    Scanner_Scan(s, tok);
                    left = ASTnode_Order(s, st, tok, ctx);

                    if (left->op != A_INTLIT) {
                        lfatal(
                            s,
                            "TypeError: case value must be an integer literal");
                    }
                    caseValue = left->intvalue;
                    debug("case value: %d", caseValue);

                    for (ca = caseTree; ca != NULL; ca = ca->right) {
                        if (ca->intvalue == caseValue) {
                            lfatala(s,
                                    "DuplicateError: duplicate case value %d",
                                    caseValue);
                        }
                    }

                    // Free it we don't need it anymore
                    ASTnode_Free(left);
                }
                match(s, tok, T_COLON, ":");

                left = Compound_Statement(c, s, st, tok, ctx);
                debug("left the case");
                caseCount++;

                if (caseTree == NULL) {
                    caseTree = caseTail =
                        ASTnode_NewUnary(op, P_NONE, left, NULL, caseValue);
                    debug("first create %p", caseTree);
                } else {
                    caseTail->right =
                        ASTnode_NewUnary(op, P_NONE, left, NULL, caseValue);
                    caseTail = caseTail->right;
                    debug("add %p", caseTail);
                }
                break;
            default:
                lfatal(s, "SyntaxError: Expected case or default");
        }
    }
    Context_DecSwitchLevel(ctx);

    n->intvalue = caseCount;
    debug("case count %d", caseCount);
    n->right = caseTree;

    rbrace(s, tok);

    return n;
}