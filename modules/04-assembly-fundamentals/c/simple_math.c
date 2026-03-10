/*
 * simple_math.c — Simple functions designed to produce readable assembly
 * ============================================================================
 *
 * MODULE 4: Assembly Fundamentals (x86-64, read-only goal)
 * EXERCISE: Map C constructs to their x86-64 assembly equivalents
 *
 * CONCEPT:
 *   Every C statement compiles to one or more assembly instructions.
 *   By writing simple functions and examining the output, you learn to
 *   READ disassembly — the goal is literacy, not writing assembly.
 *
 * BUILD:
 *   gcc -Wall -Wextra -O0 -S -o simple_math.s simple_math.c
 *     -O0  = no optimization (keeps assembly readable and predictable)
 *     -S   = stop at assembly (produce .s file instead of binary)
 *
 *   For even more readable output:
 *   gcc -Wall -Wextra -O0 -S -fverbose-asm -o simple_math_verbose.s simple_math.c
 *     -fverbose-asm = adds comments showing which C variable maps to which register
 *
 *   Compare with optimized:
 *   gcc -Wall -Wextra -O2 -S -o simple_math_opt.s simple_math.c
 *     The optimized version will be MUCH shorter — the compiler is very clever.
 *
 * QUICK REFERENCE — x86-64 Instructions you'll see:
 *   mov  rax, rbx    — Copy value from rbx to rax
 *   add  rax, rbx    — rax = rax + rbx
 *   sub  rax, rbx    — rax = rax - rbx
 *   imul rax, rbx    — rax = rax * rbx
 *   cmp  rax, rbx    — Compare rax and rbx (sets flags, no result stored)
 *   jg   label       — Jump to label if greater (based on flags from cmp)
 *   jle  label       — Jump to label if less or equal
 *   push rbx         — Push rbx onto the stack
 *   pop  rbx         — Pop top of stack into rbx
 *   call func        — Call a function (pushes return address, jumps)
 *   ret              — Return from function (pops return address, jumps)
 *
 * REGISTERS (System V AMD64 ABI — used on Linux):
 *   rdi = 1st argument    rsi = 2nd argument
 *   rdx = 3rd argument    rcx = 4th argument
 *   r8  = 5th argument    r9  = 6th argument
 *   rax = return value
 *   rsp = stack pointer   rbp = base pointer (frame pointer)
 */

#include <stdio.h>

/*
 * add — Returns a + b
 *
 * EXPECTED ASSEMBLY (at -O0):
 *   push   rbp               ; Save old base pointer (prologue)
 *   mov    rbp, rsp          ; Set up new stack frame
 *   mov    DWORD PTR [rbp-4], edi   ; Store arg 'a' (from edi) on stack
 *   mov    DWORD PTR [rbp-8], esi   ; Store arg 'b' (from esi) on stack
 *   mov    edx, DWORD PTR [rbp-4]   ; Load 'a' into edx
 *   mov    eax, DWORD PTR [rbp-8]   ; Load 'b' into eax
 *   add    eax, edx                 ; eax = eax + edx (a + b)
 *   pop    rbp               ; Restore old base pointer (epilogue)
 *   ret                      ; Return (result in eax)
 *
 * AT -O2 (optimized), this becomes just:
 *   lea    eax, [rdi+rsi]    ; eax = rdi + rsi (one instruction!)
 *   ret
 *
 * KEY INSIGHT: At -O0, the compiler stores arguments to the stack and
 * reloads them — this is wasteful but makes debugging easier (every
 * variable has a memory location you can inspect). At -O2, the compiler
 * knows the values are already in registers and uses them directly.
 *
 * NOTE: We use 'int' (32-bit), so the compiler uses edi/esi/eax (32-bit
 * register names) instead of rdi/rsi/rax (64-bit). The lower 32 bits of
 * a 64-bit register have their own name:
 *   rax (64-bit) → eax (lower 32-bit) → ax (lower 16-bit)
 */
int add(int a, int b) {
    return a + b;
}

