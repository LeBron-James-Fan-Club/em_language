#ifndef DEFS_H
#define DEFS_H

// Commonly used stuff - prevents circular includes

#define CPPCMD "cpp -P -nostdinc -isystem"
// declared in makeifle
#ifndef INCDIR
#define INCDIR "./bin/include"
#endif

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

    T_ASSIGNADD,
    T_ASSIGNSUB,
    T_ASSIGNMUL,
    T_ASSIGNDIV,
    T_ASSIGNMOD,

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

    T_INT,  // 22
    T_STRUCT,

    T_IF,     // 23
    T_ELSE,   // 24
    T_LABEL,  // 25
    T_GOTO,   // 26
    T_WHILE,  // 27
    T_FOR,    // 28
    T_VOID,
    T_CHAR,
    T_COMMA,
    T_DOT,
    T_ARROW,
    T_UNION,
    T_ENUM,
    T_TYPEDEF,
    T_BREAK,
    T_CONTINUE,
    T_SWITCH,
    T_CASE,
    T_DEFAULT,
    T_COLON,
    T_EXTERN,
    T_RETURN,
    T_STRLIT

};

enum ASTOP {
    // 1:1 (almost) with tokens
    A_NONE,

    // START OF SEMI CHECKING
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

    A_PREINC,
    A_PREDEC,
    A_POSTINC,
    A_POSTDEC,

    A_NEGATE,
    A_INVERT,
    A_LOGNOT,

    A_IDENT,

    // END OF SEMI CHECKING

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

    A_TOBOOL,

    A_BREAK,
    A_CONTINUE,

    A_SWITCH,
    A_CASE,
    A_DEFAULT,

    // misc so the compiler stops complaining
    A_STARTPAREN

};

// Primitives

// We use bitwise to keep track of pointers
// con: can only support up to 4 pointers (find a fix for this soon)
// anything in 0xf (1111) is assumed a pointer
enum ASTPRIM {
    P_NONE,

    P_VOID = 1 << 4,
    P_CHAR = 1 << 5,
    P_INT = 1 << 6,

    // P_VOIDPTR,
    // P_CHARPTR,
    // P_INTPTR,

    P_STRUCT = 1 << 7,
    P_UNION = 1 << 8
};

enum STORECLASS {
    C_NONE,
    C_GLOBAL,
    C_LOCAL,
    C_PARAM,
    C_MEMBER,
    C_STRUCT,
    C_UNION,
    C_ENUMTYPE,
    C_ENUMVAL,
    C_TYPEDEF,
    C_EXTERN
};

struct token {
    enum OPCODES token;
    int intvalue;
};

typedef struct token *Token;

#endif