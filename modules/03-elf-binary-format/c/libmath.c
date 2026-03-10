/*
 * libmath.c — Implementation of our custom shared library
 * ============================================================================
 *
 * MODULE 3: ELF Binary Format
 * EXERCISE: Build your own shared library (.so)
 *
 * This file implements the functions declared in libmath.h.
 * When compiled with -fPIC and linked with -shared, it becomes libmath.so.
 *
 * BUILD:
 *   gcc -Wall -Wextra -fPIC -c libmath.c -o libmath.o
 *   gcc -shared -o libmath.so libmath.o
 *
 * WHAT -fPIC DOES:
 *   Position Independent Code uses relative addressing instead of absolute.
 *   Without -fPIC, the code assumes it will be loaded at a fixed address.
 *   With -fPIC, all memory references are relative to the instruction pointer.
 *
 *   OBSERVE the difference:
 *     gcc -S libmath.c -o libmath_nopic.s           # Without PIC
 *     gcc -S -fPIC libmath.c -o libmath_pic.s       # With PIC
 *     diff libmath_nopic.s libmath_pic.s             # See the addressing changes
 *   You'll notice PIC code uses @PLT for function calls and
 *   RIP-relative addressing for global data access.
 *
 * WHAT TO OBSERVE IN THE SHARED LIBRARY:
 *   nm -D libmath.so          # Dynamic symbols (what other programs can use)
 *   nm libmath.so             # ALL symbols (including internal/static ones)
 *   readelf -S libmath.so     # Sections — note .plt, .got, .dynsym
 *   readelf -d libmath.so     # Dynamic section — SONAME, etc.
 *
 * IMPORTANT DISTINCTION — Two symbol tables in a shared library:
 *   .symtab  — Full symbol table (for debugging, stripped in production)
 *   .dynsym  — Dynamic symbol table (ALWAYS present, used by dynamic linker)
 *
 *   strip libmath.so              # Remove .symtab (debug symbols)
 *   nm libmath.so                 # Fails! .symtab is gone
 *   nm -D libmath.so              # Works! .dynsym is preserved
 *   readelf -s libmath.so         # Shows .dynsym still has our exports
 */

#include "libmath.h"

/*
 * A static (internal) helper function.
 * This will NOT appear in the dynamic symbol table (.dynsym).
 * It's private to this library — callers can't use it.
 *
 * OBSERVE: `nm libmath.so | grep internal_helper` → lowercase 't' (local)
 * OBSERVE: `nm -D libmath.so | grep internal_helper` → NOT FOUND
 *   The -D flag shows only .dynsym (exported symbols). Static functions
 *   are not exported.
 */
static long internal_helper_multiply(long a, long b) {
    return a * b;
}

/*
 * square — Returns x * x
 *
 * This is a global (exported) function. It WILL appear in .dynsym.
 * OBSERVE: `nm -D libmath.so | grep square` → 'T' (global text)
 */
long square(long x) {
    return internal_helper_multiply(x, x);
}

/*
 * cube — Returns x * x * x
 *
 * Note how it calls another exported function (square).
 * Within the same .so, this call may go through the PLT or be optimized
 * to a direct call depending on compiler settings.
 *
 * OBSERVE: `objdump -d libmath.so` and look at the cube function.
 *   See if it calls square@plt or calls square directly.
 */
long cube(long x) {
    return internal_helper_multiply(square(x), x);
}

/*
 * factorial — Iterative n!
 *
 * Uses a loop, which in the disassembly will show:
 *   - A comparison (cmp) to check loop condition
 *   - A conditional jump (jle/jge) to exit or continue
 *   - A multiply instruction (imul) in the loop body
 *   - An increment and jump back to the comparison
 */
long factorial(int n) {
    if (n < 0) {
        return -1;  /* Error: negative input */
    }

    long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

/*
 * fibonacci — Iterative Fibonacci
 *
 * The iterative approach avoids the exponential cost of the naive
 * recursive version. In the disassembly, you'll see:
 *   - Two "accumulator" variables in registers
 *   - A swap operation each iteration
 *   - A loop counter with comparison and conditional jump
 */
long fibonacci(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;

    long prev = 0;
    long curr = 1;

    for (int i = 2; i <= n; i++) {
        long next = prev + curr;
        prev = curr;
        curr = next;
    }
    return curr;
}

/*
 * is_prime — Primality test by trial division
 *
 * In the disassembly, look for:
 *   - The division instruction (div/idiv) for the modulo operation
 *   - The comparison with 0 to check if remainder == 0
 *   - The sqrt-like optimization (i*i <= n) as a multiply + compare
 */
int is_prime(int n) {
    if (n < 2) return 0;
    if (n < 4) return 1;
    if (n % 2 == 0) return 0;

    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}
