// =============================================================================
// Module 8: JVM Essentials — JVM vs Native Execution Comparison
// =============================================================================
//
// CONCEPT: This program contrasts JVM execution with native (C/Rust) execution.
// The JVM adds layers of abstraction that native binaries don't have:
//
// WHAT HAPPENS WHEN YOU RUN A JAVA PROGRAM:
//   1. OS loads the JVM binary (a native ELF executable)
//   2. JVM initializes: allocates heap, starts GC threads, sets up JIT
//   3. JVM loads bootstrap classes (java.lang.*, etc.) — hundreds of classes
//   4. JVM loads your .class file from disk
//   5. JVM VERIFIES your bytecode (checks types, stack depth, etc.)
//   6. JVM starts INTERPRETING your bytecode
//   7. After enough invocations, JIT compiles hot methods to native code
//   8. Your code finally runs at native speed (but with GC pauses)
//
// WHAT HAPPENS WHEN YOU RUN A NATIVE BINARY (C/Rust):
//   1. OS loads the ELF binary into memory
//   2. Dynamic linker resolves shared library references
//   3. _start calls main()
//   4. Your code runs at full speed immediately
//
// COST OF THE JVM LAYERS:
//   - Startup time: JVM initialization takes 50-200ms
//   - Memory overhead: JVM itself uses 30-100 MB before your code runs
//   - JIT warmup: first calls are 10-50x slower (interpreted)
//   - GC pauses: periodic latency spikes (microseconds to milliseconds)
//   - Safepoint overhead: JIT inserts polling code for GC coordination
//
// BENEFITS OF THE JVM:
//   - Platform independence: same .class file runs everywhere
//   - Runtime optimization: JIT can optimize based on actual runtime behavior
//   - Memory safety: no use-after-free, no buffer overflows
//   - Rich ecosystem: reflection, dynamic class loading, profiling tools
//
// THIS PROGRAM: Runs a tight computation loop and reports timing.
// Compare the results with the equivalent C (c/equivalent.c in Module 7)
// or Rust (rust/zero_cost.rs in Module 7) to see the differences.
//
// =============================================================================

public class NativeCompare {

    // =========================================================================
    // Computation: Sum of squares of even numbers (same as Modules 7 C/Rust)
    // =========================================================================

    /**
     * Tight computation loop — the SAME algorithm as in Module 7's C and Rust.
     *
     * After JIT compilation (C2), this should produce native code similar to:
     *   .loop:
     *     test %eax, 1        ; check if even
     *     jne .skip
     *     imul %rax, %rax     ; square
     *     add %rcx, %rax      ; accumulate
     *   .skip:
     *     inc %eax
     *     cmp %eax, %edx
     *     jl .loop
     *
     * But the JVM version also includes:
     *   - Safepoint poll (a load from a special page that the GC can protect)
     *   - Possible deoptimization trap (if assumptions are invalidated)
     */
    static long sumOfSquaresOfEvens(long n) {
        long sum = 0;
        for (long i = 1; i <= n; i++) {
            if (i % 2 == 0) {
                sum += i * i;
            }
        }
        return sum;
    }

