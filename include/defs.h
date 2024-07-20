#ifndef DEFS_H
#define DEFS_H

// Commonly used stuff - prevents circular includes

#define DEBUG 1

enum OPCODES {
    T_EOF,  // 0

    T_ASSIGN,  // 1

    // Not supported yet
    T_LOGOR,
    T_LOGAND,

    T_OR,
    T_XOR,
    T_AMPER,

    // Comparison
    T_EQ,  // 7
    T_NE,  // 8

    T_LT,  // 9
    T_GT,  // 10
    T_LE,  // 11
    T_GE,  // 12

    T_LSHIFT,
    T_RSHIFT,

    // Arithmetic
    T_PLUS,    // 2
    T_MINUS,   // 3
    T_STAR,    // 4
    T_SLASH,   // 5
    T_MODULO,  // 6

    T_INC,
    T_DEC,
    T_INVERT,
    T_LOGNOT,

    T_INTLIT,  // 13
    T_SEMI,    // 14
    T_IDENT,   // 15

    // Braces
    T_LBRACE,    // 16
    T_RBRACE,    // 17
    T_LPAREN,    // 18
    T_RPAREN,    // 19
    T_LBRACKET,  // 20
    T_RBRACKET,  // 21

    // Keywords
    T_PRINT,  // 20

    // INPUT a - takes input and stores it in a
    T_INPUT,  // 21
    
    // dangerous shit
    T_PEEK,
    T_POKE,

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
    T_STRLIT

};

enum ASTOP {
    // 1:1 (almost) with tokens
    A_NONE,

    A_ASSIGN,

    // Not supported yet
    A_LOGOR,
    A_LOGAND,

    A_OR,
    A_XOR,
    A_AND,

    A_EQ,
    A_NE,

    A_LT,
    A_GT,
    A_LE,
    A_GE,

    A_LSHIFT,
    A_RSHIFT,

    A_ADD,
    A_SUBTRACT,
    A_MULTIPLY,
    A_DIVIDE,
    A_MODULO,

    A_INTLIT,

    // 1:1 ends here

    // After this we need checks
    A_IDENT,

    // for some god damn reason
    // commenting out this line causes a segfault
    A_LVIDENT,

    A_PRINT,
    A_INPUT,
    
    A_PEEK,
    A_POKE,

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
    A_STRLIT,

    A_PREINC,
    A_PREDEC,
    A_POSTINC,
    A_POSTDEC,

    A_NEGATE,
    A_INVERT,
    A_LOGNOT,

    A_TOBOOL,

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

enum STORECLASS {
    C_GLOBAL = 1,
    C_LOCAL,
    C_PARAM
};

struct token {
    enum OPCODES token;
    int intvalue;
};

typedef struct token *Token;

#endif