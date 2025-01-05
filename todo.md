# TODO
- defer functions
- Add headings in beginning of every function
    - Frame, uses, clobbers
    - Will need to use a tmp txt file to cache assembly beforehand
     to write the reg uses
- Add structure comment

- implement 16 bit integers

- vars declaration is b : i32 = 2 * 4;
- need to have directives that define constants in the assembly files

- have a include assembly macro
- e.g. a macro is defined in assembly to the compiler will use that instead

- Implement peek and poke because why not? - Allows for integer values which iirc &(10) is not allowed
    - kinda unneccessary but fun ig
- make bad apple in mips

- need to fix variable index

- add a style flag
- add error recovery

- use saved registers for most used variables ($s0-$s7)

- maybe instead make a pseudo stack and push
 and pop from the stack instead of calculating offsets
 (inefficient but it would go with style)

- for unit tests:
    - compile into mips using compiler
    - test that executable with output of mipsy (diff)

- add comments - done
- add namespaces
    - Perhaps it could be added as a prefix to the variable names
- add octal support - done
- add else if
- ngl I think extern isnt needed cause of the way
 - the compiler is gonna awfully stitch the code
- get unions to work in a struct
- get structs to work in a struct
- add static for local variables
- implement two value return ($v0 and $v1)

# BUGS:
- implement the register keyword
- fix unequal new lines in the assembly file
- make it so that multiple files are included
    - if in include directory, automatically include the file
    - (crappy solution) append each file to each other and hope to god that
    - the preprocessor solves it somehow
    - (better solution) read each file individually then add to assembly file
        - should handle itself because of the way it works

- recursive typedef not supported

- make sure to restore stack pointer afterwards
    - sp offset by 4 (main) and 8 (deez nuts function)
    (HORRIBLE FIX BUT IT WORKS)

- parameters with same names collide with other functions :( - fixed
    - need to add a prefix to the parameters - fixed? (idk)

- doesn't check for parameter count

- proto forgets to add the parameters - fixed?
- input fucks up stack frame - fixed
    - parameters are now probs fucked tho

- need somewhere to store the register $a0 before its used for a syscall
    - will need to push then pop - fixed

- unncessary space created when creating union - fixed

- (BUG) - caused by non initialised variables - fixed
- annyomous strings need to declared first (probs iterate the whole global first for annoymous) - fixed
- mod might be a bit broken - fixed

# REGRETS:
- why didn't I use global variables