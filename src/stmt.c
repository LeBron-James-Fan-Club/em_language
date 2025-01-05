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

static ASTnode poke_statement(Compiler c, Scanner s, SymTable st, Token tok,
                              Context ctx);

static ASTnode exit_statement(Compiler c, Scanner s, SymTable st, Token tok,
                              Context ctx);

static ASTnode print_statement(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx);
static ASTnode input_statement(Scanner s, SymTable st, Token tok, Context ctx);
static ASTnode if_statement(Compiler c, Scanner s, SymTable st, Token tok,
                            Context ctx);

static ASTnode label_statement(Scanner s, SymTable st, Token tok);
static ASTnode goto_statement(Scanner s, SymTable st, Token tok);
static ASTnode while_statement(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx);

static ASTnode for_statement(Compiler c, Scanner s, SymTable st, Token tok,
                             Context ctx);
static ASTnode return_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx);

static ASTnode break_statement(Scanner s, Token tok, Context ctx);
static ASTnode continue_statement(Scanner s, Token tok, Context ctx);
static ASTnode switch_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx);

static ASTnode single_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx);

ASTnode Compound_Statement(Compiler c, Scanner s, SymTable st, Token tok,
                           Context ctx, bool inSwitch) {
    // * Might combine assignment and declare
    // * change syntax to lol : i32 = 2 - nah fuck that

    ASTnode left = NULL;
    ASTnode tree = NULL;

    while (true) {
        // TODO:  Compiler directive will be checked here
        tree = single_statement(c, s, st, tok, ctx);

        if (tree != NULL) {
            debug("op %d", tree->op);
        }

        if (tree) {
            left = (left == NULL) ? tree
                                  : ASTnode_New(A_GLUE, P_NONE, left, NULL,
                                                tree, NULL, NULL, 0);
        }

        if (tok->token == T_RBRACE) {
            // rbrace(s, tok);
            debug("GTFO");
            return left;
        }

        if (inSwitch && (tok->token == T_CASE || tok->token == T_DEFAULT)) {
            return left;
        }
    }
}

static ASTnode single_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx) {
    ASTnode stmt;

    SymTableEntry cType;

    switch (tok->token) {
        case T_PRINT:
            return print_statement(c, s, st, tok, ctx);
        case T_IDENT:
            if (SymTable_FindTypeDef(st, s) == NULL) {
                stmt = ASTnode_Order(c, s, st, tok, ctx, 0);
                printf("stmt %s\n", s->comment);
                stmt->comment = strdup(s->comment);
                semi(s, tok);
                return stmt;
            }
        case T_CHAR:
        case T_INT:
        case T_STRUCT:
        case T_UNION:
        case T_ENUM:
        case T_TYPEDEF:
            declare_list(c, s, st, tok, ctx, &cType, C_LOCAL, T_SEMI, T_EOF,
                         &stmt);
            if (stmt != NULL) {
                stmt->comment = strdup(s->comment);
            }
            semi(s, tok);
            return stmt;
        case T_POKE:
            return poke_statement(c, s, st, tok, ctx);
        case T_EXIT:
            return exit_statement(c, s, st, tok, ctx);
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
            return return_statement(c, s, st, tok, ctx);
        case T_BREAK:
            return break_statement(s, tok, ctx);
        case T_CONTINUE:
            return continue_statement(s, tok, ctx);
        case T_SWITCH:
            return switch_statement(c, s, st, tok, ctx);
        case T_LBRACE:
            lbrace(s, tok);
            // Edgecase: Empty
            if (tok->token == T_RBRACE) {
                rbrace(s, tok);
                return NULL;
            }
            stmt = Compound_Statement(c, s, st, tok, ctx, false);
            rbrace(s, tok);
            return stmt;
        default:
            stmt = ASTnode_Order(c, s, st, tok, ctx, 0);
            printf("stmt %s\n", s->comment);
            stmt->comment = strdup(s->comment);
            semi(s, tok);
            return stmt;
    }
}

static ASTnode poke_statement(Compiler c, Scanner s, SymTable st, Token tok,
                              Context ctx) {
    ASTnode param1, param2;
    match(s, tok, T_POKE, "poke");
    lparen(s, tok);
    param1 = ASTnode_Order(c, s, st, tok, ctx, 0);
    comma(s, tok);
    param2 = ASTnode_Order(c, s, st, tok, ctx, 0);
    rparen(s, tok);
    semi(s, tok);
    return ASTnode_New(A_POKE, P_NONE, param2, NULL, param1, NULL, NULL, 0);
}

