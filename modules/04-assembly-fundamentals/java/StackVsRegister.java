/*
 * StackVsRegister.java — Contrasts JVM's stack-based bytecode with x86 registers
 * ============================================================================
 *
 * MODULE 4: Assembly Fundamentals (x86-64, read-only goal)
 * EXERCISE: Understand the difference between stack-based and register-based
 *
 * CONCEPT:
 *   There are TWO major approaches to instruction set design:
 *
 *   1. REGISTER-BASED (x86-64, ARM, RISC-V):
 *      - Operands are in named registers (rax, rdi, rsi, ...)
 *      - Instructions specify source and destination registers
 *      - Example: add rax, rsi  (rax = rax + rsi)
 *      - Modern CPUs use this approach
 *
 *   2. STACK-BASED (JVM, WebAssembly, Python bytecode):
 *      - Operands are on a virtual stack
 *      - Instructions pop operands, compute, push result
 *      - Example: iload_0; iload_1; iadd  (pop two, add, push result)
 *      - Virtual machines often use this approach
 *
 *   WHY does the JVM use a stack?
 *     - Simpler to generate bytecode (no register allocation needed)
 *     - Portable (not tied to any CPU's register count or layout)
 *     - Compact (no register numbers in instructions, just opcodes)
 *     - The JIT compiler converts to register-based at runtime
 *
 * BUILD & EXPLORE:
 *   javac StackVsRegister.java
 *   javap -c StackVsRegister           # Show bytecode (basic)
 *   javap -c -verbose StackVsRegister  # Show bytecode with details
 *
 * COMPARE:
 *   JVM bytecode for add(a, b):         x86-64 assembly for add(a, b):
 *   ┌─────────────────────────────┐     ┌──────────────────────────────┐
 *   │  iload_0    ; push a        │     │  ; a is already in edi       │
 *   │  iload_1    ; push b        │     │  ; b is already in esi       │
 *   │  iadd       ; pop both, add │     │  lea eax, [rdi+rsi]          │
 *   │  ireturn    ; return top    │     │  ret  ; return eax           │
 *   └─────────────────────────────┘     └──────────────────────────────┘
 *
 *   The JVM version uses 4 instructions to express what x86 does in 2.
 *   But the JVM's JIT compiler translates the bytecode to native code
 *   at runtime, so performance is similar in practice.
 */
public class StackVsRegister {

    /*
     * add — Simple addition
     *
     * JVM BYTECODE (run: javap -c StackVsRegister):
     *   0: iload_0        // Push local variable #0 (a) onto operand stack
     *   1: iload_1        // Push local variable #1 (b) onto operand stack
     *   2: iadd           // Pop two ints, add them, push result
     *   3: ireturn        // Pop int from stack and return it
     *
     * STACK TRACE through execution:
     *   Step 0: Stack = []                  → iload_0 → Stack = [a]
     *   Step 1: Stack = [a]                 → iload_1 → Stack = [a, b]
     *   Step 2: Stack = [a, b]              → iadd    → Stack = [a+b]
     *   Step 3: Stack = [a+b]               → ireturn → returns a+b
     *
     * COMPARE TO x86-64:
     *   ; a is in edi, b is in esi (register-based — no stack needed!)
     *   lea    eax, [rdi+rsi]    ; eax = a + b (one instruction)
     *   ret                       ; return eax
     */
    public static int add(int a, int b) {
        return a + b;
    }

    /*
     * multiply — Simple multiplication
     *
     * JVM BYTECODE:
     *   0: iload_0        // Push a
     *   1: iload_1        // Push b
     *   2: imul           // Pop two, multiply, push result
     *   3: ireturn        // Return
     *
     * x86-64 EQUIVALENT:
     *   mov    eax, edi       ; eax = a
     *   imul   eax, esi       ; eax *= b
     *   ret
     */
    public static int multiply(int a, int b) {
        return a * b;
    }

