/*
 * Module 1: Execution Models — C (Native Compiled Language)
 * =========================================================
 *
 * WHAT THIS FILE DEMONSTRATES:
 *   C compiles directly to native machine code. When you run the resulting
 *   binary, your CPU executes the instructions directly — there is no
 *   virtual machine, no interpreter, and no runtime environment (beyond
 *   the OS and the C runtime library, libc).
 *
 * THE COMPILATION MODEL:
 *   hello.c  →  [gcc/clang]  →  hello  (ELF binary with x86-64 machine code)
 *
 *   1. Preprocessing: #include directives are expanded, macros replaced.
 *   2. Compilation:   C source is translated to assembly (x86-64 .s file).
 *   3. Assembly:      The assembler converts .s → .o (object file).
 *   4. Linking:       The linker combines .o files with libc → executable ELF.
 *
 * WHAT TO OBSERVE:
 *   - Compile:   gcc -Wall -Wextra -pedantic -o hello hello.c
 *   - Run:       ./hello
 *   - Inspect:   file hello        → shows "ELF 64-bit LSB executable"
 *   - Inspect:   ldd hello         → shows libc.so dependency (dynamic linking)
 *   - Inspect:   objdump -d hello  → shows actual x86-64 machine instructions
 *   - Size:      ls -la hello      → typically ~16 KB (very small!)
 *
 * KEY INSIGHT:
 *   The CPU runs this program's instructions directly. There is no
 *   "interpreter" or "VM" sitting between your code and the hardware.
 *   This is why C is used for operating systems, embedded systems, and
 *   performance-critical software.
 *
 * COMPARE WITH:
 *   - Ruby:  Interpreted by MRI, compiled to YARV bytecode at runtime
 *   - Java:  Compiled to JVM bytecode, then interpreted/JIT-compiled
 *   - Rust:  Also native compiled (via LLVM), similar to C
 */

#include <stdio.h>

int main(void)
{
    printf("Hello from C!\n");
    printf("\n");
    printf("Execution model: NATIVE COMPILED\n");
    printf("  - This binary contains raw machine code for your CPU.\n");
    printf("  - No interpreter. No virtual machine. No runtime.\n");
    printf("  - The kernel loaded this ELF binary into memory and\n");
    printf("    jumped directly to its entry point.\n");
    printf("\n");
    printf("Try these commands to explore:\n");
    printf("  file ./hello           # See that it's an ELF binary\n");
    printf("  ldd ./hello            # See its shared library dependencies\n");
    printf("  objdump -d ./hello     # See the actual x86-64 instructions\n");
    printf("  readelf -h ./hello     # See the ELF header\n");
    printf("  strace ./hello         # See every system call it makes\n");

    return 0;
}
