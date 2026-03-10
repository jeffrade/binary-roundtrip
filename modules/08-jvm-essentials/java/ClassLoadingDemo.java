// =============================================================================
// Module 8: JVM Essentials — Class Loading Demo
// =============================================================================
//
// CONCEPT: The JVM loads classes LAZILY — only when they are first referenced.
// This is fundamentally different from native binaries (C/Rust), where all code
// is loaded into memory at process start.
//
// CLASS LOADING HIERARCHY:
//   The JVM uses a parent-delegation model with three classloaders:
//
//   1. BOOTSTRAP ClassLoader (written in native C/C++, not a Java class)
//      - Loads core JDK classes: java.lang.*, java.util.*, etc.
//      - Loads from $JAVA_HOME/lib (or jrt:/ in Java 9+)
//      - This is the root — it has no parent
//
//   2. PLATFORM ClassLoader (Java 9+; was "Extension" ClassLoader before)
//      - Loads platform-specific extension classes
//      - Parent: Bootstrap ClassLoader
//
//   3. APPLICATION ClassLoader (also called "System" ClassLoader)
//      - Loads classes from the classpath (-cp or CLASSPATH)
//      - This is where YOUR classes are loaded from
//      - Parent: Platform ClassLoader
//
// HOW CLASS LOADING WORKS:
//   When the JVM encounters a class reference (e.g., "new MyClass()"):
//   1. Ask the Application ClassLoader: "Do you have MyClass?"
//   2. Application ClassLoader delegates to parent: "Platform, do you have it?"
//   3. Platform delegates to parent: "Bootstrap, do you have it?"
//   4. Bootstrap checks its cache and core JDK — not found
//   5. Platform checks its extensions — not found
//   6. Application searches the classpath — FOUND, loads it
//   7. The class is verified, prepared, resolved, and initialized
//
//   Steps in class initialization:
//   a. LOADING:      Read the .class file bytes
//   b. VERIFICATION: Check bytecode validity (stack depth, types, etc.)
//   c. PREPARATION:  Allocate memory for static fields (zero-initialized)
//   d. RESOLUTION:   Resolve symbolic references to other classes
//   e. INITIALIZATION: Run static initializers (<clinit>)
//
// TO OBSERVE:
//   1. Compile and run:
//        javac ClassLoadingDemo.java && java ClassLoadingDemo
//
//   2. Run with verbose class loading:
//        java -verbose:class ClassLoadingDemo
//      This prints EVERY class as it's loaded. You'll see hundreds of JDK
//      classes loaded before your code even starts!
//
//   3. Run with specific class loading tracing:
//        java -XX:+TraceClassLoading ClassLoadingDemo
//      (Available on some JVM implementations)
//
// =============================================================================

public class ClassLoadingDemo {

    // =========================================================================
    // Inner classes — each has a static initializer that prints when loaded.
    // This lets us observe the EXACT moment each class is initialized.
    // =========================================================================

    static class AlwaysUsed {
        static {
            // This static initializer runs when the class is first used.
            // "First use" means: creating an instance, calling a static method,
            // or accessing a static field (other than a compile-time constant).
            System.out.println("  [LOADED] AlwaysUsed — static initializer ran");
        }

        public void greet() {
            System.out.println("  AlwaysUsed.greet() called");
        }
    }

    static class LoadedOnCondition {
        static {
            System.out.println("  [LOADED] LoadedOnCondition — static initializer ran");
        }

        public void greet() {
            System.out.println("  LoadedOnCondition.greet() called");
        }
    }

    static class NeverReferenced {
        static {
            // This will NEVER print if we never reference this class!
            // The JVM doesn't load classes speculatively.
            System.out.println("  [LOADED] NeverReferenced — static initializer ran");
        }

        public void greet() {
            System.out.println("  NeverReferenced.greet() called");
        }
    }

    static class LazyHelper {
        static {
            System.out.println("  [LOADED] LazyHelper — static initializer ran");
        }

        // A static field that's expensive to compute.
        // This computation only happens when LazyHelper is first accessed.
        static final long EXPENSIVE_VALUE = computeExpensiveValue();

        private static long computeExpensiveValue() {
            System.out.println("  LazyHelper: computing expensive value...");
            long sum = 0;
            for (int i = 0; i < 1_000_000; i++) {
                sum += i;
            }
            return sum;
        }
    }

    // This class demonstrates that compile-time constants DON'T trigger loading.
    static class ConstantHolder {
        static {
            System.out.println("  [LOADED] ConstantHolder — static initializer ran");
        }

        // This is a COMPILE-TIME CONSTANT (static final primitive/String).
        // Accessing it does NOT trigger class loading! The compiler inlines
        // the value at the use site.
        static final int CONSTANT = 42;

        // This is NOT a compile-time constant (it's computed at runtime).
        // Accessing it DOES trigger class loading.
        static final int COMPUTED = Integer.parseInt("99");
    }

    // =========================================================================
    // Nested class loading — loading one class can trigger loading another
    // =========================================================================

    static class Parent {
        static {
            System.out.println("  [LOADED] Parent — static initializer ran");
        }

        public void method() {
            System.out.println("  Parent.method() called");
        }
    }

