#include <stdlib.h>

#include "stmt.h"
#include "misc.h"

static ASTnode poke_statement(Compiler c, Scanner s, SymTable st, Token tok,
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

    ASTnode left = nullptr;
    ASTnode tree = nullptr;

    while (true) {
        // TODO:  Compiler directive will be checked here
        tree = single_statement(c, s, st, tok, ctx);

        if (tree != nullptr) {
            debug("op %d", tree->op);
        }
        if (tree != nullptr &&
            (/*(tree->op >= A_ASSIGN && tree->op <= A_IDENT) ||*/
             tree->op == A_RETURN || tree->op == A_FUNCCALL ||
             tree->op == A_LABEL || tree->op == A_GOTO || tree->op == A_POKE ||
             tree->op == A_BREAK || tree->op == A_CONTINUE)) {
            debug("consume semi");
            s->semi(tok);
        }

        if (tree) {
            left = (left == nullptr) ? tree
                                  : ASTnode_New(A_GLUE, P_NONE, left, nullptr,
                                                tree, nullptr, nullptr, 0);
        }

        if (tok->token == T_RBRACE) {
            // s->rbrace(tok);
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
            if (st->SymTable_FindTypeDef(s) == nullptr) {
                stmt = ASTnode_Order(c, s, st, tok, ctx);
                s->semi(tok);
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
            s->semi(tok);
            return stmt;
        case T_POKE:
            return poke_statement(c, s, st, tok, ctx);
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
            s->lbrace(tok);
            // Edgecase: Empty
            if (tok->token == T_RBRACE) {
                s->rbrace(tok);
                return nullptr;
            }
            stmt = Compound_Statement(c, s, st, tok, ctx, false);
            s->rbrace(tok);
            return stmt;
        default:
            stmt = ASTnode_Order(c, s, st, tok, ctx);
            s->semi(tok);
            return stmt;
    }
}

static ASTnode poke_statement(Compiler c, Scanner s, SymTable st, Token tok,
                              Context ctx) {
    ASTnode param1, param2;
    s->match(tok, T_POKE, "poke");
    s->lparen(tok);
    param1 = ASTnode_Order(c, s, st, tok, ctx);
    s->comma(tok);
    param2 = ASTnode_Order(c, s, st, tok, ctx);
    s->rparen(tok);
    return ASTnode_New(A_POKE, P_NONE, param2, nullptr, param1, nullptr, nullptr, 0);
}

static ASTnode print_statement(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {
    ASTnode t;

    s->match(tok, T_PRINT, "print");

    // Might break if i add more types not compatable

    // loops til no more commas - we can do hello world now
    ASTnode parent = nullptr;
    bool firstGone = false;
    s->lparen(tok);
    do {
        if (parent) {
            s->Scanner_Scan(tok);
        }
        t = ASTnode_Order(c, s, st, tok, ctx);

        int rightType = t->type;
        t->rvalue = true;

        // P_NONE should never occur thereotically for now
        if (rightType == P_NONE || rightType == P_VOID) {
            fprintf(stderr, "Error: Type mismatch on line %d\n", s->line);
            exit(-1);
        }

        t = ASTnode_NewUnary(A_PRINT, static_cast<enum ASTPRIM>(rightType), t, nullptr, nullptr, 0);

        if (firstGone) {
            // memory leak occurs here
            parent =
                ASTnode_New(A_GLUE, P_NONE, parent, nullptr, t, nullptr, nullptr, 0);
        } else {
            parent = t;
            firstGone = true;
        }
    } while (tok->token == T_COMMA);
    s->rparen(tok);

    s->semi(tok);

    return parent;
}

static ASTnode input_statement(Scanner s, SymTable st, Token tok, Context ctx) {
    SymTableEntry var;

    // input(variable, type);

    s->match(tok, T_INPUT, "input");
    s->lparen(tok);

    s->ident(tok);
    
    if ((var = st->SymTable_FindSymbol(s, ctx)) == nullptr) {
        lfatala(s, "UndefinedError: Undefined variable %s", s->text);
    }

    s->comma(tok);

    ASTnode left;
    bool isStr = false;

    switch (tok->token) {
        case T_CHAR:
            s->Scanner_Scan(tok);
            if (tok->token == T_STAR) {
                debug("FOUND STRING");
                s->Scanner_Scan(tok);
                isStr = true;
                left =
                    ASTnode_NewLeaf(A_INPUT, pointer_to(P_CHAR), nullptr, var, 0);
            } else {
                s->Scanner_RejectToken(tok);
                left = ASTnode_NewLeaf(A_INPUT, P_CHAR, nullptr, nullptr, 0);
            }
            break;
        case T_INT:
            left = ASTnode_NewLeaf(A_INPUT, P_INT, nullptr, nullptr, 0);
            break;
        default:
            lfatal(s, "TypeError: Unknown type given for input");
    }

    if (isStr) {
        s->rparen(tok);
        s->semi(tok);
        return left;
    }

    ASTnode right =  ASTnode_NewLeaf(A_IDENT, var->type, var->ctype, var, 0);
    right->rvalue = 0;  // We do not need to load the value of the variable

    ASTnode tree = modify_type(right, left->type, left->ctype, A_NONE);

    tree = ASTnode_New(A_ASSIGN, P_NONE, left, nullptr, tree, nullptr, nullptr, 0);
    s->rparen(tok);

    s->semi(tok);

    return tree;
}

static ASTnode if_statement(Compiler c, Scanner s, SymTable st, Token tok,
                            Context ctx) {
    ASTnode condAST, trueAST, falseAST = nullptr;

    s->match(tok, T_IF, "if");
    s->lparen(tok);

    condAST = ASTnode_Order(c, s, st, tok, ctx);

    // Might remove self guard later
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        condAST =
            ASTnode_NewUnary(A_TOBOOL, condAST->type, condAST, nullptr, nullptr, 0);
    }

    s->rparen(tok);

    trueAST = single_statement(c, s, st, tok, ctx);

    if (tok->token == T_ELSE) {
        s->Scanner_Scan(tok);
        falseAST = single_statement(c, s, st, tok, ctx);
    }

    return ASTnode_New(A_IF, P_NONE, condAST, trueAST, falseAST, nullptr, nullptr, 0);
}

static ASTnode while_statement(Compiler c, Scanner s, SymTable st, Token tok,
                               Context ctx) {
    ASTnode condAST, bodyAST;

    s->match(tok, T_WHILE, "while");
    s->lparen(tok);

    condAST = ASTnode_Order(c, s, st, tok, ctx);
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }

    s->rparen(tok);

    Context_IncLoopLevel(ctx);
    bodyAST = single_statement(c, s, st, tok, ctx);
    Context_DecLoopLevel(ctx);

    return ASTnode_New(A_WHILE, P_NONE, condAST, nullptr, bodyAST, nullptr, nullptr, 0);
}

static ASTnode for_statement(Compiler c, Scanner s, SymTable st, Token tok,
                             Context ctx) {
    ASTnode condAST, bodyAST;
    ASTnode preopAST, postopAST;
    ASTnode t;

    s->match(tok, T_FOR, "for");
    s->lparen(tok);

    preopAST = expression_list(c, s, st, tok, ctx, T_SEMI);
    s->semi(tok);

    debug("WE ARE COND :L");

    condAST = ASTnode_Order(c, s, st, tok, ctx);
    if (condAST->op < A_EQ || condAST->op > A_GE) {
        fprintf(stderr, "Error: Bad comparison operator\n");
        exit(-1);
    }
    debug("EATING THE SEMMI :L");
    s->semi(tok);

    debug("WE ARE POST :L");

    postopAST = expression_list(c, s, st, tok, ctx, T_RPAREN);
    s->rparen(tok);

    Context_IncLoopLevel(ctx);
    bodyAST = single_statement(c, s, st, tok, ctx);
    Context_DecLoopLevel(ctx);

    t = ASTnode_New(A_GLUE, P_NONE, bodyAST, nullptr, postopAST, nullptr, nullptr, 0);
    t = ASTnode_New(A_WHILE, P_NONE, condAST, nullptr, t, nullptr, nullptr, 0);
    return ASTnode_New(A_GLUE, P_NONE, preopAST, nullptr, t, nullptr, nullptr, 0);
}

static ASTnode label_statement(Scanner s, SymTable st, Token tok) {
    s->match(tok, T_LABEL, "label");
    s->ident(tok);

    SymTableEntry var = st->SymTable_AddGlob(s->text, P_NONE, nullptr, S_LABEL,
                                         C_GLOBAL, 0, false);

    ASTnode t = ASTnode_NewLeaf(A_LABEL, P_NONE, nullptr, var, 0);

    return t;
}

static ASTnode goto_statement(Scanner s, SymTable st, Token tok) {
    SymTableEntry var;

    s->match(tok, T_GOTO, "goto");
    // ! Might be buggy?
    s->ident(tok);
    if ((var = st->SymTable_FindGlob(s)) == nullptr) {
        lfatala(s, "UndefinedError: Undefined label %s", s->text);
    }

    ASTnode t = ASTnode_NewLeaf(A_GOTO, P_NONE, nullptr, var, 0);

    return t;
}

static ASTnode return_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx) {
    ASTnode t;
    SymTableEntry func = Context_GetFunctionId(ctx);
    if (func == nullptr || func->name == nullptr) {
        fprintf(stderr, "Error: Return outside of function on line %d\n",
                s->line);
        exit(-1);
    }

    if (func->type == P_VOID) {
        fprintf(stderr, "Error: Return in void function on line %d\n", s->line);
        exit(-1);
    }

    s->match(tok, T_RETURN, "return");

    t = ASTnode_Order(c, s, st, tok, ctx);
    t->rvalue = 1;

    debug("func type %d, t type %d", func->type, t->type);

    t = modify_type(t, func->type, func->ctype, A_NONE);
    if (t == nullptr) {
        fprintf(stderr, "Error: Type mismatch in return on line %d\n", s->line);
        exit(-1);
    }
    return ASTnode_NewUnary(A_RETURN, P_NONE, t, nullptr, nullptr, 0);
}

