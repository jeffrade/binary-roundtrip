/*
 * Module 2: C Compilation Pipeline — Multi-file Project (Header File)
 * ====================================================================
 *
 * WHAT THIS FILE DEMONSTRATES:
 *   This is a HEADER FILE (.h). It contains DECLARATIONS (not definitions)
 *   of functions. Think of it as a "menu" — it tells other files what
 *   functions are available, but not how they work.
 *
 * WHY HEADER FILES EXIST:
 *   In C, each .c file is compiled independently (a "translation unit").
 *   When multi_file_main.c calls add(2, 3), the compiler needs to know:
 *     - Does add() exist?
 *     - What arguments does it take?
 *     - What does it return?
 *
 *   The header provides this information via FUNCTION DECLARATIONS
 *   (also called "function prototypes"). The actual implementation
 *   lives in math_utils.c and is connected during LINKING.
 *
 * INCLUDE GUARDS:
 *   The #ifndef / #define / #endif pattern prevents this header from
 *   being included twice in the same translation unit. Without this,
 *   if two headers both #include "math_utils.h", you'd get duplicate
 *   declaration errors.
 *
 *   Modern alternative: #pragma once (not standard C, but widely supported)
 *
 * THE PIPELINE:
 *   1. PREPROCESSING: #include "math_utils.h" in multi_file_main.c gets
 *      replaced with the entire contents of this file.
 *   2. COMPILATION: The compiler uses these declarations to type-check
 *      calls to add(), multiply(), factorial() in multi_file_main.c.
 *   3. ASSEMBLY: Function calls become `call` instructions with placeholder
 *      addresses (the assembler doesn't know where add() is yet).
 *   4. LINKING: The linker finds the actual implementations in math_utils.o
 *      and fills in the correct addresses.
 */

/* INCLUDE GUARD: If MATH_UTILS_H is not yet defined, define it and
 * include the contents. If it's already defined (this file was already
 * included), skip everything up to #endif. */
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

/*
 * FUNCTION DECLARATIONS (prototypes)
 *
 * These tell the compiler:
 *   - The function name
 *   - The parameter types
 *   - The return type
 *
 * They do NOT tell the compiler how the function works (that's in the .c file).
 * Notice: no function body, just a semicolon at the end.
 */

/* Add two integers and return the result.
 * Declaration only — implementation is in math_utils.c */
int add(int a, int b);

/* Multiply two integers and return the result.
 * Declaration only — implementation is in math_utils.c */
int multiply(int a, int b);

/* Compute n! (factorial) recursively.
 * Declaration only — implementation is in math_utils.c
 *
 * Note: This uses unsigned long to handle larger factorials,
 * but will still overflow for n > 20 on 64-bit systems. */
unsigned long factorial(unsigned int n);

#endif /* MATH_UTILS_H — end of include guard */
