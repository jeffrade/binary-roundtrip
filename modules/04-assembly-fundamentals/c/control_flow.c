/*
 * control_flow.c — Maps C control structures to assembly patterns
 * ============================================================================
 *
 * MODULE 4: Assembly Fundamentals (x86-64, read-only goal)
 * EXERCISE: Recognize control flow patterns in disassembly
 *
 * CONCEPT:
 *   High-level control structures (if/else, for, while, switch) all compile
 *   down to just TWO assembly primitives:
 *     1. cmp/test — compare values (sets CPU flags)
 *     2. jCC      — conditional jump (branch based on flags)
 *
 *   That's it. Every if/else, every loop, every switch — all built from
 *   compare + conditional jump. Once you recognize these patterns, you can
 *   read disassembly of any C program.
 *
 * BUILD:
 *   gcc -Wall -Wextra -O0 -S -o control_flow.s control_flow.c
 *   gcc -Wall -Wextra -O0 -S -fverbose-asm -o control_flow_verbose.s control_flow.c
 *
 * CONDITIONAL JUMP INSTRUCTIONS (jCC):
 *   je  / jz    — Jump if Equal / Zero             (ZF=1)
 *   jne / jnz   — Jump if Not Equal / Not Zero     (ZF=0)
 *   jg  / jnle  — Jump if Greater (signed)         (ZF=0 and SF=OF)
 *   jge / jnl   — Jump if Greater or Equal (signed) (SF=OF)
 *   jl  / jnge  — Jump if Less (signed)            (SF!=OF)
 *   jle / jng   — Jump if Less or Equal (signed)   (ZF=1 or SF!=OF)
 *   ja          — Jump if Above (unsigned)          (CF=0 and ZF=0)
 *   jb          — Jump if Below (unsigned)          (CF=1)
 *
 * FLAGS REGISTER:
 *   The CPU has a FLAGS register with individual bits:
 *     ZF (Zero Flag)  — set when result is zero
 *     SF (Sign Flag)  — set when result is negative
 *     CF (Carry Flag) — set on unsigned overflow
 *     OF (Overflow Flag) — set on signed overflow
 *
 *   'cmp a, b' is really 'sub a, b' but the result is discarded;
 *   only the flags are kept. This is why cmp + jg means "if a > b".
 */

#include <stdio.h>

/*
 * PATTERN 1: if/else → cmp + conditional jump
 * ============================================================================
 *
 * EXPECTED ASSEMBLY (at -O0):
 *   mov    eax, DWORD PTR [rbp-4]    ; Load 'x' into eax
 *   cmp    eax, 0                     ; Compare x with 0
 *   jle    .L_not_positive            ; If x <= 0, jump to else branch
 *   ... (positive branch code) ...    ; "positive" path
 *   jmp    .L_endif                   ; Skip else branch
 * .L_not_positive:
 *   cmp    eax, 0                     ; Compare x with 0 again
 *   jne    .L_negative                ; If x != 0, jump to negative branch
 *   ... (zero branch code) ...        ; "zero" path
 *   jmp    .L_endif
 * .L_negative:
 *   ... (negative branch code) ...    ; "negative" path
 * .L_endif:
 *
 * KEY INSIGHT: The C code reads top-to-bottom, but the assembly JUMPS.
 *   The "if" condition is INVERTED: if (x > 0) in C becomes jle .L_else
 *   in assembly. The compiler inverts the condition to jump OVER the
 *   "then" branch when the condition is false.
 */
const char *classify_number(int x) {
    if (x > 0) {
        return "positive";
    } else if (x == 0) {
        return "zero";
    } else {
        return "negative";
    }
}

/*
 * PATTERN 2: for loop → initialize, compare, body, increment, jump back
 * ============================================================================
 *
 * A for loop: for (i = 0; i < n; i++) { body; }
 * Compiles to:
 *
 *   mov    DWORD PTR [rbp-4], 0       ; i = 0 (initialization)
 *   jmp    .L_check                    ; Jump to condition check
 * .L_body:
 *   ... (loop body) ...               ; The actual work
 *   add    DWORD PTR [rbp-4], 1       ; i++ (increment)
 * .L_check:
 *   mov    eax, DWORD PTR [rbp-4]     ; Load i
 *   cmp    eax, DWORD PTR [rbp-8]     ; Compare i with n
 *   jl     .L_body                     ; If i < n, jump back to body
 *
 * AT -O2, the compiler may:
 *   1. Keep 'i' in a register (never store to stack)
 *   2. Unroll the loop (repeat body 2-4 times per iteration)
 *   3. Vectorize with SIMD (process 4+ elements per iteration)
 *   4. Completely eliminate the loop (compute result directly!)
 */
