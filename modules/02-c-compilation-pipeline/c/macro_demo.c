/*
 * Module 2: C Compilation Pipeline — Macro & Preprocessing Demo
 * ==============================================================
 *
 * WHAT THIS FILE DEMONSTRATES:
 *   The C preprocessor is a text transformation engine that runs BEFORE
 *   the actual compiler ever sees your code. It handles:
 *     - #include   (file inclusion — literally pastes file contents)
 *     - #define    (text substitution macros)
 *     - #ifdef     (conditional compilation)
 *     - __FILE__   (predefined macros — filled in by the preprocessor)
 *
 * HOW TO EXPLORE:
 *   1. Without DEBUG:
 *      gcc -E macro_demo.c | tail -80
 *      → See the preprocessed output. DEBUG blocks are stripped out.
 *
 *   2. With DEBUG:
 *      gcc -E -DDEBUG macro_demo.c | tail -80
 *      → The -D flag defines DEBUG, so debug code is included.
 *
 *   3. Compile and run both ways:
 *      gcc -Wall -Wextra -pedantic -o macro_demo macro_demo.c
 *      ./macro_demo                        ← no debug output
 *
 *      gcc -Wall -Wextra -pedantic -DDEBUG -o macro_demo_debug macro_demo.c
 *      ./macro_demo_debug                  ← debug output included
 *
 * KEY INSIGHT:
 *   The preprocessor is a purely textual tool. It doesn't understand C
 *   syntax — it just does search-and-replace, file inclusion, and
 *   conditional text removal. The actual C compiler never sees any
 *   #directives — they're all resolved before compilation begins.
 *
 *   Run `make macros` to see this in action.
 */

#include <stdio.h>

/* =========================================================================
 * OBJECT-LIKE MACROS (simple text substitution)
 * =========================================================================
 * These are replaced by the preprocessor before compilation.
 * After preprocessing, MAX_STUDENTS becomes 30, PI becomes 3.14159, etc.
 * The compiler never sees the names MAX_STUDENTS or PI.
 */
#define MAX_STUDENTS  30
#define PI            3.14159
#define COURSE_NAME   "Source to Running Binary"

/* =========================================================================
 * FUNCTION-LIKE MACROS (parameterized text substitution)
 * =========================================================================
 * These look like function calls but are text substitutions.
 * SQUARE(5)     becomes ((5) * (5))
 * SQUARE(x+1)   becomes ((x+1) * (x+1))   ← parentheses prevent bugs!
 * MAX(a, b)     becomes ((a) > (b) ? (a) : (b))
 *
 * WARNING: Without the extra parentheses, SQUARE(x+1) would become
 *          (x+1 * x+1) = (x + x + 1) which is WRONG.
 *          Always parenthesize macro arguments!
 */
#define SQUARE(x)     ((x) * (x))
#define MAX(a, b)     ((a) > (b) ? (a) : (b))
#define MIN(a, b)     ((a) < (b) ? (a) : (b))

/* =========================================================================
 * MULTI-LINE MACRO (using backslash continuation)
 * =========================================================================
 * The backslash at end of line continues the macro to the next line.
 * This is still just text substitution, but across multiple lines.
 */
#define PRINT_HEADER(title) do {                           \
    printf("============================================\n"); \
    printf(" %s\n", (title));                                 \
    printf("============================================\n"); \
} while (0)

/* =========================================================================
 * CONDITIONAL COMPILATION (#ifdef / #ifndef / #if)
 * =========================================================================
 * These directives include or exclude code at preprocessing time.
 * If DEBUG is not defined, the preprocessor REMOVES the debug code entirely.
 * The compiler never sees it. It's as if it was never written.
 */

/* DEBUG_PRINT: only generates code if DEBUG is defined.
 * If DEBUG is not defined, this macro expands to nothing. */
