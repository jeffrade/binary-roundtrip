// =============================================================================
// Module 8: JVM Essentials — JIT Warmup Effect
// =============================================================================
//
// CONCEPT: The JVM starts by INTERPRETING bytecode, then progressively
// compiles "hot" methods to native code using the JIT (Just-In-Time) compiler.
// This means the first few executions of a method are SLOW (interpreted),
// and later executions are FAST (compiled to native code).
//
// JIT COMPILATION TIERS (HotSpot JVM):
//   Tier 0: Interpreter
//     - Bytecode is executed instruction-by-instruction
//     - No compilation overhead, but slow execution
//     - The JVM profiles which methods are called frequently
//
//   Tier 1-3: C1 Compiler (Client compiler)
//     - Quick compilation with basic optimizations
//     - Produces decent native code fast
//     - Continues profiling for further optimization
//
//   Tier 4: C2 Compiler (Server compiler)
//     - Aggressive optimization: inlining, loop unrolling, escape analysis,
//       vectorization, dead code elimination
//     - Takes longer to compile but produces very fast code
//     - This is where JVM performance matches (or exceeds) static compilers
//
// THE WARMUP EFFECT:
//   - First invocations: interpreted (slow)
//   - After ~1,500 invocations: C1 compiles the method (medium speed)
//   - After ~10,000 invocations: C2 compiles the method (full speed)
//
//   These thresholds are controlled by:
//     -XX:CompileThreshold=10000  (for C2 compilation)
//     -XX:Tier3CompileThreshold=2000  (for C1 compilation)
//
// WHY THIS MATTERS:
//   - Benchmarks of JVM code must include warmup (otherwise you're measuring
//     the interpreter, not the JIT'd code)
//   - Short-lived programs pay the warmup cost but never see the benefit
//   - Server applications benefit most: warmup happens once, then everything
//     runs at native speed for hours/days
//
// TO OBSERVE:
//   1. Compile and run:
//        javac JITWarmup.java && java JITWarmup
//
//   2. Run with JIT compilation logging:
//        java -XX:+PrintCompilation JITWarmup
//      This prints every method as the JIT compiles it. You'll see:
//        - Tier 3 compilations first (C1)
//        - Tier 4 compilations later (C2)
//        - The "%" symbol means OSR (On-Stack Replacement — the JIT replaced
//          the method while it was still running)
//
//   3. Run with assembly output (needs hsdis plugin):
//        java -XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly JITWarmup
//      This shows the actual x86 assembly the JIT generates.
//      (Requires the hsdis disassembler plugin to be installed.)
//
//   4. Compare with Rust/C: Those have NO warmup because they compile
//      ahead-of-time. The first invocation is as fast as the millionth.
//
// =============================================================================

public class JITWarmup {

    // =========================================================================
    // Computation-heavy method: matrix multiplication
    // =========================================================================
    // We use matrix multiplication because:
    //   - It's computationally intensive (O(n^3))
    //   - The JIT can apply many optimizations:
    //     * Loop unrolling
    //     * Bounds check elimination (array indices are predictable)
    //     * Vectorization (SIMD instructions for parallel multiply-add)
    //     * Register allocation optimization
    //   - The difference between interpreted and JIT'd is dramatic

    static final int MATRIX_SIZE = 64;