int sum_range(int n) {
    int total = 0;
    for (int i = 1; i <= n; i++) {
        total += i;
    }
    return total;
    /*
     * AT -O2, the compiler may recognize this is n*(n+1)/2 and emit:
     *   lea    eax, [rdi+1]     ; eax = n + 1
     *   imul   eax, edi         ; eax = n * (n + 1)
     *   shr    eax, 1           ; eax = eax / 2
     *   ret
     * No loop at all! The compiler is smarter than you might expect.
     */
}

/*
 * PATTERN 3: while loop → condition check, body, jump back
 * ============================================================================
 *
 * Similar to a for loop but with the check at the top:
 *
 * .L_check:
 *   cmp    DWORD PTR [rbp-4], 1       ; Compare n with 1
 *   jle    .L_done                     ; If n <= 1, exit loop
 *   ... (loop body — n = n / 2 or n = 3*n + 1) ...
 *   jmp    .L_check                    ; Jump back to check
 * .L_done:
 */
int collatz_steps(int n) {
    /*
     * Collatz conjecture: Starting from any positive integer:
     *   - If even: n = n / 2
     *   - If odd:  n = 3n + 1
     *   Eventually reaches 1. (Unproven, but true for all tested numbers!)
     *
     * OBSERVE in the assembly:
     *   - The even/odd check uses 'test eax, 1' (faster than 'and' for bit 0)
     *     or 'and eax, 1' followed by 'je' (jump if even)
     *   - The division by 2 uses 'sar eax, 1' (shift right — faster than div!)
     *   - The multiply by 3 may use 'lea eax, [rax+rax*2]' (faster than imul!)
     */
    int steps = 0;
    while (n > 1) {
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
        steps++;
    }
    return steps;
}

/*
 * PATTERN 4: switch statement → jump table or chain of comparisons
 * ============================================================================
 *
 * A switch with sequential cases (0, 1, 2, 3, ...) compiles to a JUMP TABLE:
 *
 *   cmp    edi, 4                      ; Is n > 4? (number of cases - 1)
 *   ja     .L_default                  ; If above, go to default
 *   lea    rdx, .L_jumptable[rip]      ; Address of jump table
 *   movsxd rax, DWORD PTR [rdx+rdi*4] ; Load offset for case n
 *   add    rax, rdx                    ; Compute target address
 *   jmp    rax                         ; Jump to the right case!
 *
 * The jump table is stored in .rodata as an array of offsets:
 *   .L_jumptable: .long .L_case0-.L_jumptable
 *                 .long .L_case1-.L_jumptable
 *                 .long .L_case2-.L_jumptable
 *                 ...
 *
 * WHY a jump table? It's O(1) — constant time regardless of case count.
 * An if/else chain would be O(n) — check each condition one by one.
 *
 * NOTE: The compiler uses a jump table when cases are DENSE (sequential or
 * near-sequential). For SPARSE cases (like 1, 100, 10000), it uses a chain
 * of comparisons or a binary search.
 */
const char *day_name(int day) {
    switch (day) {
        case 0: return "Sunday";
        case 1: return "Monday";
        case 2: return "Tuesday";
        case 3: return "Wednesday";
        case 4: return "Thursday";
        case 5: return "Friday";
        case 6: return "Saturday";
        default: return "Invalid";
    }
}

/*
 * PATTERN 5: switch with sparse cases → comparison chain (no jump table)
 * ============================================================================
 *
 * When cases are not sequential, the compiler falls back to comparisons:
 *
 *   cmp    edi, 100
 *   je     .L_case_100
 *   cmp    edi, 200
 *   je     .L_case_200
 *   cmp    edi, 404
 *   je     .L_case_404
 *   jmp    .L_default
 *
 * OBSERVE: Compare the assembly of day_name (dense → jump table) vs
 *   http_status_category (sparse → comparison chain). The compiler
 *   chooses the best strategy automatically.
 */
