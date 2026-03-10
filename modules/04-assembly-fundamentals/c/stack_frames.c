/*
 * stack_frames.c — Demonstrates stack frame mechanics
 * ============================================================================
 *
 * MODULE 4: Assembly Fundamentals (x86-64, read-only goal)
 * EXERCISE: Understand how the stack grows and shrinks with function calls
 *
 * CONCEPT:
 *   Every function call creates a STACK FRAME — a region of the stack that
 *   holds the function's local variables, saved registers, and bookkeeping.
 *
 *   The stack grows DOWNWARD on x86-64 (toward lower addresses).
 *
 *   STACK FRAME LAYOUT (typical at -O0):
 *   ┌─────────────────────────────────┐  High address
 *   │  Caller's stack frame           │
 *   ├─────────────────────────────────┤
 *   │  Arg 8 (if >6 args)            │  [rbp+24]
 *   │  Arg 7 (if >6 args)            │  [rbp+16]
 *   │  Return address (from 'call')   │  [rbp+8]
 *   │  Saved rbp (from 'push rbp')    │  [rbp+0]  ← rbp points here
 *   │  Local variable 1               │  [rbp-4]
 *   │  Local variable 2               │  [rbp-8]
 *   │  Local variable 3               │  [rbp-12]
 *   │  ...                            │
 *   │  (padding for alignment)        │  ← rsp points here (top of stack)
 *   └─────────────────────────────────┘  Low address
 *
 *   FUNCTION PROLOGUE (executed at function entry):
 *     push   rbp           ; Save caller's base pointer
 *     mov    rbp, rsp      ; Set our base pointer = current stack pointer
 *     sub    rsp, N        ; Allocate N bytes for local variables
 *
 *   FUNCTION EPILOGUE (executed at function exit):
 *     leave                ; Equivalent to: mov rsp, rbp; pop rbp
 *     ret                  ; Pop return address from stack, jump to it
 *
 * BUILD:
 *   gcc -Wall -Wextra -O0 -S -o stack_frames.s stack_frames.c
 *   gcc -Wall -Wextra -O0 -S -fverbose-asm -o stack_frames_verbose.s stack_frames.c
 *   gcc -Wall -Wextra -O0 -o stack_frames stack_frames.c
 *
 * WHAT TO OBSERVE:
 *   1. Every function starts with push rbp; mov rbp, rsp (prologue)
 *   2. Local variables are at [rbp - offset]
 *   3. Each 'call' pushes a return address (8 bytes)
 *   4. 'leave' and 'ret' reverse the prologue and call
 *   5. Nested calls create a chain of saved rbp values (the "call stack")
 */

#include <stdio.h>
#include <stdint.h>

/*
 * func_c — The innermost function in our call chain
 *
 * When we reach func_c, the stack looks like this:
 *
 *   [main's frame]
 *     ↓ call func_a
 *   [func_a's frame] — saved rbp points to main's frame
 *     ↓ call func_b
 *   [func_b's frame] — saved rbp points to func_a's frame
 *     ↓ call func_c
 *   [func_c's frame] — saved rbp points to func_b's frame  ← WE ARE HERE
 *
 * The chain of saved rbp values IS the call stack. Debuggers follow
 * this chain to show you the backtrace (stack trace).
 *
 * OBSERVE: In the assembly, func_c's local variable 'local_c' is at
 *   [rbp-4] (or similar), just like every other function's first local.
 *   Each function "thinks" it's at the bottom of the world, but they're
 *   all stacked on top of each other.
 */
int func_c(int x) {
    int local_c = x * 3;  /* Local variable — lives on this frame */

    /* Print stack addresses so you can see the growth pattern */
    printf("    func_c: &local_c  = %p (frame 3 — deepest)\n",
           (void *)&local_c);

    return local_c;
}

/*
 * func_b — Middle of the call chain
 *
 * OBSERVE: func_b's frame sits between func_a's and func_c's.
 *   &local_b's address will be BETWEEN the addresses of local_a and local_c.
 *   Remember: stack grows DOWN, so local_b's address < local_a's address
 *   and local_b's address > local_c's address.
 */
int func_b(int x) {
    int local_b = x * 2;  /* Local variable — lives on this frame */

    printf("  func_b: &local_b  = %p (frame 2)\n", (void *)&local_b);

    /* Call func_c — creates yet another stack frame below ours */
    int result = func_c(local_b);
    return result + local_b;
}

/*
 * func_a — Outermost in our call chain (called from main)
 *
 * OBSERVE the PROLOGUE in the assembly:
 *   push   rbp           ; Save main's rbp (so we can return to main's frame)
 *   mov    rbp, rsp      ; Our rbp = current top of stack
 *   sub    rsp, 16       ; Make room for local variables
 *
 * OBSERVE the EPILOGUE:
 *   leave                ; rsp = rbp, then pop rbp (restores main's rbp)
 *   ret                  ; Pop return address, jump back to main
 */
int func_a(int x) {
    int local_a = x + 1;  /* Local variable — lives on this frame */

    printf("func_a: &local_a  = %p (frame 1)\n", (void *)&local_a);

    /* Call func_b — creates a new stack frame below ours */
    int result = func_b(local_a);
    return result + local_a;
}