static ASTnode break_statement(Scanner s, Token tok, Context ctx) {
    if (Context_GetLoopLevel(ctx) == 0) {
        lfatal(s, "SyntaxError: break outside of loop");
    }
    s->Scanner_Scan(tok);
    return ASTnode_NewLeaf(A_BREAK, P_NONE, nullptr, nullptr, 0);
}

static ASTnode continue_statement(Scanner s, Token tok, Context ctx) {
    if (Context_GetLoopLevel(ctx) == 0) {
        lfatal(s, "SyntaxError: continue outside of loop");
    }
    s->Scanner_Scan(tok);
    return ASTnode_NewLeaf(A_CONTINUE, P_NONE, nullptr, nullptr, 0);
}

static ASTnode switch_statement(Compiler c, Scanner s, SymTable st, Token tok,
                                Context ctx) {
    // n = node, ca = case
    ASTnode left, n, ca, caseTree = nullptr, caseTail;
    bool inLoop = true, seenDefault = false;
    int caseCount = 0;
    enum ASTOP op;
    int caseValue;

    s->Scanner_Scan(tok);

    s->lparen(tok);
    left = ASTnode_Order(c, s, st, tok, ctx);
    s->rparen(tok);

    s->lbrace(tok);

    debug("after lbrace");

    if (!inttype(left->type)) {
        fatal("TypeError: switch expression must be an integer");
    }

    n = ASTnode_NewUnary(A_SWITCH, P_NONE, left, nullptr, nullptr, 0);

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
                    s->Scanner_Scan(tok);
                } else {
                    debug("A CASE");
                    op = A_CASE;
                    s->Scanner_Scan(tok);
                    left = ASTnode_Order(c, s, st, tok, ctx);

                    if (left->op != A_INTLIT) {
                        lfatal(
                            s,
                            "TypeError: case value must be an integer literal");
                    }
                    caseValue = left->intvalue;
                    debug("case value: %d", caseValue);

                    for (ca = caseTree; ca != nullptr; ca = ca->right) {
                        if (ca->intvalue == caseValue) {
                            lfatala(s,
                                    "DuplicateError: duplicate case value %d",
                                    caseValue);
                        }
                    }

                    // Free it we don't need it anymore
                    ASTnode_Free(left);
                }
                s->match(tok, T_COLON, ":");

                // Edge case : empty
                if (tok->token != T_CASE) {
                    left = Compound_Statement(c, s, st, tok, ctx, true);
                } else {
                    left = nullptr;
                }

                debug("left the case");
                caseCount++;

                if (caseTree == nullptr) {
                    caseTree = caseTail = ASTnode_NewUnary(
                        op, P_NONE, left, nullptr, nullptr, caseValue);
                    debug("first create %p", caseTree);
                } else {
                    caseTail->right = ASTnode_NewUnary(op, P_NONE, left, nullptr,
                                                       nullptr, caseValue);
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

    s->rbrace(tok);

    return n;
}