    /*
     * quadratic — Computes a*x*x + b*x + c (a more complex expression)
     *
     * JVM BYTECODE (roughly):
     *   iload_0          // Push a
     *   iload_3          // Push x     Stack: [a, x]
     *   imul             // a*x        Stack: [a*x]
     *   iload_3          // Push x     Stack: [a*x, x]
     *   imul             // a*x*x      Stack: [a*x*x]
     *   iload_1          // Push b     Stack: [a*x*x, b]
     *   iload_3          // Push x     Stack: [a*x*x, b, x]
     *   imul             // b*x        Stack: [a*x*x, b*x]
     *   iadd             // a*x*x+b*x  Stack: [a*x*x+b*x]
     *   iload_2          // Push c     Stack: [a*x*x+b*x, c]
     *   iadd             // result     Stack: [a*x*x+b*x+c]
     *   ireturn          // Return
     *
     * OBSERVE: Every intermediate value lives on the stack. The JVM
     * never says "put this in register r3" — it just pushes and pops.
     * This is what makes bytecode portable: no register assumptions.
     *
     * x86-64 EQUIVALENT (at -O2):
     *   imul   edi, ecx       ; a *= x
     *   imul   esi, ecx       ; b *= x
     *   lea    eax, [rdi+rsi] ; eax = a*x + b*x  (wait, that's wrong)
     *   Actually: the compiler would use specific registers efficiently.
     *   The point is: x86 keeps values in registers, JVM uses a stack.
     */
    public static int quadratic(int a, int b, int c, int x) {
        return a * x * x + b * x + c;
    }

    /*
     * max — Conditional (uses if_icmple bytecode)
     *
     * JVM BYTECODE:
     *   0: iload_0             // Push a
     *   1: iload_1             // Push b
     *   2: if_icmple 7         // If a <= b, jump to offset 7
     *   5: iload_0             // Push a (a > b case)
     *   6: ireturn             // Return a
     *   7: iload_1             // Push b (a <= b case)
     *   8: ireturn             // Return b
     *
     * OBSERVE: The JVM comparison instruction 'if_icmple' POPS both
     * values from the stack to compare them, then conditionally jumps.
     * In x86, 'cmp' doesn't pop anything — values stay in registers.
     *
     * x86-64 EQUIVALENT (at -O2):
     *   cmp    edi, esi       ; Compare a and b (values stay in registers)
     *   mov    eax, esi       ; Assume result = b
     *   cmovge eax, edi       ; If a >= b, result = a (no branch!)
     *   ret
     */
    public static int max(int a, int b) {
        if (a > b) {
            return a;
        } else {
            return b;
        }
    }

    /*
     * sumLoop — A for loop (shows JVM loop bytecode)
     *
     * JVM BYTECODE:
     *   0: iconst_0            // Push 0
     *   1: istore_1            // Store to local var 1 (total = 0)
     *   2: iconst_1            // Push 1
     *   3: istore_2            // Store to local var 2 (i = 1)
     *   4: iload_2             // Push i
     *   5: iload_0             // Push n
     *   6: if_icmpgt 19        // If i > n, exit loop
     *   9: iload_1             // Push total
     *  10: iload_2             // Push i
     *  11: iadd                // total + i
     *  12: istore_1            // total = total + i
     *  13: iinc 2, 1           // i++ (special instruction for incrementing a local)
     *  16: goto 4              // Jump back to loop condition
     *  19: iload_1             // Push total
     *  20: ireturn             // Return total
     *
     * OBSERVE: 'iinc' is a special bytecode that increments a local variable
     * in-place without going through the operand stack. It's an optimization
     * for the very common i++ pattern.
     *
     * x86-64 at -O2 might not even have a loop — the compiler computes n*(n+1)/2.
     */
    public static int sumLoop(int n) {
        int total = 0;
        for (int i = 1; i <= n; i++) {
            total += i;
        }
        return total;
    }