static ASTnode exit_statement(Compiler c, Scanner s, SymTable st, Token tok, Context ctx) {
    ASTnode param;
    match(s, tok, T_EXIT, "exit");
    lparen(s, tok);
    param = ASTnode_Order(c, s, st, tok, ctx, 0);
    rparen(s, tok);
    ASTnode stmt = ASTnode_New(A_EXIT, P_NONE, param, NULL, NULL, NULL, NULL, 0);
    stmt->comment = strdup(s->comment);
    semi(s, tok);
    return stmt;
}

static ASTnode print_statement(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {
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
        t = ASTnode_Order(c, s, st, tok, ctx, 0);

        int rightType = t->type;
        t->rvalue = true;

        // P_NONE should never occur thereotically for now
        if (rightType == P_NONE || rightType == P_VOID) {
            fprintf(stderr, "Error: Type mismatch on line %d\n", s->line);
            exit(-1);
        }

        t = ASTnode_NewUnary(A_PRINT, rightType, t, NULL, NULL, 0);

        if (firstGone) {
            // memory leak occurs here
            parent =
                ASTnode_New(A_GLUE, P_NONE, parent, NULL, t, NULL, NULL, 0);
        } else {
            parent = t;
            firstGone = true;
        }
    } while (tok->token == T_COMMA);
    debug("test??? 1 %s", tok->tokstr);
    rparen(s, tok);
    debug("test??? 2 %s", tok->tokstr);

    semi(s, tok);

    return parent;
}

static ASTnode input_statement(Scanner s, SymTable st, Token tok, Context ctx) {
    SymTableEntry var;

    // input(variable, type);

    match(s, tok, T_INPUT, "input");
    lparen(s, tok);

    ident(s, tok);

    if ((var = SymTable_FindSymbol(st, s, ctx)) == NULL) {
        lfatala(s, "UndefinedError: Undefined variable %s", s->text);
    }

    comma(s, tok);

    ASTnode left;
    bool isStr = false;

    // ! BUG HERE int and char input doesnt work
    switch (tok->token) {
        case T_CHAR:
            Scanner_Scan(s, tok);
            if (tok->token == T_STAR) {
                debug("FOUND STRING");
                isStr = true;
                left =
                    ASTnode_NewLeaf(A_INPUT, pointer_to(P_CHAR), NULL, var, 0);
            } else {
                Scanner_RejectToken(s, tok);
                debug("bussin with the char");
                left = ASTnode_NewLeaf(A_INPUT, P_CHAR, NULL, NULL, 0);
            }
            break;
        case T_INT:
            debug("we got the int bro");
            left = ASTnode_NewLeaf(A_INPUT, P_INT, NULL, NULL, 0);
            break;
        default:
            lfatal(s, "TypeError: Unknown type given for input");
    }
    Scanner_Scan(s, tok);

    if (isStr) {
        rparen(s, tok);
        semi(s, tok);
        return left;
    }

    ASTnode right = ASTnode_NewLeaf(A_IDENT, var->type, var->ctype, var, 0);
    right->rvalue = 0;  // We do not need to load the value of the variable

    ASTnode tree = modify_type(right, left->type, left->ctype, A_NONE);

    tree = ASTnode_New(A_ASSIGN, P_NONE, left, NULL, tree, NULL, NULL, 0);
    rparen(s, tok);

    semi(s, tok);

    return tree;
}

