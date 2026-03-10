// =============================================================================
// Module 8: JVM Essentials — Garbage Collection Demo
// =============================================================================
//
// CONCEPT: The JVM automatically reclaims memory from objects that are no
// longer reachable. This is a fundamental difference from C/Rust, where
// memory management is the programmer's (or compiler's) responsibility.
//
// HOW GC WORKS (Generational Hypothesis):
//   Most objects die young. The JVM exploits this with generational collection:
//
//   YOUNG GENERATION (Eden + Survivor spaces):
//     - Where new objects are allocated (fast: just bump a pointer)
//     - Collected frequently with a "minor GC" (copying collector)
//     - Most objects die here and are collected quickly
//     - Surviving objects are copied to Survivor space, then eventually
//       promoted to the Old Generation
//
//   OLD GENERATION (Tenured space):
//     - Long-lived objects end up here after surviving several minor GCs
//     - Collected less frequently with a "major/full GC"
//     - Full GC is much more expensive (scans the entire heap)
//
// THE COST OF GC:
//   1. SAFEPOINTS: The JIT compiler inserts "safepoint polls" in generated
//      native code. At these points, the JVM can pause all threads for GC.
//      This is WHY GC exists as a runtime cost even in JIT'd code.
//
//   2. STOP-THE-WORLD pauses: During GC, all application threads are paused.
//      Modern collectors (G1, ZGC, Shenandoah) minimize pause times, but
//      pauses still exist.
//
//   3. WRITE BARRIERS: When an old-gen object references a young-gen object,
//      the JVM must track this (via "card marking" or similar). This adds
//      a small overhead to every reference write.
//
// CONTRAST WITH NATIVE LANGUAGES:
//   C:    Programmer calls malloc/free. Bugs: use-after-free, double-free, leaks.
//   Rust: Compiler inserts drop() calls at compile time. Zero runtime cost.
//         The borrow checker prevents use-after-free at compile time.
//   Java: GC runs at runtime. No memory bugs, but unpredictable pauses.
//
// TO OBSERVE:
//   1. Compile and run with GC logging:
//        javac GCDemo.java
//        java -verbose:gc GCDemo
//
//   2. Detailed GC logging (Java 9+):
//        java -Xlog:gc*=info GCDemo
//
//   3. Set heap sizes to make GC more visible:
//        java -Xms32m -Xmx64m -verbose:gc GCDemo
//
//   4. Try different collectors:
//        java -XX:+UseSerialGC -verbose:gc GCDemo
//        java -XX:+UseG1GC -verbose:gc GCDemo
//        java -XX:+UseZGC -verbose:gc GCDemo       (Java 15+)
//        java -XX:+UseShenandoahGC -verbose:gc GCDemo (if available)
//
// =============================================================================

public class GCDemo {

    // =========================================================================
    // A class with a known size to make memory effects visible
    // =========================================================================

    static class DataChunk {
        // Each DataChunk holds a 1KB byte array.
        // 10,000 of these = ~10 MB of heap data.
        byte[] data;
        int id;

        DataChunk(int id) {
            this.id = id;
            this.data = new byte[1024]; // 1 KB payload
        }
    }

    // =========================================================================
    // Helper: Print memory statistics
    // =========================================================================

    static void printMemory(String label) {
        Runtime rt = Runtime.getRuntime();
        long total = rt.totalMemory();    // Current heap size
        long free = rt.freeMemory();      // Free space in current heap
        long max = rt.maxMemory();        // Maximum heap size (-Xmx)
        long used = total - free;

        System.out.printf("  [%s] Used: %,d KB | Free: %,d KB | Total: %,d KB | Max: %,d KB%n",
                          label, used / 1024, free / 1024, total / 1024, max / 1024);
    }

    // =========================================================================
    // Demo 1: Objects that become garbage
    // =========================================================================

    static void shortLivedObjects() {
        System.out.println("--- Demo 1: Short-Lived Objects (Young Generation GC) ---");
        System.out.println("  Allocating 50,000 objects that immediately become garbage...");
        System.out.println("  (These will be collected by minor/young GC.)");
        System.out.println();

        printMemory("Before");

        for (int i = 0; i < 50_000; i++) {
            // Each iteration creates a DataChunk that goes out of scope immediately.
            // The object is unreachable after this statement → eligible for GC.
            @SuppressWarnings("unused")
            DataChunk chunk = new DataChunk(i);
            // chunk goes out of scope here → garbage

            // Print progress periodically
            if (i % 10_000 == 0 && i > 0) {
                printMemory("After " + i + " allocations");
            }
        }

        printMemory("After all allocations");

        // Suggest GC (the JVM may or may not honor this)
        System.gc();
        printMemory("After System.gc()");
        System.out.println();
    }

