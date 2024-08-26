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

# BUGS:
- make sure to restore stack pointer afterwards
    - sp offset by 4 (main) and 8 (deez nuts function)
    (HORRIBLE FIX BUT IT WORKS)

- parameters with same names collide with other functions :(
    - need to add a prefix to the parameters

- proto forgets to add the parameters