static ASTnode if_statement(Compiler c, Scanner s, SymTable st, Token tok,
                            Context ctx) {
    ASTnode condAST, trueAST, falseAST = NULL;

    match(s, tok, T_IF, "if");

    bool customLabel = false;
    char *label = NULL;

    if (tok->token == T_LBRACKET) {
        Scanner_Scan(s, tok);
        customLabel = true;
        if (tok->token != T_IDENT)
            lfatal(s, "SyntaxError: Expected identifier in custom label");
        label = strdup(s->text);
        Scanner_Scan(s, tok);
        rbracket(s, tok);
    }

    lparen(s, tok);

    condAST = ASTnode_Order(c, s, st, tok, ctx, 0);

    // Might remove this guard later
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        condAST =
            ASTnode_NewUnary(A_TOBOOL, condAST->type, condAST, NULL, NULL, 0);
    }

    Scanner_EndComment(s);
    condAST->comment = strdup(s->comment);
    rparen(s, tok);


    trueAST = single_statement(c, s, st, tok, ctx);

    if (tok->token == T_ELSE) {
        Scanner_Scan(s, tok);
        falseAST = single_statement(c, s, st, tok, ctx);
    }

    ASTnode n =
        ASTnode_New(A_IF, P_NONE, condAST, trueAST, falseAST, NULL, NULL, 0);
    if (customLabel) {
        n->label.customLabel = label;
        n->label.hasCustomLabel = customLabel;
    }

    return n;
}

static ASTnode while_statement(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {
    ASTnode condAST, bodyAST;

    match(s, tok, T_WHILE, "while");

    bool customLabel = false;
    char *label = NULL;

    if (tok->token == T_LBRACKET) {
        Scanner_Scan(s, tok);
        customLabel = true;
        if (tok->token != T_IDENT)
            lfatal(s, "SyntaxError: Expected identifier in custom label");
        label = strdup(s->text);
        Scanner_Scan(s, tok);
        rbracket(s, tok);
    }

    lparen(s, tok);

    condAST = ASTnode_Order(c, s, st, tok, ctx, 0);
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }

    rparen(s, tok);

    Context_IncLoopLevel(ctx);
    bodyAST = single_statement(c, s, st, tok, ctx);
    Context_DecLoopLevel(ctx);

    ASTnode n =
        ASTnode_New(A_WHILE, P_NONE, condAST, NULL, bodyAST, NULL, NULL, 0);
    if (customLabel) {
        n->label.customLabel = label;
        n->label.hasCustomLabel = customLabel;
    }

    return n;
}

static ASTnode for_statement(Compiler c, Scanner s, SymTable st, Token tok,
                             Context ctx) {
    ASTnode condAST, bodyAST;
    ASTnode preopAST, postopAST;
    ASTnode t;

    match(s, tok, T_FOR, "for");

    bool customLabel = false;
    char *label = NULL;

    if (tok->token == T_LBRACKET) {
        Scanner_Scan(s, tok);
        customLabel = true;
        if (tok->token != T_IDENT)
            lfatal(s, "SyntaxError: Expected identifier in custom label");
        label = strdup(s->text);
        Scanner_Scan(s, tok);
        rbracket(s, tok);
    }

    lparen(s, tok);

    preopAST = expression_list(c, s, st, tok, ctx, T_SEMI);
    preopAST->comment = strdup(s->comment);
    semi(s, tok);

    debug("WE ARE COND :L");

    condAST = ASTnode_Order(c, s, st, tok, ctx, 0);
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }
    debug("EATING THE SEMMI :L");
    condAST->comment = strdup(s->comment);
    semi(s, tok);

    debug("WE ARE POST :L");

    postopAST = expression_list(c, s, st, tok, ctx, T_RPAREN);
    rparen(s, tok);

    Context_IncLoopLevel(ctx);
    bodyAST = single_statement(c, s, st, tok, ctx);
    Context_DecLoopLevel(ctx);

    t = ASTnode_New(A_GLUE, P_NONE, bodyAST, NULL, postopAST, NULL, NULL, 0);
    t = ASTnode_New(A_WHILE, P_NONE, condAST, NULL, t, NULL, NULL, 0);
    if (customLabel) {
        t->label.customLabel = label;
        t->label.hasCustomLabel = customLabel;
    }

    return ASTnode_New(A_GLUE, P_NONE, preopAST, NULL, t, NULL, NULL, 0);
}

static ASTnode label_statement(Scanner s, SymTable st, Token tok) {
    match(s, tok, T_LABEL, "label");
    ident(s, tok);

    SymTableEntry var = SymTable_AddGlob(st, s->text, P_NONE, NULL, S_LABEL,
                                         C_GLOBAL, NULL, 0, false);

    ASTnode t = ASTnode_NewLeaf(A_LABEL, P_NONE, NULL, var, 0);
    semi(s, tok);

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

    ASTnode t = ASTnode_NewLeaf(A_GOTO, P_NONE, NULL, var, 0);
    semi(s, tok);

    return t;
}