    // =========================================================================
    // Demo 2: Objects that survive (promote to old generation)
    // =========================================================================

    static void longLivedObjects() {
        System.out.println("--- Demo 2: Long-Lived Objects (Old Generation) ---");
        System.out.println("  Keeping references to objects so they survive GC.");
        System.out.println("  These will be promoted from Young to Old generation.");
        System.out.println();

        printMemory("Before");

        // Keep references in an array → objects survive GC
        DataChunk[] survivors = new DataChunk[10_000];

        for (int i = 0; i < 10_000; i++) {
            survivors[i] = new DataChunk(i);

            if (i % 2_000 == 0 && i > 0) {
                printMemory("After " + i + " live allocations");
            }
        }

        printMemory("All 10,000 objects alive");

        // Now release half of them
        System.out.println();
        System.out.println("  Releasing 5,000 objects (nulling references)...");
        for (int i = 0; i < 5_000; i++) {
            survivors[i] = null;  // Object is now unreachable → garbage
        }

        System.gc();
        printMemory("After releasing 5,000 + GC");

        // Release the rest
        System.out.println("  Releasing remaining 5,000 objects...");
        for (int i = 5_000; i < 10_000; i++) {
            survivors[i] = null;
        }

        System.gc();
        printMemory("After releasing all + GC");
        System.out.println();
    }

    // =========================================================================
    // Demo 3: GC pressure — rapid allocation causing multiple collections
    // =========================================================================

    static void gcPressure() {
        System.out.println("--- Demo 3: GC Pressure (Rapid Allocation) ---");
        System.out.println("  Rapidly allocating and discarding objects to trigger GC.");
        System.out.println("  Each allocation replaces the previous one (continuous churn).");
        System.out.println();

        printMemory("Before");

        long startTime = System.nanoTime();
        long totalAllocated = 0;

        // Keep a small array of "live" references, but constantly replace them.
        // This creates high allocation rate with moderate live data.
        DataChunk[] ring = new DataChunk[100];

        for (int i = 0; i < 100_000; i++) {
            // Replace one slot — the old object becomes garbage.
            ring[i % ring.length] = new DataChunk(i);
            totalAllocated += 1024;  // Track how much we've allocated

            if (i % 20_000 == 0 && i > 0) {
                long elapsed = (System.nanoTime() - startTime) / 1_000_000;
                System.out.printf("  [%d ms] Allocated %,d KB total, %d objects live%n",
                                  elapsed, totalAllocated / 1024, ring.length);
            }
        }

        long totalTime = (System.nanoTime() - startTime) / 1_000_000;
        System.out.printf("  Total: %,d KB allocated in %d ms%n",
                          totalAllocated / 1024, totalTime);
        System.out.printf("  Allocation rate: %,d KB/ms%n",
                          totalAllocated / 1024 / Math.max(totalTime, 1));
        printMemory("After pressure test");
        System.out.println();
    }

    // =========================================================================
    // Demo 4: Finalization (deprecated but educational)
    // =========================================================================

    static class FinalizableObject {
        int id;

        FinalizableObject(int id) {
            this.id = id;
        }

        @SuppressWarnings("deprecation")
        @Override
        protected void finalize() {
            // WARNING: finalize() is deprecated since Java 9 and removed in
            // later versions. It exists here for educational purposes only.
            //
            // Problems with finalize():
            //   1. Non-deterministic: you don't know WHEN it will run
            //   2. May never run at all (JVM doesn't guarantee it)
            //   3. Adds overhead: GC must track finalizable objects separately
            //   4. Can resurrect objects (if finalize() stores 'this' somewhere)
            //
            // Compare with Rust's Drop trait:
            //   - Deterministic: runs at end of scope
            //   - Always runs (unless you call std::mem::forget)
            //   - No GC overhead
            //   - Cannot resurrect the object
            System.out.println("    Finalize called for object " + id);
        }
    }

