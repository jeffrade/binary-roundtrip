/*
 * calling_convention.c — Demonstrates the System V AMD64 ABI calling convention
 * ============================================================================
 *
 * MODULE 4: Assembly Fundamentals (x86-64, read-only goal)
 * EXERCISE: Understand how function arguments and return values are passed
 *
 * CONCEPT:
 *   The "calling convention" is the agreement between caller and callee about:
 *     1. WHERE arguments go (which registers, or on the stack)
 *     2. WHERE the return value goes (which register)
 *     3. WHO saves which registers (caller-saved vs callee-saved)
 *     4. HOW the stack is aligned (16-byte boundary before call)
 *
 *   On Linux x86-64, the System V AMD64 ABI dictates:
 *
 *   ARGUMENT PASSING (integer/pointer arguments):
 *     Arg 1 → rdi     Arg 2 → rsi     Arg 3 → rdx
 *     Arg 4 → rcx     Arg 5 → r8      Arg 6 → r9
 *     Arg 7+ → pushed onto the stack (right-to-left)
 *
 *   FLOATING-POINT ARGUMENTS:
 *     xmm0, xmm1, xmm2, ..., xmm7 (first 8 float/double args)
 *
 *   RETURN VALUE:
 *     Integer/pointer → rax (and rdx for 128-bit values)
 *     Float/double    → xmm0
 *
 *   CALLER-SAVED (volatile — caller must save if it needs them):
 *     rax, rcx, rdx, rsi, rdi, r8, r9, r10, r11
 *     The callee is FREE to destroy these.
 *
 *   CALLEE-SAVED (non-volatile — callee must preserve):
 *     rbx, rbp, r12, r13, r14, r15
 *     If the callee uses these, it must save and restore them.
 *
 * BUILD:
 *   gcc -Wall -Wextra -O0 -S -o calling_convention.s calling_convention.c
 *   gcc -Wall -Wextra -O0 -S -fverbose-asm -o calling_convention_verbose.s calling_convention.c
 *
 * WHAT TO OBSERVE:
 *   In the assembly output, look for:
 *   1. Arguments being placed in rdi, rsi, rdx, rcx, r8, r9
 *   2. The 7th+ arguments being pushed onto the stack
 *   3. Return values placed in eax/rax
 *   4. The 'call' instruction transferring control
 *   5. Stack alignment adjustments before calls
 */

#include <stdio.h>

/*
 * two_args — Takes 2 arguments: both fit in registers
 *
 * EXPECTED ASSEMBLY AT CALL SITE:
 *   mov    edi, 10      ; First arg → edi (lower 32 bits of rdi)
 *   mov    esi, 20      ; Second arg → esi (lower 32 bits of rsi)
 *   call   two_args     ; Jump to function, push return address
 *                        ; Result comes back in eax
 *
 * INSIDE THE FUNCTION (at -O0):
 *   The compiler stores register args to the stack (for debuggability):
 *   mov    DWORD PTR [rbp-4], edi   ; Save arg1 from register to stack
 *   mov    DWORD PTR [rbp-8], esi   ; Save arg2 from register to stack
 *   ... compute ...
 *   mov    eax, result              ; Place return value in eax
 *   ret
 */
int two_args(int a, int b) {
    return a + b;
}

/*
 * six_args — Takes exactly 6 arguments: all fit in registers!
 *
 * This is the maximum number of INTEGER arguments that can be passed
 * entirely in registers on System V AMD64. All six register slots are used.
 *
 * EXPECTED ASSEMBLY AT CALL SITE:
 *   mov    edi, 1       ; Arg 1 → rdi
 *   mov    esi, 2       ; Arg 2 → rsi
 *   mov    edx, 3       ; Arg 3 → rdx
 *   mov    ecx, 4       ; Arg 4 → rcx
 *   mov    r8d, 5       ; Arg 5 → r8
 *   mov    r9d, 6       ; Arg 6 → r9
 *   call   six_args
 *
 * Note: r8d and r9d are the lower 32-bit halves of r8 and r9.
 */
int six_args(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
}

/*
 * eight_args — Takes 8 arguments: 6 in registers + 2 on the stack!
 *
 * THIS IS THE KEY EXERCISE. When you have more than 6 integer arguments,
 * the extras go on the stack.
 *
 * EXPECTED ASSEMBLY AT CALL SITE:
 *   push   8            ; Arg 8 → stack (pushed first, right-to-left)
 *   push   7            ; Arg 7 → stack
 *   mov    r9d, 6       ; Arg 6 → r9
 *   mov    r8d, 5       ; Arg 5 → r8
 *   mov    ecx, 4       ; Arg 4 → rcx
 *   mov    edx, 3       ; Arg 3 → rdx
 *   mov    esi, 2       ; Arg 2 → rsi
 *   mov    edi, 1       ; Arg 1 → rdi
 *   call   eight_args
 *   add    rsp, 16      ; Clean up the 2 stack arguments (8 bytes each)
 *
 * OBSERVE: Args 7 and 8 are on the STACK, accessed relative to rbp:
 *   mov    eax, DWORD PTR [rbp+16]   ; Arg 7 (first stack arg)
 *   mov    eax, DWORD PTR [rbp+24]   ; Arg 8 (second stack arg)
 *
 * WHY +16? Because the stack at function entry has:
 *   [rbp+0]  = saved rbp (from push rbp)
 *   [rbp+8]  = return address (from call instruction)
 *   [rbp+16] = arg 7
 *   [rbp+24] = arg 8
 */
