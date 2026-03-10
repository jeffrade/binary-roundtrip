/*
 * Module 2: C Compilation Pipeline — Multi-file Project (Implementation)
 * ======================================================================
 *
 * WHAT THIS FILE DEMONSTRATES:
 *   This is the IMPLEMENTATION FILE. It contains the actual function
 *   definitions (the code that runs) for the functions declared in
 *   math_utils.h.
 *
 * SEPARATE COMPILATION:
 *   This file is compiled independently from multi_file_main.c:
 *     gcc -c math_utils.c    → produces math_utils.o
 *     gcc -c multi_file_main.c → produces multi_file_main.o
 *
 *   Each .o file contains compiled machine code but with UNRESOLVED
 *   REFERENCES. For example, multi_file_main.o has a call to add()
 *   but doesn't know its address yet.
 *
 * LINKING CONNECTS THEM:
 *   gcc -o multi_file multi_file_main.o math_utils.o
 *
 *   The linker:
 *     1. Finds the definition of add() in math_utils.o
 *     2. Finds the reference to add() in multi_file_main.o
 *     3. Fills in the correct address so the call instruction works
 *
 * WHAT TO OBSERVE:
 *   After compiling:
 *     nm math_utils.o
 *       → Shows add, multiply, factorial as "T" (defined in Text section)
 *
 *     nm multi_file_main.o
 *       → Shows add, multiply, factorial as "U" (Undefined — needs linking)
 *       → Shows main as "T" (defined here)
 *
 *   After linking:
 *     nm multi_file
 *       → ALL symbols resolved, all have addresses
 */

/* We include our own header to ensure the declarations match the
 * definitions. If they don't match (e.g., wrong return type), the
 * compiler will catch the error HERE rather than at link time. */
#include "math_utils.h"

/*
 * add — Add two integers
 *
 * This is the DEFINITION: it provides the actual implementation.
 * The DECLARATION (prototype) in math_utils.h told other files
 * that this function exists and what its signature is.
 *
 * In the object file (math_utils.o), this function appears as a
 * symbol in the .text section with type "T" (defined text symbol).
 */
int add(int a, int b)
{
    return a + b;
}

/*
 * multiply — Multiply two integers
 *
 * Another simple function to demonstrate multi-file linking.
 * When multi_file_main.c calls multiply(), the compiler generates
 * a `call` instruction but the address is 0x00000000 (placeholder).
 * The linker fills in the real address from math_utils.o.
 */
int multiply(int a, int b)
{
    return a * b;
}

/*
 * factorial — Compute n! recursively
 *
 * This recursive function is more interesting in the assembly output:
 *   - It calls itself, so you'll see a `call factorial` instruction.
 *   - It uses the stack to save state between recursive calls.
 *   - The compiler may optimize this into a loop (tail call optimization)
 *     at higher optimization levels (-O2, -O3).
 *
 * Try comparing the assembly at different optimization levels:
 *   gcc -S -O0 math_utils.c -o math_utils_O0.s
 *   gcc -S -O2 math_utils.c -o math_utils_O2.s
 *   diff math_utils_O0.s math_utils_O2.s
 */
unsigned long factorial(unsigned int n)
{
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}