    static class Child extends Parent {
        static {
            System.out.println("  [LOADED] Child — static initializer ran");
            // Note: Parent's static initializer runs BEFORE Child's,
            // because the JVM must initialize the parent before the child.
        }

        @Override
        public void method() {
            System.out.println("  Child.method() called");
        }
    }

    // =========================================================================
    // Main: Demonstrate lazy class loading
    // =========================================================================

    public static void main(String[] args) {
        System.out.println("=== Module 8: Class Loading Demo ===");
        System.out.println();

        // ---------------------------------------------------------------------
        // Classloader hierarchy
        // ---------------------------------------------------------------------
        System.out.println("--- ClassLoader Hierarchy ---");
        ClassLoader appLoader = ClassLoadingDemo.class.getClassLoader();
        System.out.println("  ClassLoadingDemo loaded by: " + appLoader);
        System.out.println("  Parent of app classloader:  " +
            (appLoader != null ? appLoader.getParent() : "null (bootstrap)"));

        ClassLoader stringLoader = String.class.getClassLoader();
        System.out.println("  String loaded by: " +
            (stringLoader != null ? stringLoader : "null (bootstrap classloader — native code)"));
        System.out.println();

        // ---------------------------------------------------------------------
        // Lazy loading demonstration
        // ---------------------------------------------------------------------
        System.out.println("--- Lazy Class Loading ---");
        System.out.println("At this point, none of our inner classes are loaded yet.");
        System.out.println();

        System.out.println("Step 1: Creating an AlwaysUsed instance...");
        AlwaysUsed a = new AlwaysUsed();
        a.greet();
        System.out.println("  (AlwaysUsed was loaded because we created an instance.)");
        System.out.println();

        // Conditional loading
        boolean condition = args.length > 0;
        System.out.println("Step 2: LoadedOnCondition will only load if args.length > 0");
        System.out.println("  args.length = " + args.length + ", condition = " + condition);
        if (condition) {
            System.out.println("  Condition is true — loading LoadedOnCondition:");
            LoadedOnCondition b = new LoadedOnCondition();
            b.greet();
        } else {
            System.out.println("  Condition is false — LoadedOnCondition is NOT loaded!");
            System.out.println("  (Run with: java ClassLoadingDemo somearg to trigger it)");
        }
        System.out.println();

        System.out.println("Step 3: NeverReferenced is never used, so it's never loaded.");
        System.out.println("  (You won't see '[LOADED] NeverReferenced' anywhere.)");
        System.out.println();

        // ---------------------------------------------------------------------
        // Compile-time constants vs runtime values
        // ---------------------------------------------------------------------
        System.out.println("--- Compile-Time Constants vs Runtime Values ---");

        System.out.println("Step 4: Accessing ConstantHolder.CONSTANT (compile-time constant)...");
        int val = ConstantHolder.CONSTANT;
        System.out.println("  Value: " + val);
        System.out.println("  (If ConstantHolder was NOT loaded, that proves the constant");
        System.out.println("   was inlined by javac at compile time.)");
        System.out.println();

        System.out.println("Step 5: Accessing ConstantHolder.COMPUTED (runtime value)...");
        int computed = ConstantHolder.COMPUTED;
        System.out.println("  Value: " + computed);
        System.out.println("  (ConstantHolder IS loaded now — accessing a non-constant");
        System.out.println("   static field triggers class initialization.)");
        System.out.println();

        // ---------------------------------------------------------------------
        // Lazy initialization pattern
        // ---------------------------------------------------------------------
        System.out.println("--- Lazy Initialization via Class Loading ---");
        System.out.println("Step 6: LazyHelper has an expensive static field.");
        System.out.println("  It won't be computed until we first access the class.");
        System.out.println("  Accessing LazyHelper.EXPENSIVE_VALUE now:");
        long expVal = LazyHelper.EXPENSIVE_VALUE;
        System.out.println("  Value: " + expVal);
        System.out.println("  (This pattern is used in production for thread-safe lazy init.)");
        System.out.println();

        // ---------------------------------------------------------------------
        // Parent-child class loading order
        // ---------------------------------------------------------------------
        System.out.println("--- Parent-Child Loading Order ---");
        System.out.println("Step 7: Creating a Child instance (Parent must be loaded first):");
        Child child = new Child();
        child.method();
        System.out.println("  (Notice: Parent's static initializer ran BEFORE Child's.)");
        System.out.println();

        // ---------------------------------------------------------------------
        // Summary
        // ---------------------------------------------------------------------
        System.out.println("--- Summary ---");
        System.out.println("  Classes are loaded LAZILY — only on first active use.");
        System.out.println("  Compile-time constants are inlined and don't trigger loading.");
        System.out.println("  Parent classes are always initialized before children.");
        System.out.println("  The bootstrap classloader loads core JDK classes.");
        System.out.println("  The application classloader loads your classes.");
        System.out.println();
        System.out.println("  Run with -verbose:class to see ALL class loading activity:");
        System.out.println("    java -verbose:class ClassLoadingDemo");
        System.out.println("  You'll see hundreds of classes loaded before main() even starts!");
    }
}