#ifdef DEBUG
    #define DEBUG_PRINT(fmt, ...) \
        printf("[DEBUG %s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) /* nothing — removed by preprocessor */
#endif

/* =========================================================================
 * INCLUDE GUARDS (standard pattern, shown for education)
 * =========================================================================
 * In header files, you use #ifndef to prevent double inclusion:
 *   #ifndef MY_HEADER_H
 *   #define MY_HEADER_H
 *   ... declarations ...
 *   #endif
 * This file isn't a header, but the pattern is shown in math_utils.h.
 */

/* =========================================================================
 * STRINGIFICATION AND TOKEN PASTING (advanced macros)
 * =========================================================================
 * # (stringification): Converts a macro argument to a string literal.
 *   STRINGIFY(hello)  →  "hello"
 *
 * ## (token pasting): Concatenates two tokens into one.
 *   CONCAT(var, 1)  →  var1
 */
#define STRINGIFY(x)    #x
#define CONCAT(a, b)    a ## b

int main(void)
{
    /* =====================================================================
     * PREDEFINED MACROS
     * =====================================================================
     * The preprocessor automatically defines several macros:
     *   __FILE__     → The current source file name (string)
     *   __LINE__     → The current line number (integer)
     *   __DATE__     → The compilation date (string, "Mar 10 2026")
     *   __TIME__     → The compilation time (string, "14:30:00")
     *   __STDC__     → 1 if the compiler conforms to ANSI C
     *
     * These are filled in by the preprocessor, not at runtime.
     * The compiled binary contains the literal date/time of compilation.
     */
    PRINT_HEADER("Predefined Macros");
    printf("__FILE__ = %s\n", __FILE__);
    printf("__LINE__ = %d\n", __LINE__);
    printf("__DATE__ = %s\n", __DATE__);
    printf("__TIME__ = %s\n", __TIME__);
    printf("__STDC__ = %d\n", __STDC__);
    printf("\n");

    /* =====================================================================
     * OBJECT-LIKE MACROS in action
     * =====================================================================
     * After preprocessing, this code has no macro names — just literal values.
     * MAX_STUDENTS → 30, PI → 3.14159, COURSE_NAME → "Source to Running Binary"
     */
    PRINT_HEADER("Object-like Macros");
    printf("MAX_STUDENTS = %d\n", MAX_STUDENTS);
    printf("PI           = %.5f\n", PI);
    printf("COURSE_NAME  = \"%s\"\n", COURSE_NAME);
    printf("\n");

    /* =====================================================================
     * FUNCTION-LIKE MACROS in action
     * =====================================================================
     * SQUARE(5) is replaced with ((5) * (5)) by the preprocessor.
     * The compiler sees ((5) * (5)) and evaluates it at compile time!
     */
    PRINT_HEADER("Function-like Macros");
    printf("SQUARE(5)    = %d\n", SQUARE(5));
    printf("SQUARE(2+3)  = %d  (parentheses matter!)\n", SQUARE(2 + 3));
    printf("MAX(10, 20)  = %d\n", MAX(10, 20));
    printf("MIN(10, 20)  = %d\n", MIN(10, 20));
    printf("\n");

    /* =====================================================================
     * STRINGIFICATION in action
     * =====================================================================
     * STRINGIFY(hello_world) → "hello_world" (the argument becomes a string)
     */
    PRINT_HEADER("Stringification (#) and Token Pasting (##)");
    printf("STRINGIFY(hello_world) = \"%s\"\n", STRINGIFY(hello_world));
    printf("STRINGIFY(42)          = \"%s\"\n", STRINGIFY(42));

    /* Token pasting: CONCAT(my_var, _1) becomes my_var_1 */
    int CONCAT(my_var, _1) = 100;
    int CONCAT(my_var, _2) = 200;
    printf("CONCAT(my_var, _1) created variable my_var_1 = %d\n", my_var_1);
    printf("CONCAT(my_var, _2) created variable my_var_2 = %d\n", my_var_2);
    printf("\n");

    /* =====================================================================
     * CONDITIONAL COMPILATION (#ifdef DEBUG)
     * =====================================================================
     * If DEBUG is defined (compile with -DDEBUG), the debug prints appear.
     * If not, they're completely removed — zero runtime overhead.
     */
    PRINT_HEADER("Conditional Compilation");
#ifdef DEBUG
    printf("DEBUG mode is ON  (compiled with -DDEBUG)\n");
#else
    printf("DEBUG mode is OFF (compile with -DDEBUG to enable)\n");
#endif

    /* These calls expand to printf() if DEBUG is defined,
     * or to nothing if DEBUG is not defined. */
    DEBUG_PRINT("This message only appears in debug mode");
    DEBUG_PRINT("Variable values: MAX_STUDENTS=%d, PI=%.2f", MAX_STUDENTS, PI);

    printf("\n");
    printf("To see the preprocessing output:\n");
    printf("  gcc -E macro_demo.c | tail -80          (without DEBUG)\n");
    printf("  gcc -E -DDEBUG macro_demo.c | tail -80  (with DEBUG)\n");
    printf("\n");
    printf("Key insight: The preprocessor is pure text manipulation.\n");
    printf("It runs BEFORE the C compiler. After preprocessing, there\n");
    printf("are no #directives left — just plain C code.\n");

    return 0;
}
