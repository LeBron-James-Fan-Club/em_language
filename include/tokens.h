#ifndef DEFS_H
#define DEFS_H

enum OPCODES {
    T_EOF,  // 0

    // Arithmetic
    T_PLUS,    // 1
    T_MINUS,   // 2
    T_STAR,    // 3
    T_SLASH,   // 4
    T_MODULO,  // 5

    // Comparison
    T_EQ,  // 6
    T_NE,  // 7
    T_LT,  // 8
    T_GT,  // 9
    T_LE,  // 10
    T_GE,  // 11

    // Braces
    T_LBRACE,  // 12
    T_RBRACE,  // 13
    T_LPAREN,  // 14
    T_RPAREN,  // 15

    T_INTLIT,  // 16
    T_SEMI,    // 17

    T_ASSIGN,  // 18

    T_IDENT,  // 19

    // Keywords
    T_PRINT,  // 20

    // INPUT a - takes input and stores it in a
    T_INPUT,  // 21
    T_INT,    // 22
    T_IF,     // 23
    T_ELSE,   // 24
    T_LABEL,  // 25
    T_GOTO,   // 26
    T_WHILE   // 27

};

struct token {
    enum OPCODES token;
    int intvalue;
};

typedef struct token *Token;

#endif