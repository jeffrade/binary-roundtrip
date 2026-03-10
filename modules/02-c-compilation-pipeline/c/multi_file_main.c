/*
 * Module 2: C Compilation Pipeline — Multi-file Project (Main Program)
 * ====================================================================
 *
 * WHAT THIS FILE DEMONSTRATES:
 *   This is the main program for the multi-file project. It uses functions
 *   defined in math_utils.c by including math_utils.h. This demonstrates
 *   WHY linking is a separate step in the compilation pipeline.
 *
 * THE COMPILATION PROCESS FOR MULTI-FILE PROJECTS:
 *
 *   Step 1: Compile each .c file independently (separate translation units)
 *
 *     gcc -c multi_file_main.c    →  multi_file_main.o
 *       This file CALLS add(), multiply(), factorial() but doesn't
 *       DEFINE them. The object file has "undefined symbol" placeholders.
 *
 *     gcc -c math_utils.c         →  math_utils.o
 *       This file DEFINES add(), multiply(), factorial().
 *       The object file has these as "defined symbols."
 *
 *   Step 2: Link the object files together
 *
 *     gcc -o multi_file multi_file_main.o math_utils.o
 *       The linker:
 *       - Scans multi_file_main.o: "I need add, multiply, factorial"
 *       - Scans math_utils.o: "I have add, multiply, factorial"
 *       - Connects the references to the definitions
 *       - Also links with libc for printf
 *       - Produces the final ELF executable
 *
 * WHAT TO OBSERVE:
 *   Run `make multi` in the module directory to see this process with
 *   explanatory output, including nm output showing symbols.
 *
 * WHY SEPARATE COMPILATION MATTERS:
 *   - In a large project (Linux kernel: 30+ million lines of C), you don't
 *     want to recompile everything when you change one file.
 *   - With separate compilation, changing math_utils.c only requires:
 *     1. Recompile math_utils.c → math_utils.o  (fast)
 *     2. Relink all .o files                     (fast)
 *     No need to recompile multi_file_main.c!
 *   - This is exactly what `make` automates: it tracks dependencies and
 *     only rebuilds what changed.
 */

#include <stdio.h>

/* This #include is resolved during PREPROCESSING:
 * The entire contents of math_utils.h are pasted here.
 * After preprocessing, the compiler sees the function prototypes
 * for add(), multiply(), and factorial() right here. */
#include "math_utils.h"

int main(void)
{
    printf("=== Multi-file C Project Demo ===\n\n");

    /* These function calls compile to `call` instructions.
     * At compile time, the compiler trusts the declarations in math_utils.h
     * that these functions exist and have the specified signatures.
     * At link time, the linker resolves the actual addresses. */

    int a = 7, b = 5;

    printf("Testing math_utils functions:\n");
    printf("  add(%d, %d)       = %d\n", a, b, add(a, b));
    printf("  multiply(%d, %d)  = %d\n", a, b, multiply(a, b));
    printf("  factorial(10)     = %lu\n", factorial(10));
    printf("  factorial(20)     = %lu\n", factorial(20));
    printf("\n");

    /* Demonstrate that these are real function calls resolved at link time */
    printf("How this works:\n");
    printf("  1. This file (multi_file_main.c) was compiled to multi_file_main.o\n");
    printf("  2. math_utils.c was compiled to math_utils.o\n");
    printf("  3. The linker connected the call sites to the implementations.\n");
    printf("\n");
    printf("Verify with:\n");
    printf("  nm multi_file_main.o | grep -E 'add|multiply|factorial'\n");
    printf("    → Shows 'U' (Undefined) — this file uses but doesn't define them.\n");
    printf("  nm math_utils.o | grep -E 'add|multiply|factorial'\n");
    printf("    → Shows 'T' (Text) — this file defines them.\n");

    return 0;
}