/*
 * multiply — Returns a * b
 *
 * EXPECTED ASSEMBLY (at -O0):
 *   ... (prologue as above) ...
 *   mov    eax, DWORD PTR [rbp-4]   ; Load 'a'
 *   imul   eax, DWORD PTR [rbp-8]   ; eax = eax * b (signed multiply)
 *   ... (epilogue) ...
 *   ret
 *
 * NOTE: x86-64 uses 'imul' for signed integer multiplication.
 * There is no separate 'mul' instruction for general-purpose registers
 * in the modern two/three-operand form — 'imul' handles both.
 */
int multiply(int a, int b) {
    return a * b;
}

/*
 * max — Returns the larger of a and b
 *
 * This function uses a CONDITIONAL (if/else), which compiles to:
 *   1. cmp instruction — compare the two values (sets CPU flags)
 *   2. conditional jump — branch based on flags
 *
 * EXPECTED ASSEMBLY (at -O0):
 *   ... (prologue) ...
 *   mov    eax, DWORD PTR [rbp-4]   ; Load 'a'
 *   cmp    eax, DWORD PTR [rbp-8]   ; Compare a with b (sets flags)
 *   jle    .L_else                   ; If a <= b, jump to else branch
 *   mov    eax, DWORD PTR [rbp-4]   ; (then branch) result = a
 *   jmp    .L_end                    ; Skip else branch
 * .L_else:
 *   mov    eax, DWORD PTR [rbp-8]   ; (else branch) result = b
 * .L_end:
 *   ... (epilogue) ...
 *   ret
 *
 * AT -O2, the compiler may use cmov (conditional move) instead:
 *   cmp    edi, esi          ; Compare a and b
 *   mov    eax, esi          ; Assume result = b
 *   cmovge eax, edi          ; If a >= b, result = a (no branch!)
 *   ret
 *
 * KEY INSIGHT: cmov avoids a branch, which is better for the CPU's
 * branch predictor. Branch mispredictions are expensive (~15 cycles).
 */
int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

/*
 * subtract — Returns a - b
 *
 * EXPECTED ASSEMBLY: Uses 'sub' instruction.
 * Very similar to add, just with sub instead of add.
 */
int subtract(int a, int b) {
    return a - b;
}

/*
 * absolute_value — Returns |x|
 *
 * Interesting because it demonstrates negation (neg instruction)
 * combined with conditional logic.
 *
 * EXPECTED ASSEMBLY (at -O0):
 *   cmp    DWORD PTR [rbp-4], 0    ; Compare x with 0
 *   jns    .L_positive              ; If not negative (sign flag clear), skip
 *   neg    DWORD PTR [rbp-4]       ; Negate x (x = -x)
 * .L_positive:
 *   mov    eax, DWORD PTR [rbp-4]  ; result = x (now positive)
 */
int absolute_value(int x) {
    if (x < 0) {
        x = -x;
    }
    return x;
}

/*
 * main — Calls all functions and prints results
 *
 * Observe in the assembly: main calls each function using the 'call'
 * instruction. Before each call, it sets up arguments in registers:
 *   edi = first argument, esi = second argument
 *
 * After each call, the result is in eax, which main then passes
 * to printf (as the second argument, so it goes in esi).
 */
int main(void) {
    printf("=== Simple Math — Assembly Exploration ===\n\n");

    int a = 10, b = 3;

    printf("add(%d, %d) = %d\n", a, b, add(a, b));
    printf("subtract(%d, %d) = %d\n", a, b, subtract(a, b));
    printf("multiply(%d, %d) = %d\n", a, b, multiply(a, b));
    printf("max(%d, %d) = %d\n", a, b, max(a, b));
    printf("absolute_value(%d) = %d\n", -7, absolute_value(-7));

    printf("\n=== How to explore the assembly ===\n\n");
    printf("Generate assembly (no optimization):\n");
    printf("  gcc -O0 -S -o simple_math.s simple_math.c\n\n");
    printf("Generate verbose assembly (with C variable names as comments):\n");
    printf("  gcc -O0 -S -fverbose-asm -o simple_math_verbose.s simple_math.c\n\n");
    printf("Generate optimized assembly (compare to -O0!):\n");
    printf("  gcc -O2 -S -o simple_math_opt.s simple_math.c\n\n");
    printf("Disassemble the binary (objdump):\n");
    printf("  gcc -O0 -o simple_math simple_math.c && objdump -d simple_math\n\n");
    printf("Online: paste this code into https://godbolt.org/ for interactive exploration\n");

    return 0;
}
