/*
 * libmath.h — Header for our custom shared library
 * ============================================================================
 *
 * MODULE 3: ELF Binary Format
 * EXERCISE: Build your own shared library (.so)
 *
 * CONCEPT:
 *   A shared library is an ELF file of type ET_DYN (shared object).
 *   It contains compiled code and data that can be loaded by multiple
 *   programs at runtime. This header declares the public API.
 *
 * WHAT IS A HEADER FILE?
 *   The .h file is a CONTRACT — it tells callers what functions exist
 *   and what their signatures are, without revealing implementation.
 *   At the ELF level, these function names become GLOBAL symbols in
 *   the .dynsym (dynamic symbol table) of the resulting .so file.
 *
 * BUILD THE SHARED LIBRARY:
 *   gcc -Wall -Wextra -fPIC -c libmath.c -o libmath.o   # Compile to PIC object
 *   gcc -shared -o libmath.so libmath.o                   # Create shared library
 *
 * EXPLORE THE LIBRARY:
 *   file libmath.so                   # "ELF 64-bit shared object"
 *   readelf -h libmath.so             # Type: DYN (Shared object file)
 *   readelf -d libmath.so             # Dynamic section
 *   nm -D libmath.so                  # Dynamic symbols (our exported functions)
 *   objdump -d libmath.so             # Disassemble the library code
 *
 * KEY CONCEPTS:
 *   -fPIC (Position Independent Code):
 *     The library can be loaded at ANY address in memory. All internal
 *     references use relative addressing (RIP-relative on x86-64).
 *     This is essential for shared libraries because different programs
 *     may load the library at different virtual addresses.
 *
 *   -shared:
 *     Tells the linker to create a shared object (ET_DYN) instead of
 *     an executable (ET_EXEC).
 */

#ifndef LIBMATH_H
#define LIBMATH_H

/*
 * square — Returns x * x
 *
 * OBSERVE: After building, `nm -D libmath.so | grep square`
 *   Shows: T square (global text symbol — exported)
 */
long square(long x);

/*
 * cube — Returns x * x * x
 */
long cube(long x);

/*
 * factorial — Returns n! (n factorial)
 *
 * Uses iterative computation. Returns -1 for negative input.
 * Note: long overflows for n > 20 on 64-bit systems.
 */
long factorial(int n);

/*
 * fibonacci — Returns the nth Fibonacci number
 *
 * Uses iterative computation. F(0)=0, F(1)=1, F(n)=F(n-1)+F(n-2)
 */
long fibonacci(int n);

/*
 * is_prime — Returns 1 if n is prime, 0 otherwise
 */
int is_prime(int n);

#endif /* LIBMATH_H */