/*
 * factorial_recursive — Recursive function to demonstrate stack growth
 *
 * Each recursive call creates a NEW stack frame with its own copy of 'n'.
 * For factorial(5), there are 6 stack frames for this function alone:
 *   factorial(5) → factorial(4) → factorial(3) → factorial(2) → factorial(1) → factorial(0)
 *
 * OBSERVE: The address of 'n' gets LOWER with each call (stack grows down).
 * The difference between addresses ≈ the size of one stack frame.
 *
 * WARNING: Deep recursion can cause a STACK OVERFLOW because the stack
 * has a finite size (typically 8MB on Linux). Try:
 *   ulimit -s    # Shows stack size limit in KB
 *
 * ASSEMBLY PATTERN for recursion:
 *   The function checks the base case with cmp/je (compare and jump-if-equal).
 *   If not base case, it calls ITSELF with 'call factorial_recursive'.
 *   Each call pushes another return address and allocates another frame.
 *   On return, each frame multiplies by its saved 'n' value and returns.
 */
long factorial_recursive(int n) {
    /* Print the address of n to show stack growth */
    printf("  factorial(%d): &n = %p", n, (void *)&n);

    if (n <= 1) {
        printf(" [BASE CASE — stack starts unwinding]\n");
        return 1;
    }

    printf("\n");

    /* Recursive call — creates a new stack frame below this one */
    long sub_result = factorial_recursive(n - 1);

    /* After the recursive call returns, we're back in OUR frame.
     * Our 'n' is still intact because it was saved on OUR stack frame.
     * The sub-frames have been destroyed (rsp moved back up). */
    return (long)n * sub_result;
}

/*
 * demonstrate_frame_pointer — Shows how rbp chains form the call stack
 *
 * At -O0, every function saves the previous rbp value. This creates a
 * LINKED LIST of frame pointers:
 *
 *   rbp → [saved_rbp] → [saved_rbp] → [saved_rbp] → ... → 0
 *         (our frame)   (caller)     (caller's caller)     (bottom)
 *
 * This is how debuggers walk the stack to produce backtraces.
 *
 * NOTE: At -O2 or with -fomit-frame-pointer, the compiler may NOT use rbp
 * as a frame pointer. Instead, it uses DWARF unwinding info (.eh_frame
 * section) to walk the stack. This frees rbp for general use.
 */
void demonstrate_frame_pointer(void) {
    /* Read the current rbp value using inline assembly */
    uintptr_t current_rbp;
    __asm__ volatile("mov %%rbp, %0" : "=r"(current_rbp));

    printf("\n=== Frame Pointer Chain (rbp linked list) ===\n\n");
    printf("Walking the saved-rbp chain from current frame:\n\n");

    uintptr_t *frame = (uintptr_t *)current_rbp;
    int depth = 0;

    /* Walk up to 6 frames (or until we hit a suspicious address) */
    while (frame != NULL && depth < 6) {
        uintptr_t saved_rbp = frame[0];       /* [rbp+0] = saved rbp */
        uintptr_t return_addr = frame[1];      /* [rbp+8] = return address */

        printf("  Frame %d: rbp=%p, saved_rbp=%p, return_addr=%p\n",
               depth, (void *)frame, (void *)saved_rbp, (void *)return_addr);

        /* Sanity check: if saved_rbp doesn't look like a stack address, stop */
        if (saved_rbp == 0 || saved_rbp < 0x7f0000000000UL) {
            printf("  (End of frame chain or left stack region)\n");
            break;
        }

        frame = (uintptr_t *)saved_rbp;
        depth++;
    }

    printf("\nThis chain is exactly what 'backtrace' in GDB follows!\n");
}

int main(void) {
    printf("=== Stack Frame Explorer ===\n\n");

    /* Part 1: Nested function calls — see frames stacking up */
    printf("--- Nested Call Chain: main → func_a → func_b → func_c ---\n\n");

    int main_local = 100;
    printf("main: &main_local = %p (main's frame — highest address)\n",
           (void *)&main_local);

    int result = func_a(main_local);
    printf("\nResult: %d\n", result);

    printf("\nOBSERVE: Addresses DECREASE with each deeper call.\n");
    printf("The stack grows DOWNWARD (toward lower memory addresses).\n");

    /* Part 2: Recursion — see the stack grow with each recursive call */
    printf("\n--- Recursive Factorial: Stack Growth ---\n\n");
    printf("Watch the address of 'n' decrease with each recursive call:\n\n");

    long fact = factorial_recursive(6);
    printf("\n6! = %ld\n", fact);

    /* Part 3: Frame pointer chain */
    demonstrate_frame_pointer();

    printf("\n=== Exploration Commands ===\n\n");
    printf("Generate assembly to see prologue/epilogue:\n");
    printf("  gcc -O0 -S -fverbose-asm -o stack_frames.s stack_frames.c\n");
    printf("  # Look for: push rbp / mov rbp,rsp / sub rsp,N (prologue)\n");
    printf("  # Look for: leave / ret (epilogue)\n\n");
    printf("Compare with -fomit-frame-pointer (no rbp-based frames):\n");
    printf("  gcc -O2 -fomit-frame-pointer -S -o stack_frames_nofp.s stack_frames.c\n");
    printf("  # Notice: no push rbp / mov rbp,rsp — rbp is used as a general register\n");

    return 0;
}