    static void finalizationDemo() {
        System.out.println("--- Demo 4: Finalization (Deprecated, Educational) ---");
        System.out.println("  Creating objects with finalize() method...");
        System.out.println("  Note: finalize() is deprecated. Use try-with-resources instead.");
        System.out.println();

        for (int i = 0; i < 5; i++) {
            @SuppressWarnings("unused")
            FinalizableObject obj = new FinalizableObject(i);
            // obj goes out of scope → eligible for GC → finalize() eventually called
        }

        System.out.println("  Objects created and abandoned. Requesting GC...");
        System.gc();

        // Give the finalizer thread a moment to run
        try { Thread.sleep(100); } catch (InterruptedException e) { /* ignore */ }

        System.out.println("  (Finalize may or may not have run — it's non-deterministic!)");
        System.out.println("  Compare: Rust's Drop trait is deterministic and guaranteed.");
        System.out.println();
    }

    // =========================================================================
    // Demo 5: Memory leak via unintentional reference retention
    // =========================================================================

    static void memoryLeakDemo() {
        System.out.println("--- Demo 5: Simulated Memory Leak ---");
        System.out.println("  In Java, 'memory leaks' happen when you hold references");
        System.out.println("  to objects you no longer need (e.g., in a growing list).");
        System.out.println();

        printMemory("Before leak");

        // Simulate a leak: keep adding to a list, never removing.
        // In a real app, this might be a cache without eviction, a listener
        // list that keeps growing, or a static collection.
        java.util.ArrayList<DataChunk> leakyList = new java.util.ArrayList<>();

        for (int i = 0; i < 20_000; i++) {
            leakyList.add(new DataChunk(i));  // Growing forever!

            if (i % 5_000 == 0 && i > 0) {
                printMemory("After " + i + " objects added to list");
            }
        }

        printMemory("20,000 objects in list (all live — GC can't help!)");

        // GC won't help because all objects are still reachable through the list
        System.gc();
        printMemory("After System.gc() (no change — objects are still referenced)");

        // Fix the leak by clearing the list
        leakyList.clear();
        System.gc();
        printMemory("After clearing list + GC (memory recovered!)");
        System.out.println();
    }

    // =========================================================================
    // Main
    // =========================================================================

    public static void main(String[] args) {
        System.out.println("=== Module 8: Garbage Collection Demo ===");
        System.out.println();

        // Print JVM and GC info
        System.out.println("--- JVM & GC Information ---");
        System.out.println("  JVM: " + System.getProperty("java.vm.name"));
        System.out.println("  Version: " + System.getProperty("java.version"));

        // Print which GC is in use (Java 9+)
        for (java.lang.management.GarbageCollectorMXBean gc :
             java.lang.management.ManagementFactory.getGarbageCollectorMXBeans()) {
            System.out.println("  GC: " + gc.getName() + " (collections: " +
                             gc.getCollectionCount() + ")");
        }

        Runtime rt = Runtime.getRuntime();
        System.out.printf("  Heap: max=%,d KB, total=%,d KB, free=%,d KB%n",
                          rt.maxMemory() / 1024, rt.totalMemory() / 1024,
                          rt.freeMemory() / 1024);
        System.out.println();

        // Run demos
        shortLivedObjects();
        longLivedObjects();
        gcPressure();
        finalizationDemo();
        memoryLeakDemo();

        // Final GC statistics
        System.out.println("--- Final GC Statistics ---");
        for (java.lang.management.GarbageCollectorMXBean gc :
             java.lang.management.ManagementFactory.getGarbageCollectorMXBeans()) {
            System.out.println("  " + gc.getName() + ":");
            System.out.println("    Collections: " + gc.getCollectionCount());
            System.out.println("    Total time:  " + gc.getCollectionTime() + " ms");
        }
        System.out.println();

        System.out.println("--- Exploration Guide ---");
        System.out.println();
        System.out.println("  1. See GC events as they happen:");
        System.out.println("       java -verbose:gc GCDemo");
        System.out.println();
        System.out.println("  2. Detailed GC logging:");
        System.out.println("       java -Xlog:gc*=info GCDemo");
        System.out.println();
        System.out.println("  3. Small heap (more GC pressure):");
        System.out.println("       java -Xms16m -Xmx32m -verbose:gc GCDemo");
        System.out.println();
        System.out.println("  4. Try different collectors:");
        System.out.println("       java -XX:+UseSerialGC -verbose:gc GCDemo    (simple, STW)");
        System.out.println("       java -XX:+UseG1GC -verbose:gc GCDemo        (default, balanced)");
        System.out.println("       java -XX:+UseZGC -verbose:gc GCDemo         (low latency)");
        System.out.println();
        System.out.println("  KEY COST OF GC:");
        System.out.println("    The JIT inserts safepoint polls in compiled code.");
        System.out.println("    At these points, the GC can pause ALL threads.");
        System.out.println("    This is a cost that native languages (C/Rust) never pay.");
    }
}