    /*
     * localVariables — Shows how locals are stored in JVM
     *
     * JVM uses a "local variable array" (not the operand stack) for named
     * variables. Instructions like istore_N and iload_N move values between
     * the operand stack and the local variable array.
     *
     * Local variable array slots:
     *   Slot 0 = first arg (or 'this' for instance methods)
     *   Slot 1 = second arg (or first arg for static methods with 1+ params)
     *   ...
     *   Slot N = first local variable after args
     *
     * COMPARE: In x86, local variables live on the stack frame [rbp-offset]
     *   or in registers. The JVM's local variable array is conceptually
     *   similar to the stack frame's local variable area.
     */
    public static int localVariables(int x) {
        int a = x + 1;       // iload_0; iconst_1; iadd; istore_1
        int b = a * 2;       // iload_1; iconst_2; imul; istore_2
        int c = a + b;       // iload_1; iload_2; iadd; istore_3
        return c;             // iload_3; ireturn
    }

    public static void main(String[] args) {
        System.out.println("=== Stack-Based (JVM) vs Register-Based (x86) ===\n");

        System.out.println("Results (verify these are correct):");
        System.out.println("  add(10, 3) = " + add(10, 3));
        System.out.println("  multiply(10, 3) = " + multiply(10, 3));
        System.out.println("  quadratic(1, -3, 2, 5) = " + quadratic(1, -3, 2, 5));
        System.out.println("    (1*25 + -3*5 + 2 = 25 - 15 + 2 = 12)");
        System.out.println("  max(10, 3) = " + max(10, 3));
        System.out.println("  sumLoop(10) = " + sumLoop(10));
        System.out.println("  localVariables(5) = " + localVariables(5));
        System.out.println("    (a=6, b=12, c=18 → returns 18)");

        System.out.println("\n=== Stack-Based vs Register-Based ===\n");

        System.out.println("  Stack-Based (JVM)              | Register-Based (x86-64)");
        System.out.println("  -------------------------------|---------------------------");
        System.out.println("  Operands on a virtual stack    | Operands in named registers");
        System.out.println("  iload, istore to move data     | mov to move data");
        System.out.println("  iadd pops 2, pushes result     | add writes to dest register");
        System.out.println("  if_icmpXX pops and branches    | cmp + jCC (values stay)");
        System.out.println("  Portable (no register deps)    | CPU-specific");
        System.out.println("  Compact bytecode               | Larger instructions");
        System.out.println("  JIT compiles to register-based | Already register-based");

        System.out.println("\n=== The JVM Execution Pipeline ===\n");
        System.out.println("  .java → javac → .class (stack-based bytecode)");
        System.out.println("       → JVM interpreter (executes bytecode)");
        System.out.println("       → JIT compiler (compiles hot methods to native code)");
        System.out.println("       → x86-64 machine code (register-based!)");
        System.out.println();
        System.out.println("  So ultimately, JVM code DOES run as register-based x86.");
        System.out.println("  The stack-based bytecode is an intermediate representation.");

        System.out.println("\n=== Exploration Commands ===\n");
        System.out.println("  javac StackVsRegister.java");
        System.out.println("  javap -c StackVsRegister              # Basic bytecode");
        System.out.println("  javap -c -verbose StackVsRegister     # With constant pool");
        System.out.println();
        System.out.println("  Focus on the add() method first — it's the simplest.");
        System.out.println("  Then look at quadratic() to see a complex expression.");
        System.out.println("  Compare sumLoop() to the x86 assembly of sum_range() in C.");
        System.out.println();
        System.out.println("  To see the JIT-compiled native code (advanced):");
        System.out.println("  java -XX:+PrintCompilation StackVsRegister");
        System.out.println("  java -XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly StackVsRegister");
        System.out.println("  (The latter requires hsdis-amd64.so — the HotSpot disassembler plugin)");
    }
}
