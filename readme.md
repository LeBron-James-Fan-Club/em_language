# Em Language

### This converts c-like code into MIPS (32 bit)

## Features

- If statements
- While/For loops
- Gotos/Labels
- Hexadecimal support
- Functions
- Function arguments
- Global/Local variables
- Arrays (no multidemensional support)
- Indexing
- Int/Char types
- Poke/Peek
- Bitwise operators
- Pre/Post increment/decrement
- Pointers
- String literals
- Input (String, Char, Int)/Print
- Normal mathy stuff and negation
  - \- \+ % / \*
- Bodmas
- Logical operators (besides || and &&)
- Structs
- Union
- Comments
- typedef
- enums
- break/continue
- switch
- macros
- Array initisaliation
- Local variable initialisation
- Type casting
- octal support
- Sizeof operator
- Ternary operator
- || and && operators
- Function headings (WIP)
- Register spilling (broken at the moment)
- Lazy Evaluation

## What will be added soon (Hopefully)

- Multidimensional arrays
- Compiler directives
  - Including the ability to manually name any label
- Namespace
- Inline assembly and registers
- Scopes (probs wont be done)
- scope creation (probs wont be done)

## Low chance of being added but I might try

- Heap support
- unsigned types
- Optimization

# Documentation

## Order of Procedence

The higher the number, the higher the procedence.

| Operator | Precedence |
| -------- | ---------- |
| ( )      | 13         |
| <        | 12         |
| >        | 12         |
| <=       | 12         |
| >=       | 12         |
| ==       | 11         |
| !=       | 11         |
| %        | 10         |
| /        | 10         |
| \*       | 10         |
| +        | 9          |
| -        | 9          |
| <<       | 8          |
| >>       | 8          |
| &        | 7          |
| ^        | 6          |
| \|       | 5          |
| &&       | 4          |
| \|\|     | 3          |
| ? :      | 2          |
| =        | 1          |
| +=       | 1          |
| -=       | 1          |
| \*=      | 1          |
| /=       | 1          |
| %=       | 1          |
| ,        | 0          |

## Preprocessor

#### BNF for include and define:

```
    <preprocessor> ::= "#" <directive>
    <directive> ::= "include" (<string> | "<" <text> ">" ) | "define" <id> <string>
```

#### Example:

```
    #include "stdio.h"
    #include <stdio.h>
    #define PI 3.14
```

**NOTE** Includes for the standard library is different to C.
Also its currently not working as intended.

## Variables

#### BNF for global variables:

```
    <statement> ::= <type> <declaration> ( "," <declaration> )* ";"
    <declaration> ::= <identifier> [ "[" <number> "]" ] [ "=" <value> | "=" "{" <value> ( "," <value> )* "}" ]
```

#### Example:

```
    i32 a = 2;
    i8 bob;
    i32 b, c = 3;
    i32 d[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }, e;
    i32 d[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }, e = 2;
    i32 d[10], e = 2;
    i32 d[10], e;
    i8 *a = "Hello";
    i8 *deez_nuts = "Gottem";
```

**NOTE**: You can only go up to 15 \*'s.

#### BNF for local variables:

```
    <statement> ::= <type> <declaration> ( "," <declaration> )* ";"
    <declaration> ::= <identifier> [ "[" <expression> "]" ] | <identifier> [ "=" <expression> ]
```

#### Example:

```
   i32 a;
   i32 a = 2, b;
   i32 a[10], b;
   i8 *a;
   i8 **a;
```

**NOTE**: You cannot initialise local arrays.<br>
**NOTE**: You can only go up to 15 \*'s.

## Structs

#### BNF for structs:

```
    <struct> ::= "struct" <id> "{" <statements> "}"
```

#### Example:

```
    struct Jeff {
        i32 money;
        i8 *name;
        i8 address[120];
        struct Jeff *next;
    }
```

**NOTE**: You cannot initialise values in the struct.

## Enums

#### BNF for enums:

```
    <enum> ::= "enum" <id> "{" <id> [ "=" <number> ] ( "," <id> [ "=" <number> ] )* "}"
```

#### Example:

