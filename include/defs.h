#ifndef DEFS_H
#define DEFS_H

// Commonly used stuff - prevents circular includes

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
    T_WHILE,  // 27
    T_FOR,    // 28
    T_VOID,
    T_CHAR,
    T_COMMA,
    T_RETURN

};

enum ASTOP {
    // 1:1 (almost) with tokens
    A_ADD = 1,
    A_SUBTRACT,
    A_MULTIPLY,
    A_DIVIDE,
    A_MODULO,
    
    A_EQ, A_NE,
    A_LT, A_GT,
    A_LE, A_GE,

    A_INTLIT,

    // After this we need checks
    A_IDENT,
    A_LVIDENT,
    A_ASSIGN,
    A_PRINT,
    A_INPUT,
    A_GLUE,
    A_IF,
    A_LABEL,
    A_GOTO,
    
    // for is also A_WHILE
    A_WHILE,
    A_FUNCTION,
    A_WIDEN,
    A_FUNCCALL,
    A_RETURN

};

// Primitives
enum ASTPRIM {
    P_NONE,
    P_VOID,
    P_CHAR,
    P_INT,
    P_LONG
};


struct token {
    enum OPCODES token;
    int intvalue;
};

typedef struct token *Token;

#endif