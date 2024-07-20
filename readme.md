# Em Language
### This converts c-like code into MIPS (32 bit)

## Features
 - If statements
 - While/For loops
 - Gotos/Labels
 - Hexadecimal support
 - Functions
 - Function arguments - currently broken
 - Global/Local variables
 - Arrays (Only supported globally and no multidemensional support)
 - Indexing
 - Int/Char types (will soon be called i32 and i8 tho)
 - Poke/Peek
 - Bitwise operators
 - Pre/Post increment/decrement
 - Pointers
 - String literals
 - Input (integer only)/Print
 - Normal mathy stuff and negation
    - \- \+ % / \*
 - Bodmas
 - Logical operators (besides || and &&)

## What will be added soon (Hopefully)
 - Local arrays
 - Multidimensional arrays
 - Function headings
 - Compiler directives
    - Including the ability to manually name any label
 - Macros?
 - Structs
 - Array initisaliation
 - Local variable initialisation
 - Namespace
 - || and && operators
 - Inline assembly and registers
 - Scopes
 - Sizeof operator
 - Type casting
 - Comments

## Low chance of being added but I might try
 - Heap support
 - unsigned types
 - Optimization

# Documentation
 
 ## Order of Procedence
 The higher the number, the higher the procedence.

 | Operator | Precedence |
 |----------|------------|
 | ( )      | 10         |
 | <        | 9          |
 | >        | 9          |
 | <=       | 9          |
 | >=       | 9          |
 | ==       | 8          |
 | !=       | 8          |
 | %        | 7          |
 | /        | 7          |
 | *        | 7          |
 | +        | 6          |
 | -        | 6          |
 | <<       | 5          |
 | >>       | 5          |
 | &        | 4          |
 | ^        | 3          |
 | \|       | 2          |
 | =        | 1          |
 | ,        | 0          |

 ## Variables
 
 ####  BNF for global variables:
 ```
    <var> ::= <type> <id> [ "=" <expr> ] ";"
 ```

 #### Example:
 ```
    int a = 2;
 ```
 
 #### BNF for local variables:
 ```
        var> ::= <type> <id> ";"
 ```

 #### Example:
 ```
    int a;
 ```

 #### BNF to set local/global variables:
 ```
    <set> ::= <id> "=" <expr> ";"
 ```
 __NOTE__: For global variables, you can only inside the function.

 ## Functions

 #### BNF for functions:
 ```
        <func> ::= <type> <id> "(" [ <args> ] ")" "{" <stmts> "}"
 ```

#### Example:

```
    int Domain_Expansion(int a, int b) {
        print("Nah I'd win", 10);
        print("a: ", a, ", b: ", b, 10);
        return 0;
    }
```

#### BNF for function arguments:
```
    <args> ::= <type> <id> [ "," <args> ]
```

#### Example:
```
    int Domain_Expansion(int a) {
        a = 2;
        print("a: ", a, 10);
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

## Arrays

#### BNF for global arrays:
```
    <array> ::= <type> <id> "[" <expr> "]" ";"
```

#### Example:
```
    int a[10];
```

#### BNF for setting arrays:
```
    <set> ::= <id> "[" <expr> "]" "=" <expr> ";"
```

#### Example:
```
    a[0] = 360;
```

## Pointers

#### BNF for pointers:
```
    <ptr> ::= "*" <id>
```

#### Example:

```
    int a = 2;
    int *b = &a;
    int **c = &b;
    print(**c, 10);
```

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
    int a = 2;
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
    int a = 0;
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
    int a = 0;
    for (a = 0; a < 10; a++) {
        print(a, 10);
    }
```

## Input/Print

#### BNF for input:
```
    <input> ::= "input" "(" <id> ")"
```

#### Example:
```
    int a;
    input(a);
```

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
    int a = 2;
    int b = 3;
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
    int a = 2;
    int b = 3;
    print(a == b, 10);
    print(a != b, 10);
    print(a < b, 10);
    print(a > b, 10);
    print(a <= b, 10);
    print(a >= b, 10);
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
    int a = 2;
    print(a++, 10);
    print(a, 10);
    print(++a, 10);
    print(a, 10);
    
    print(a--, 10);
    print(a, 10);
    print(--a, 10);
    print(a, 10);
```

## Poke/Peek

#### BNF for poke/peek:
```
    <pokepeek> ::= "poke" "(" <expr> "," <expr> ")"
                 | "peek" "(" <expr> ")"
```

#### Example:
```
    int a = 2;
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
    char *sigma = "Skibidi toilet\n";
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
    int a = 2;
    int b = 3;
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
    int a = 2;
    int b = 3;
    print((a + b) * 2, 10);
```