```
    enum Jeff {
        JEFF = 1,
        JEFFERY = 2,
        JEFFREY = 3
    }
```

## Typedef

#### BNF for typedef:

```
    <typedef> ::= "typedef" <type> <id>
```

#### Example:

```
    typedef i32 Jeff;
    typedef i8 *Jeff;
    typedef struct Jeff poo;
```

## Unions

#### BNF for unions:

```
    <union> ::= "union" <id> "{" <statements> "}"
```

#### Example:

```
    union Jeff {
        i32 money;
        i8 *name;
        i8 address[120];
        struct Jeff *next;
    }
```

## Functions

#### BNF for functions:

```
       <func> ::= <type> <id> "(" [ <args> ] ")" "{" <stmts> "}"
```

#### Example:

```
    i32 Domain_Expansion(i32 a, i32 b) {
        print("Nah I'd win", 10);
        print("a: ", a, ", b: ", b, 10);
        return 0;
    }
```

#### BNF for function calls:

```
    <call> ::= <id> "(" [ <args> ] ")"
```

#### Example:

```
    Domain_Expansion(69, 420);
```

## Pointers

#### BNF for pointers:

```
    <ptr> ::= "*" <id>
```

#### Example:

```
    i32 a = 2;
    i32 *b = &a;
    i32 **c = &b;
    print(**c, 10);
```

**NOTE**: You can only go up to 15 \*'s.

## Gotos/Labels

#### BNF for labels:

```
    <label> ::= "label" <id> ":"
```

#### Example:

```
    label start:
    print("Hello", 10);
```

#### BNF for gotos:

```
    <goto> ::= "goto" <id> ";"
```

#### Example:

```
    label start;
    print("This is gold experience requiem", 10);
    goto start;
```

## If statements

#### BNF for if statements:

```
    <if> ::= "if" "(" <expr> ")" "{" <stmts> "}"
```

#### Example:

```
    i32 a = 2;
    if (a == 2) {
        print("a is 2", 10);
    }
```

## While loops

#### BNF for while loops:

```
    <while> ::= "while" "(" <expr> ")" "{" <stmts> "}"
```

#### Example:

```
    i32 a = 0;
    while (a < 10) {
        print(a, 10);
        a = a + 1;
    }
```

## For loops

#### BNF for for loops:

```
    <for> ::= "for" "(" <set> <expr> ";" <set> ")" "{" <stmts> "}"
```

#### Example:

```
    i32 a = 0;
    for (a = 0; a < 10; a++) {
        print(a, 10);
    }
```

## Break/Continue

#### BNF for break/continue:

```
    <breakcont> ::= "break" ";"
                 | "continue" ";"
```

#### Example:

```
    i32 a = 0;
    for (a = 0; a < 10; a++) {
        if (a == 5) {
            break;
        }
        print(a, 10);
    }
```

## Switch

#### BNF for switch:

```
    <switch> ::= "switch" "(" <expr> ")" "{" <cases> "}"
    <cases> ::= <case> <cases>
              | <default>
    <case> ::= "case" <expr> ":" <stmts>
    <default> ::= "default" ":" <stmts>
```

#### Example:

```
    i32 a = 2;
    switch (a) {
        case 1:
            print("a is 1", 10);
            break;
        case 2:
            print("a is 2", 10);
            break;
        default:
            print("a is not 1 or 2", 10);
            break;
    }
```

## Input/Print

#### BNF for input:

```
    <input> ::= "input" "(" <id> "," <type> ")"
```

#### Example:

```
    i32 a;
    input(a, i32);
    i8 choice;
    input(choice, i8);
    i8 creditCardDetails[120];
    input(creditCardDetails, i8*);
```

**NOTE**: Input implicitly uses the array size.

#### BNF for print:

```
    <print> ::= "print" "(" <args> ")"
```

#### Example:

```
    print("Hello", 10);
```

## Bitwise operators

#### BNF for bitwise operators:

```
    <bitwise> ::= <expr> "&" <expr>
                | <expr> "|" <expr>
                | <expr> "^" <expr>
                | <expr> "<<" <expr>
                | <expr> ">>" <expr>
```