const char *http_status_category(int code) {
    switch (code) {
        case 200: return "OK";
        case 301: return "Moved Permanently";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}

/*
 * PATTERN 6: Ternary operator → same as if/else (or cmov)
 * ============================================================================
 *
 * The ternary operator (a ? b : c) compiles to the SAME assembly as
 * if/else. There's no special instruction for it.
 *
 * At -O2, the compiler may use cmov (conditional move):
 *   cmp    edi, esi
 *   mov    eax, esi       ; Assume result = b
 *   cmovl  eax, edi       ; If a < b, result = a
 *   ret
 */
int min_val(int a, int b) {
    return (a < b) ? a : b;
}

/*
 * PATTERN 7: do-while → body first, then check
 * ============================================================================
 *
 * A do-while puts the condition check at the BOTTOM:
 *
 * .L_body:
 *   ... (loop body) ...
 *   cmp    eax, target
 *   jne    .L_body          ; If not done, jump back to body
 *
 * This is actually the most "natural" loop for the CPU — there's only
 * ONE branch instruction (at the bottom). For loops and while loops
 * have TWO (an initial jump to the condition, plus the loop-back jump).
 */
int count_digits(int n) {
    if (n < 0) n = -n;
    if (n == 0) return 1;

    int count = 0;
    do {
        n /= 10;
        count++;
    } while (n > 0);

    return count;
}

int main(void) {
    printf("=== Control Flow in Assembly ===\n\n");

    /* Pattern 1: if/else → cmp + conditional jump */
    printf("--- Pattern 1: if/else (cmp + jCC) ---\n");
    printf("classify_number(42)  = %s\n", classify_number(42));
    printf("classify_number(0)   = %s\n", classify_number(0));
    printf("classify_number(-7)  = %s\n\n", classify_number(-7));

    /* Pattern 2: for loop → init, check, body, increment, jump back */
    printf("--- Pattern 2: for loop (cmp + jl + body + jmp) ---\n");
    printf("sum_range(10)  = %d (expected: 55)\n", sum_range(10));
    printf("sum_range(100) = %d (expected: 5050)\n\n", sum_range(100));

    /* Pattern 3: while loop → check at top, body, jump back */
    printf("--- Pattern 3: while loop (with if/else inside) ---\n");
    printf("collatz_steps(7)   = %d steps to reach 1\n", collatz_steps(7));
    printf("collatz_steps(27)  = %d steps to reach 1\n\n", collatz_steps(27));

    /* Pattern 4: switch (dense) → jump table */
    printf("--- Pattern 4: switch/dense (jump table) ---\n");
    for (int d = 0; d <= 6; d++) {
        printf("day_name(%d) = %s\n", d, day_name(d));
    }
    printf("\n");

    /* Pattern 5: switch (sparse) → comparison chain */
    printf("--- Pattern 5: switch/sparse (comparison chain) ---\n");
    int codes[] = {200, 301, 404, 500, 999};
    for (int i = 0; i < 5; i++) {
        printf("http_status_category(%d) = %s\n", codes[i],
               http_status_category(codes[i]));
    }
    printf("\n");

    /* Pattern 6: ternary → cmov at -O2 */
    printf("--- Pattern 6: ternary operator (cmov at -O2) ---\n");
    printf("min_val(3, 7) = %d\n", min_val(3, 7));
    printf("min_val(9, 2) = %d\n\n", min_val(9, 2));

    /* Pattern 7: do-while → condition at bottom */
    printf("--- Pattern 7: do-while (condition at bottom of loop) ---\n");
    printf("count_digits(0)     = %d\n", count_digits(0));
    printf("count_digits(42)    = %d\n", count_digits(42));
    printf("count_digits(12345) = %d\n", count_digits(12345));

    printf("\n=== Assembly Pattern Summary ===\n\n");
    printf("  C construct    | Assembly pattern\n");
    printf("  ---------------|------------------------------------------\n");
    printf("  if (cond)      | cmp + jCC_inverse over 'then' block\n");
    printf("  if/else        | cmp + jCC_inverse + then + jmp + else\n");
    printf("  for loop       | init + jmp check + body + incr + check\n");
    printf("  while loop     | jmp check + body + check\n");
    printf("  do-while       | body + check (condition at bottom)\n");
    printf("  switch/dense   | cmp range + jump table lookup\n");
    printf("  switch/sparse  | cmp chain (one cmp+je per case)\n");
    printf("  ternary        | cmp + jCC or cmov (at -O2)\n");

    printf("\n=== Exploration Commands ===\n\n");
    printf("  gcc -O0 -S -fverbose-asm -o control_flow.s control_flow.c\n");
    printf("  # Examine each function — identify the patterns above.\n\n");
    printf("  gcc -O2 -S -o control_flow_opt.s control_flow.c\n");
    printf("  # Compare: How does the optimizer transform loops and branches?\n");
    printf("  # Look for: cmov replacing branches, loop unrolling,\n");
    printf("  #   arithmetic shortcuts (n*(n+1)/2 instead of a loop).\n");

    return 0;
}