    /**
     * Multiply two square matrices.
     *
     * In the INTERPRETER, each bytecode instruction is dispatched individually:
     *   - iaload: array load (with bounds check)
     *   - imul: integer multiply
     *   - iadd: integer add
     *   - iastore: array store (with bounds check)
     *   Each operation involves looking up the bytecode, dispatching to a handler,
     *   and returning. Massive overhead.
     *
     * After JIT COMPILATION (C2), this becomes:
     *   - Bounds checks eliminated (the JIT proves indices are always valid)
     *   - Inner loop vectorized with SIMD (processes 4-8 elements at once)
     *   - Array accesses use direct pointer arithmetic (no bounds check)
     *   - Registers allocated optimally
     *   - Result: ~10-50x faster than interpreted
     */
    static int[][] matmul(int[][] a, int[][] b) {
        int n = a.length;
        int[][] result = new int[n][n];
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int sum = 0;
                for (int k = 0; k < n; k++) {
                    sum += a[i][k] * b[k][j];
                }
                result[i][j] = sum;
            }
        }
        return result;
    }

    /**
     * A simpler computation for additional warmup observation:
     * sum of squares up to n.
     */
    static long sumOfSquares(int n) {
        long sum = 0;
        for (int i = 1; i <= n; i++) {
            sum += (long) i * i;
        }
        return sum;
    }

    // =========================================================================
    // Warmup measurement
    // =========================================================================

    public static void main(String[] args) {
        System.out.println("=== Module 8: JIT Warmup Effect ===");
        System.out.println();

        // ---------------------------------------------------------------------
        // Matrix multiplication warmup curve
        // ---------------------------------------------------------------------
        System.out.println("--- Matrix Multiplication Warmup ---");
        System.out.println("Multiplying " + MATRIX_SIZE + "x" + MATRIX_SIZE +
                           " matrices repeatedly.");
        System.out.println("Watch the time decrease as the JIT kicks in.");
        System.out.println();

        // Create test matrices
        int[][] a = new int[MATRIX_SIZE][MATRIX_SIZE];
        int[][] b = new int[MATRIX_SIZE][MATRIX_SIZE];
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                a[i][j] = i + j;
                b[i][j] = i - j;
            }
        }

        // Run multiple iterations, timing each one
        int iterations = 200;
        long[] times = new long[iterations];

        System.out.printf("  %-12s %-15s %-15s %s%n",
                          "Iteration", "Time (us)", "Speedup", "Phase");
        System.out.println("  " + "-".repeat(60));

        for (int iter = 0; iter < iterations; iter++) {
            long start = System.nanoTime();
            int[][] result = matmul(a, b);
            long end = System.nanoTime();

            times[iter] = end - start;
            long timeUs = times[iter] / 1000;

            // Prevent dead code elimination — use the result
            if (result[0][0] == Integer.MIN_VALUE) {
                System.out.println("unexpected");
            }

            // Print selected iterations to show the warmup curve
            if (iter < 10 || iter % 20 == 0 || iter == iterations - 1) {
                double speedup = (double) times[0] / times[iter];
                String phase;
                if (iter < 3) {
                    phase = "Interpreted (Tier 0)";
                } else if (iter < 20) {
                    phase = "C1 compiling (Tier 1-3)";
                } else if (iter < 50) {
                    phase = "C2 compiling (Tier 4)";
                } else {
                    phase = "Fully JIT'd";
                }

                System.out.printf("  %-12d %-15d %-15.2fx %s%n",
                                  iter, timeUs, speedup, phase);
            }
        }

        System.out.println();
        System.out.printf("  First iteration:  %d us (interpreted)%n", times[0] / 1000);
        System.out.printf("  Last iteration:   %d us (JIT compiled)%n",
                          times[iterations - 1] / 1000);
        System.out.printf("  Speedup:          %.1fx%n",
                          (double) times[0] / times[iterations - 1]);
        System.out.println();

        // ---------------------------------------------------------------------
        // Sum of squares: simpler warmup example
        // ---------------------------------------------------------------------
        System.out.println("--- Sum of Squares Warmup ---");
        System.out.println("Computing sum of squares up to 1,000,000 repeatedly.");
        System.out.println();

        int N = 1_000_000;
        int simpleIterations = 100;

        System.out.printf("  %-12s %-15s %-15s%n", "Iteration", "Time (us)", "Speedup");
        System.out.println("  " + "-".repeat(45));

        long[] simpleTimes = new long[simpleIterations];
        for (int iter = 0; iter < simpleIterations; iter++) {
            long start = System.nanoTime();
            long result = sumOfSquares(N);
            long end = System.nanoTime();

            simpleTimes[iter] = end - start;

            // Prevent dead code elimination
            if (result == Long.MIN_VALUE) {
                System.out.println("unexpected");
            }

            if (iter < 5 || iter % 10 == 0 || iter == simpleIterations - 1) {
                double speedup = (double) simpleTimes[0] / simpleTimes[iter];
                System.out.printf("  %-12d %-15d %-15.2fx%n",
                                  iter, simpleTimes[iter] / 1000, speedup);
            }
        }

        System.out.println();
        System.out.printf("  First:   %d us%n", simpleTimes[0] / 1000);
        System.out.printf("  Last:    %d us%n", simpleTimes[simpleIterations - 1] / 1000);
        System.out.printf("  Speedup: %.1fx%n",
                          (double) simpleTimes[0] / simpleTimes[simpleIterations - 1]);
        System.out.println();

        // ---------------------------------------------------------------------
        // JVM info
        // ---------------------------------------------------------------------
        System.out.println("--- JVM Information ---");
        System.out.println("  JVM: " + System.getProperty("java.vm.name"));
        System.out.println("  Version: " + System.getProperty("java.version"));
        System.out.println("  Vendor: " + System.getProperty("java.vm.vendor"));
        System.out.println("  Available processors: " + Runtime.getRuntime().availableProcessors());
        System.out.println();

        // ---------------------------------------------------------------------
        // How to explore further
        // ---------------------------------------------------------------------
        System.out.println("--- Exploration Guide ---");
        System.out.println();
        System.out.println("  1. See JIT compilation events:");
        System.out.println("       java -XX:+PrintCompilation JITWarmup");
        System.out.println("     Output columns: timestamp compile_id tier method");
        System.out.println("     Tier 3 = C1 (quick), Tier 4 = C2 (optimized)");
        System.out.println("     '%' = OSR (On-Stack Replacement — compiled mid-execution)");
        System.out.println();
        System.out.println("  2. See generated assembly (needs hsdis plugin):");
        System.out.println("       java -XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly JITWarmup");
        System.out.println();
        System.out.println("  3. Disable JIT to see pure interpreter speed:");
        System.out.println("       java -Xint JITWarmup");
        System.out.println("     (ALL iterations will be slow — no warmup improvement)");
        System.out.println();
        System.out.println("  4. Force immediate C2 compilation:");
        System.out.println("       java -XX:CompileThreshold=1 JITWarmup");
        System.out.println("     (Warmup is faster, but compilation overhead is front-loaded)");
        System.out.println();
        System.out.println("  KEY CONTRAST WITH NATIVE CODE:");
        System.out.println("    Rust/C compile ahead-of-time. First call = fastest call.");
        System.out.println("    JVM compiles at runtime. First calls are slow (interpreted).");
        System.out.println("    But: the JIT can optimize based on RUNTIME profiling data");
        System.out.println("    that a static compiler can't see (e.g., which branch is");
        System.out.println("    taken most often, which types are actually used).");
    }
}
