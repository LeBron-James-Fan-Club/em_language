# TODO
- Have ownership system - hardest?
- defer functions
- Add headings in beginning of every function
    - Frame, uses, clobbers
    - Will need to use a tmp txt file to cache assembly beforehand
     to write the reg uses
- Add structure comment
- all int is explicit - done
    - u8, i8, u16, i16, u32, i32

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

- maybe instead make a pseudo stack and push
 and pop from the stack instead of calculating offsets
 (inefficient but it would go with style)

- for unit tests:
    - compile into mips using compiler
    - test that executable with output of mipsy (diff)

- add comments - done
- add namespaces
    - Perhaps it could be added as a prefix to the variable names
- add octal support

# BUGS:
- make it so that multiple files are included

- recursive typedef not supported

- make sure to restore stack pointer afterwards
    - sp offset by 4 (main) and 8 (deez nuts function)
    (HORRIBLE FIX BUT IT WORKS)

- parameters with same names collide with other functions :(
    - need to add a prefix to the parameters - fixed? (idk)

- doesn't check for parameter count

- proto forgets to add the parameters - fixed?
- input fucks up stack frame - fixed
    - parameters are now probs fucked tho

- need somewhere to store the register $a0 before its used for a syscall
    - will need to push then pop - fixed

- unncessary space created when creating union