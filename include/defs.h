#ifndef DEFS_H
#define DEFS_H

// Commonly used stuff - prevents circular includes

enum OPCODES {
    T_EOF,  // 0

    T_ASSIGN,  // 1

    // Arithmetic
    T_PLUS,    // 2
    T_MINUS,   // 3
    T_STAR,    // 4
    T_SLASH,   // 5
    T_MODULO,  // 6

    // Comparison
    T_EQ,  // 7
    T_NE,  // 8
    T_LT,  // 9
    T_GT,  // 10
    T_LE,  // 11
    T_GE,  // 12

    T_INTLIT,  // 13
    T_SEMI,    // 14
    T_IDENT,  // 15

        // Braces
    T_LBRACE,  // 16
    T_RBRACE,  // 17
    T_LPAREN,  // 18
    T_RPAREN,  // 19
    T_LBRACKET, // 20
    T_RBRACKET, // 21

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
    T_RETURN,
    T_AMPER,
    T_LOGAND

};

enum ASTOP {
    // 1:1 (almost) with tokens
    A_NONE,
    A_ASSIGN,
    A_ADD,
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
    // for some god damn reason
    // commenting out this line causes a segfault
    A_LVIDENT,
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
    A_RETURN,
    A_ADDR,
    A_DEREF,
    A_SCALE,
    
    // misc so the compiler stops complaining
    A_STARTPAREN

};

// Primitives
enum ASTPRIM {
    P_NONE,

    P_VOID,
    P_CHAR,
    P_INT,

    P_VOIDPTR,
    P_CHARPTR,
    P_INTPTR
};


struct token {
    enum OPCODES token;
    int intvalue;
};

typedef struct token *Token;

#endif