#### Example:

```
    i32 a = 2;
    i32 b = 3;
    print(a & b, 10);
    print(a | b, 10);
    print(a ^ b, 10);
    print(a << b, 10);
    print(a >> b, 10);
```

## Logical operators

#### BNF for logical operators:

```
    <logical> ::= <expr> "==" <expr>
                | <expr> "!=" <expr>
                | <expr> "<" <expr>
                | <expr> ">" <expr>
                | <expr> "<=" <expr>
                | <expr> ">=" <expr>
```

#### Example:

```
    i32 a = 2;
    i32 b = 3;
    print(a == b, 10);
    print(a != b, 10);
    print(a < b, 10);
    print(a > b, 10);
    print(a <= b, 10);
    print(a >= b, 10);
```

#### BNF for ternary operator:

```
    <ternary> ::= <expr> "?" <expr> ":" <expr>
```

#### Example:

```
    i32 a = 2;
    i32 b = 3;
    print(a == b ? 1 : 0, 10);
```

#### BNF for and and or operators:

```
    <andor> ::= <expr> "&&" <expr>
              | <expr> "||" <expr>
```

#### Example:

```
    i32 a = 2;
    i32 b = 3;
    print(a == 2 && b == 3, 10);
    print(a == 2 || b == 3, 10);
```

#### BNF for not operator:

```
    <not> ::= "!" <expr>
```

## Pre/Post increment/decrement

#### BNF for pre/post increment/decrement:

```
    <incdec> ::= "++" <id>
               | "--" <id>
               | <id> "++"
               | <id> "--"
```

#### Example:

```
    i32 a = 2;
    print(a++, 10);
    print(a, 10);
    print(++a, 10);
    print(a, 10);

    print(a--, 10);
    print(a, 10);
    print(--a, 10);
    print(a, 10);
```

**NOTE**: You can only have one increment/decrement per statement.

## Poke/Peek

#### BNF for poke/peek:

```
    <pokepeek> ::= "poke" "(" <expr> "," <expr> ")"
                 | "peek" "(" <expr> ")"
```

#### Example:

```
    i32 a = 2;
    poke(0x10010000, a);
    print(peek(0x10010000), 10);
```

## String literals

#### BNF for string literals:

```
    <string> ::= <string> <string>
               | <char>
```

#### Example:

```
    i8 *sigma = "Skibidi toilet\n";
    i8 *hello = "how, " "are, " "you?\n";
```

## Artihmetic

#### BNF for arithmetic:

```
    <arith> ::= <expr> "+" <expr>
              | <expr> "-" <expr>
              | <expr> "*" <expr>
              | <expr> "/" <expr>
              | <expr> "%" <expr>
```

#### Example:

```
    i32 a = 2;
    i32 b = 3;
    print(a + b, 10);
    print(a - b, 10);
    print(a * b, 10);
    print(a / b, 10);
    print(a % b, 10);
```

## Parentheses

#### BNF for parentheses:

```
    <paren> ::= "(" <expr> ")"
```

#### Example:

```
    i32 a = 2;
    i32 b = 3;
    print((a + b) * 2, 10);
```

## Type casting

#### BNF for type casting:

```
    <cast> ::= "(" <type> ")" <expr>
```

#### Example:

```
    i32 a = 2;
    i8 b = (i8)a;
    print(b, 10);
```

## Octal support

#### BNF for octal support:

```
    <octal> ::= "0" <octal>
             | "0"
```

#### Example:

```
    i32 a = 010;
    print(a, 10);
```

## Hexadecimal support

#### BNF for hexadecimal support:

```
    <hex> ::= "0x" <hex>
           | "0x" <digit>
           | "0x" <letter>
```

#### Example:

```
    i32 a = 0x10;
    print(a, 10);
```

## Sizeof operator

#### BNF for sizeof operator:

```
    <sizeof> ::= "sizeof" "(" <type> ")"
```

#### Example:

```
    print(sizeof(i32), 10);
    print(sizeof(i8), 10);
    print(sizeof(i32*), 10);
    print(sizeof(i8*), 10);
```