int eight_args(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Sum them all to force the compiler to actually use every argument */
    return a + b + c + d + e + f + g + h;
}

/*
 * returns_long — Returns a 64-bit value in rax (full register width)
 *
 * OBSERVE: When returning a 64-bit value, the compiler uses rax.
 * When returning a 32-bit value (int), it uses eax (lower half of rax).
 */
long returns_long(long x) {
    return x * 1000000L;
}

/*
 * callee_saved_demo — A function that uses callee-saved registers
 *
 * If the compiler needs more registers than the caller-saved set provides,
 * it will use callee-saved registers (rbx, r12-r15) and save/restore them.
 *
 * OBSERVE: At -O0, the compiler might not use callee-saved registers.
 * At higher optimization levels, you may see:
 *   push   rbx              ; Save callee-saved register
 *   ... (use rbx for computation) ...
 *   pop    rbx              ; Restore callee-saved register
 *   ret
 *
 * The push/pop of callee-saved registers is the function's OBLIGATION.
 * The caller trusts that these registers are unchanged after the call.
 */
long callee_saved_demo(long a, long b, long c, long d) {
    /* Use many intermediate values to encourage register usage */
    long sum = a + b;
    long product = c * d;
    long result = sum * product;
    result += a * c;
    result -= b * d;
    return result;
}

/*
 * variadic_example — printf is a variadic function
 *
 * Variadic functions (like printf) have a special rule:
 *   al (lowest byte of rax) must contain the number of vector (SSE)
 *   registers used for arguments. For printf with only integer args,
 *   al is set to 0.
 *
 * OBSERVE in the assembly before a printf call:
 *   mov    eax, 0        ; 0 floating-point args in SSE registers
 *   call   printf
 */

int main(void) {
    printf("=== Calling Convention Explorer ===\n\n");

    /* Two arguments: both in registers (edi, esi) */
    printf("two_args(10, 20) = %d\n", two_args(10, 20));

    /* Six arguments: all in registers (edi, esi, edx, ecx, r8d, r9d) */
    printf("six_args(1,2,3,4,5,6) = %d\n", six_args(1, 2, 3, 4, 5, 6));

    /* Eight arguments: 6 in registers + 2 ON THE STACK */
    printf("eight_args(1,2,3,4,5,6,7,8) = %d\n",
           eight_args(1, 2, 3, 4, 5, 6, 7, 8));

    /* 64-bit return value in rax */
    printf("returns_long(42) = %ld\n", returns_long(42));

    /* Many intermediates — may trigger callee-saved register usage */
    printf("callee_saved_demo(2,3,4,5) = %ld\n",
           callee_saved_demo(2, 3, 4, 5));

    printf("\n=== System V AMD64 ABI Summary ===\n\n");
    printf("  Argument Register Map (integer/pointer args):\n");
    printf("    Arg 1 → rdi (edi for 32-bit)\n");
    printf("    Arg 2 → rsi (esi for 32-bit)\n");
    printf("    Arg 3 → rdx (edx for 32-bit)\n");
    printf("    Arg 4 → rcx (ecx for 32-bit)\n");
    printf("    Arg 5 → r8  (r8d for 32-bit)\n");
    printf("    Arg 6 → r9  (r9d for 32-bit)\n");
    printf("    Arg 7+ → stack (right-to-left push order)\n\n");
    printf("  Return: rax (integer), xmm0 (float/double)\n\n");
    printf("  Caller-saved: rax, rcx, rdx, rsi, rdi, r8-r11\n");
    printf("  Callee-saved: rbx, rbp, r12-r15\n\n");

    printf("=== Exploration Commands ===\n\n");
    printf("  gcc -O0 -S -fverbose-asm -o calling_convention.s calling_convention.c\n");
    printf("  # Then open calling_convention.s and find each function.\n");
    printf("  # Look at how arguments move from registers to stack at -O0.\n\n");
    printf("  # Pay special attention to eight_args:\n");
    printf("  # - Args 1-6 arrive in registers\n");
    printf("  # - Args 7-8 are on the stack at [rbp+16] and [rbp+24]\n");

    return 0;
}
