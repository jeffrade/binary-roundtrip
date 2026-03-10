/*
 * Module 1: Execution Models — Java Execution Model Deep Dive
 * ============================================================
 *
 * WHAT THIS FILE DEMONSTRATES:
 *   This program shows that Java runs on a Virtual Machine, and demonstrates
 *   the JVM's execution progression:
 *     Bytecode → Interpretation → JIT Compilation → Native Code
 *
 * THE JVM's EXECUTION PROGRESSION:
 *   1. LOADING:        The JVM loads Hello.class, verifies the bytecode.
 *   2. INTERPRETATION: The JVM interpreter executes bytecode instruction
 *                      by instruction (slow, but starts immediately).
 *   3. PROFILING:      The JVM tracks which methods are called frequently
 *                      (these are "hot" methods).
 *   4. JIT COMPILATION: Hot methods are compiled to native machine code
 *                       by the C1 (client) or C2 (server) JIT compiler.
 *   5. NATIVE:         Subsequent calls to JIT-compiled methods run the
 *                       native code directly — as fast as C!
 *
 * WHAT TO OBSERVE:
 *   - Run normally:    java ExecutionModelDemo
 *   - With JIT info:   java -XX:+PrintCompilation ExecutionModelDemo
 *     In PrintCompilation output, look for:
 *       - Lines with numbers increasing (compilation IDs)
 *       - "hotLoop" or "ExecutionModelDemo::hotLoop" appearing
 *       - This means the JIT detected the loop as hot and compiled it!
 *
 * KEY INSIGHT:
 *   Modern JVMs don't just "interpret" — they adaptively optimize. Code
 *   starts interpreted but hot paths get compiled to native code that
 *   can rival C performance. This is why Java is fast despite running
 *   on a VM.
 */

public class ExecutionModelDemo {

    /*
     * This method runs a tight loop to trigger JIT compilation.
     * The JVM's profiler counts how many times the loop body executes.
     * After a threshold (typically ~10,000 iterations), the JIT compiler
     * kicks in and compiles this method to native machine code.
     */
    public static long hotLoop(int iterations) {
        long sum = 0;
        for (int i = 0; i < iterations; i++) {
            sum += i;
            /* Each iteration of this loop is initially interpreted:
             * the JVM reads bytecode instructions one at a time.
             * After enough iterations, the JIT compiler generates
             * optimized native x86-64 code for this entire method. */
        }
        return sum;
    }

    public static void main(String[] args) {
        System.out.println("=== Java Execution Model Demo ===");
        System.out.println();

        /* ---------------------------------------------------------
         * 1. Show that we're running on a Virtual Machine
         * --------------------------------------------------------- */
        System.out.println("[1] JVM Runtime Information:");
        Runtime runtime = Runtime.getRuntime();
        System.out.println("    Java version:      " + System.getProperty("java.version"));
        System.out.println("    JVM name:          " + System.getProperty("java.vm.name"));
        System.out.println("    JVM vendor:        " + System.getProperty("java.vm.vendor"));
        System.out.println("    JVM version:       " + System.getProperty("java.vm.version"));
        System.out.println("    OS architecture:   " + System.getProperty("os.arch"));
        System.out.println("    Available CPUs:    " + runtime.availableProcessors());
        System.out.println("    Max memory (JVM):  " + runtime.maxMemory() / (1024 * 1024) + " MB");
        System.out.println("    Free memory (JVM): " + runtime.freeMemory() / (1024 * 1024) + " MB");
        System.out.println();

        /* ---------------------------------------------------------
         * 2. Show where the class file lives
         * --------------------------------------------------------- */
        System.out.println("[2] Class File Location:");
        try {
            /* getProtectionDomain().getCodeSource() tells us where the
             * JVM loaded this class from. This shows that the JVM loads
             * .class files (bytecode) at runtime — unlike C where the
             * binary is self-contained. */
            java.security.CodeSource cs =
                ExecutionModelDemo.class.getProtectionDomain().getCodeSource();
            if (cs != null) {
                System.out.println("    Loaded from: " + cs.getLocation());
            } else {
                System.out.println("    Loaded from: (bootstrap classloader)");
            }
        } catch (SecurityException e) {
            System.out.println("    (Could not determine — security restriction)");
        }
        System.out.println();

        /* ---------------------------------------------------------
         * 3. Demonstrate the interpretation → JIT progression
         * --------------------------------------------------------- */
        System.out.println("[3] JIT Compilation Demonstration:");
        System.out.println("    Running a hot loop to trigger JIT compilation...");
        System.out.println();

        /* Run the loop several times with increasing iteration counts.
         * The first few runs are interpreted. As the JVM's profiler
         * detects that hotLoop() is "hot" (called frequently with many
         * iterations), it triggers JIT compilation. */
        int[] iterationCounts = {100, 1000, 10000, 100000, 1000000, 10000000};

        for (int count : iterationCounts) {
            long startNs = System.nanoTime();
            long result = hotLoop(count);
            long elapsedNs = System.nanoTime() - startNs;
            double elapsedMs = elapsedNs / 1_000_000.0;

            System.out.printf("    %,10d iterations: sum = %,15d  time = %8.3f ms%n",
                              count, result, elapsedMs);
        }

        System.out.println();
        System.out.println("    Observe: later runs may be proportionally faster as");
        System.out.println("    the JIT compiler optimizes the hot loop to native code.");
        System.out.println();

        /* ---------------------------------------------------------
         * 4. Summary
         * --------------------------------------------------------- */
        System.out.println("[4] Key Observations:");
        System.out.println("    - This program is NOT an ELF binary. It's JVM bytecode.");
        System.out.println("    - The JVM (a native C++ program) loaded and ran our .class file.");
        System.out.println("    - The JVM started by interpreting the bytecode.");
        System.out.println("    - Hot methods were JIT-compiled to native machine code.");
        System.out.println("    - This adaptive optimization is why Java can be fast.");
        System.out.println();
        System.out.println("To see JIT compilation in action, re-run with:");
        System.out.println("    java -XX:+PrintCompilation ExecutionModelDemo");
        System.out.println();
        System.out.println("To see the bytecode instructions:");
        System.out.println("    javap -c ExecutionModelDemo");
    }
}