    /**
     * Matrix multiplication — a more complex computation.
     */
    static long matmulChecksum(int size, int iterations) {
        int[][] a = new int[size][size];
        int[][] b = new int[size][size];
        int[][] c = new int[size][size];

        // Initialize
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                a[i][j] = i + j;
                b[i][j] = i - j;
            }
        }

        // Multiply (repeated for benchmarking)
        for (int iter = 0; iter < iterations; iter++) {
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    int sum = 0;
                    for (int k = 0; k < size; k++) {
                        sum += a[i][k] * b[k][j];
                    }
                    c[i][j] = sum;
                }
            }
        }

        // Compute checksum
        long checksum = 0;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                checksum += c[i][j];
            }
        }
        return checksum;
    }

    // =========================================================================
    // Benchmark harness
    // =========================================================================

    public static void main(String[] args) {
        System.out.println("=== Module 8: JVM vs Native Execution ===");
        System.out.println();

        // -----------------------------------------------------------------
        // JVM information
        // -----------------------------------------------------------------
        System.out.println("--- JVM Information ---");
        System.out.println("  VM:       " + System.getProperty("java.vm.name"));
        System.out.println("  Version:  " + System.getProperty("java.version"));
        System.out.println("  Vendor:   " + System.getProperty("java.vm.vendor"));
        System.out.println("  OS:       " + System.getProperty("os.name") + " " +
                           System.getProperty("os.arch"));
        System.out.println("  CPUs:     " + Runtime.getRuntime().availableProcessors());

        Runtime rt = Runtime.getRuntime();
        System.out.printf("  Heap max: %,d MB%n", rt.maxMemory() / (1024 * 1024));
        System.out.println();

        // -----------------------------------------------------------------
        // At javac time: NO ELF binary is produced
        // -----------------------------------------------------------------
        System.out.println("--- Key Difference: Compilation ---");
        System.out.println("  javac NativeCompare.java  ->  NativeCompare.class (bytecode)");
        System.out.println("  rustc zero_cost.rs        ->  zero_cost (ELF binary)");
        System.out.println("  gcc equivalent.c          ->  equivalent (ELF binary)");
        System.out.println();
        System.out.println("  The .class file contains JVM bytecode, NOT native code.");
        System.out.println("  Native code is generated at RUNTIME by the JIT compiler.");
        System.out.println("  This means:");
        System.out.println("    - Java cannot run without a JVM installed");
        System.out.println("    - First executions are slow (interpreted)");
        System.out.println("    - But the JIT can optimize using runtime profiling data");
        System.out.println();

        // -----------------------------------------------------------------
        // Benchmark: Sum of squares
        // -----------------------------------------------------------------
        System.out.println("--- Benchmark: Sum of Squares of Evens (1..10,000,000) ---");

        long n = 10_000_000L;

        // Warmup (critical for fair JVM benchmarks!)
        System.out.println("  Warming up JIT (5 iterations)...");
        for (int i = 0; i < 5; i++) {
            sumOfSquaresOfEvens(n);
        }

        // Timed runs
        long bestTime = Long.MAX_VALUE;
        long result = 0;
        int trials = 10;

        System.out.println("  Running " + trials + " timed trials...");
        for (int i = 0; i < trials; i++) {
            long start = System.nanoTime();
            result = sumOfSquaresOfEvens(n);
            long elapsed = System.nanoTime() - start;

            if (elapsed < bestTime) {
                bestTime = elapsed;
            }

            System.out.printf("    Trial %d: %,d us (result: %d)%n",
                              i, elapsed / 1000, result);
        }

        System.out.printf("  Best time: %,d us%n", bestTime / 1000);
        System.out.printf("  Result:    %d%n", result);
        System.out.println();

        // Without warmup (measuring interpreted + JIT)
        System.out.println("  For comparison, time WITHOUT warmup (fresh method):");
        System.out.println("  (The JVM already JIT'd this method, so this isn't a true");
        System.out.println("   cold start. Use -Xint to disable JIT for pure interpretation.)");
        System.out.println();

        // -----------------------------------------------------------------
        // Benchmark: Matrix multiplication
        // -----------------------------------------------------------------
        System.out.println("--- Benchmark: Matrix Multiplication (64x64, 100 iterations) ---");

        // Warmup
        System.out.println("  Warming up...");
        matmulChecksum(64, 10);

        long matStart = System.nanoTime();
        long matResult = matmulChecksum(64, 100);
        long matTime = System.nanoTime() - matStart;

        System.out.printf("  Time:     %,d us%n", matTime / 1000);
        System.out.printf("  Checksum: %d%n", matResult);
        System.out.println();

        // -----------------------------------------------------------------
        // Startup overhead measurement
        // -----------------------------------------------------------------
        System.out.println("--- Startup Overhead ---");
        long processStart = Long.parseLong(
            java.lang.management.ManagementFactory.getRuntimeMXBean().getStartTime() + "");
        long now = System.currentTimeMillis();
        System.out.printf("  JVM uptime (including warmup loops): %d ms%n", now - processStart);
        System.out.println("  Of this, 50-200 ms is JVM startup (class loading, init).");
        System.out.println("  A native binary starts in < 1 ms.");
        System.out.println();

        // -----------------------------------------------------------------
        // Memory overhead
        // -----------------------------------------------------------------
        System.out.println("--- Memory Overhead ---");
        System.gc();
        System.out.printf("  Heap used:  %,d KB%n",
                          (rt.totalMemory() - rt.freeMemory()) / 1024);
        System.out.printf("  Heap total: %,d KB%n", rt.totalMemory() / 1024);
        System.out.printf("  Heap max:   %,d KB%n", rt.maxMemory() / 1024);
        System.out.println("  (A native binary with the same logic uses < 1 MB.)");
        System.out.println("  (The JVM overhead includes: JIT compiler, GC, class metadata,");
        System.out.println("   thread stacks, internal data structures.)");
        System.out.println();

        // -----------------------------------------------------------------
        // Comparison instructions
        // -----------------------------------------------------------------
        System.out.println("--- How to Compare with Native ---");
        System.out.println();
        System.out.println("  1. Run the Rust equivalent (Module 7):");
        System.out.println("       cd ../07-rust-llvm-deep-dive");
        System.out.println("       rustc -C opt-level=2 rust/zero_cost.rs -o build/zero_cost");
        System.out.println("       ./build/zero_cost");
        System.out.println();
        System.out.println("  2. Run the C equivalent (Module 7):");
        System.out.println("       gcc -O2 c/equivalent.c -o build/equivalent");
        System.out.println("       ./build/equivalent");
        System.out.println();
        System.out.println("  3. Disable JIT to see pure interpretation speed:");
        System.out.println("       java -Xint NativeCompare");
        System.out.println();
        System.out.println("  4. See JIT compilation events:");
        System.out.println("       java -XX:+PrintCompilation NativeCompare");
        System.out.println();
        System.out.println("  EXPECTED RESULTS:");
        System.out.println("    - JIT'd Java is within 1-3x of C/Rust at -O2");
        System.out.println("    - Interpreted Java is 10-50x slower than C/Rust");
        System.out.println("    - JVM startup is 50-200x slower than native startup");
        System.out.println("    - JVM memory usage is 10-100x higher than native");
    }
}