static ASTnode return_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx) {
    ASTnode t = NULL;
    // no need to read as compound statements are only called inside functions
    // :)

    match(s, tok, T_RETURN, "return");

    if (tok->token != T_SEMI) {
        t = ASTnode_Order(c, s, st, tok, ctx, 0);
        t->rvalue = 1;

        t = modify_type(t, ctx->functionId->type, ctx->functionId->ctype,
                        A_NONE);
        if (t == NULL) {
            lfatal(s, "TypeError: Type mismatch in return");
        }
    } else {
        if (ctx->functionId->type != P_VOID) {
            lfatal(
                s,
                "TypeError: Value must be returned from a non-void function");
        }
    }

    t->comment = strdup(s->comment);

    semi(s, tok);

    return ASTnode_NewUnary(A_RETURN, P_NONE, t, NULL, NULL, 0);
}

static ASTnode break_statement(Scanner s, Token tok, Context ctx) {
    if (Context_GetLoopLevel(ctx) == 0) {
        lfatal(s, "SyntaxError: break outside of loop");
    }
    Scanner_Scan(s, tok);
    semi(s, tok);
    return ASTnode_NewLeaf(A_BREAK, P_NONE, NULL, NULL, 0);
}

static ASTnode continue_statement(Scanner s, Token tok, Context ctx) {
    if (Context_GetLoopLevel(ctx) == 0) {
        lfatal(s, "SyntaxError: continue outside of loop");
    }
    Scanner_Scan(s, tok);
    semi(s, tok);
    return ASTnode_NewLeaf(A_CONTINUE, P_NONE, NULL, NULL, 0);
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

    bool customLabel = false;
    char *label = NULL;

    if (tok->token == T_LBRACKET) {
        Scanner_Scan(s, tok);
        customLabel = true;
        if (tok->token != T_IDENT)
            lfatal(s, "SyntaxError: Expected identifier in custom label");
        label = strdup(s->text);
        Scanner_Scan(s, tok);
        rbracket(s, tok);
    }

    lparen(s, tok);
    left = ASTnode_Order(c, s, st, tok, ctx, 0);
    rparen(s, tok);

    lbrace(s, tok);

    debug("after lbrace");

    if (!inttype(left->type)) {
        fatal("TypeError: switch expression must be an integer");
    }

    n = ASTnode_NewUnary(A_SWITCH, P_NONE, left, NULL, NULL, 0);

    if (customLabel) {
        n->label.customLabel = label;
        n->label.hasCustomLabel = customLabel;
    }

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

                bool caseCustomLabel = false;
                char *caseLabel = NULL;

                if (tok->token == T_DEFAULT) {
                    debug("A DEFAULT");

                    op = A_DEFAULT;
                    seenDefault = true;
                    Scanner_Scan(s, tok);
                } else {
                    debug("A CASE");
                    op = A_CASE;
                    Scanner_Scan(s, tok);
                    left = ASTnode_Order(c, s, st, tok, ctx, 0);

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

                if (tok->token == T_LBRACKET) {
                    Scanner_Scan(s, tok);
                    caseCustomLabel = true;
                    if (tok->token != T_IDENT)
                        lfatal(
                            s,
                            "SyntaxError: Expected identifier in custom label");
                    caseLabel = strdup(s->text);
                    Scanner_Scan(s, tok);
                    rbracket(s, tok);
                }

                match(s, tok, T_COLON, ":");

                // Edge case : empty
                if (tok->token != T_CASE) {
                    left = Compound_Statement(c, s, st, tok, ctx, true);
                } else {
                    left = NULL;
                }

                debug("left the case");
                caseCount++;

                if (caseTree == NULL) {
                    caseTree = caseTail = ASTnode_NewUnary(
                        op, P_NONE, left, NULL, NULL, caseValue);
                } else {
                    caseTail->right = ASTnode_NewUnary(op, P_NONE, left, NULL,
                                                       NULL, caseValue);
                    caseTail = caseTail->right;
                }

                if (caseCustomLabel) {
                    caseTail->label.customLabel = caseLabel;
                    caseTail->label.hasCustomLabel = caseCustomLabel;
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