/*
 * Module 2: C Compilation Pipeline — Hello World (All Stages)
 * ============================================================
 *
 * This file is designed to show interesting things at every stage of the
 * C compilation pipeline:
 *
 *   STAGE 1 — PREPROCESSING (gcc -E):
 *     - The #include <stdio.h> will expand to hundreds of lines of
 *       declarations, typedefs, and function prototypes.
 *     - The #define GREETING macro will be replaced with its value.
 *     - Try: gcc -E hello.c | wc -l   (you'll see ~700+ lines!)
 *
 *   STAGE 2 — COMPILATION (gcc -S):
 *     - The compiler translates C into assembly language.
 *     - You'll see the string "Hello from C!" in a .rodata section.
 *     - You'll see a `call printf` or `call puts` instruction.
 *     - Try: gcc -S hello.c && cat hello.s
 *
 *   STAGE 3 — ASSEMBLY (gcc -c):
 *     - The assembler converts assembly to machine code in an object file.
 *     - The object file is NOT yet an executable (it has unresolved symbols).
 *     - Try: gcc -c hello.c && file hello.o && nm hello.o
 *       (nm shows symbols; printf will appear as "U" = undefined)
 *
 *   STAGE 4 — LINKING (gcc -o hello hello.o):
 *     - The linker resolves all symbols (connects our printf call to libc).
 *     - Produces the final ELF executable.
 *     - Try: gcc -o hello hello.o && file hello && nm hello | grep printf
 *
 * WHAT TO OBSERVE:
 *   Run `make pipeline` in the module directory to see all four stages
 *   with explanatory output.
 */

/* PREPROCESSING: This #include pulls in the entire stdio.h header.
 * After preprocessing, this single line becomes ~700+ lines of
 * declarations for FILE, printf, scanf, fopen, etc. */
#include <stdio.h>

/* PREPROCESSING: This #define creates a text substitution macro.
 * Every occurrence of GREETING in the code below will be replaced
 * with the string literal before compilation even begins. */
#define GREETING "Hello from C's compilation pipeline!"

/* COMPILATION: This function will appear in the assembly output as a
 * label followed by instructions. The string literal will be placed
 * in the .rodata (read-only data) section of the assembly. */
int main(void)
{
    /* COMPILATION: This printf call becomes a `call` instruction in assembly.
     * LINKING: The linker must resolve `printf` to its address in libc. */
    printf("%s\n", GREETING);

    printf("\nThis file demonstrates all 4 stages of C compilation:\n");
    printf("  1. Preprocessing: #include expanded, #define replaced\n");
    printf("  2. Compilation:   C source → assembly (hello.s)\n");
    printf("  3. Assembly:      Assembly → object file (hello.o)\n");
    printf("  4. Linking:       Object file → executable (hello)\n");

    return 0;
}
