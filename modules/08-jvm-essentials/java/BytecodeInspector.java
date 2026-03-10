// =============================================================================
// Module 8: JVM Essentials — Bytecode Inspector
// =============================================================================
//
// CONCEPT: When you compile Java with javac, it produces bytecode — a set of
// stack-based instructions for the JVM virtual machine. This is NOT native
// code. The JVM interprets (or JIT-compiles) this bytecode at runtime.
//
// KEY DIFFERENCE FROM NATIVE CODE:
//   C/Rust: Source -> Machine Code (x86 instructions: mov, add, call)
//   Java:   Source -> Bytecode (JVM instructions: iload, iadd, invokevirtual)
//
//   The bytecode is platform-independent. The same .class file runs on
//   x86, ARM, RISC-V — anywhere a JVM implementation exists.
//
// JVM IS STACK-BASED:
//   Unlike x86 (register-based), the JVM uses an operand stack:
//     iload_1      Push local variable 1 onto the stack
//     iload_2      Push local variable 2 onto the stack
//     iadd         Pop two values, add them, push result
//     istore_3     Pop result and store in local variable 3
//
//   Compare with x86 (register-based):
//     mov eax, [rbp-4]    Load local 1 into register
//     add eax, [rbp-8]    Add local 2
//     mov [rbp-12], eax   Store to local 3
//
// TO EXPLORE:
//   1. Compile:
//        javac BytecodeInspector.java
//
//   2. Disassemble the bytecode:
//        javap -c BytecodeInspector
//      This shows the bytecode instructions for each method.
//
//   3. Full verbose output:
//        javap -c -verbose BytecodeInspector
//      This also shows the constant pool, line number table, and stack map.
//
//   4. Private methods too:
//        javap -c -p BytecodeInspector
//
// BYTECODE INSTRUCTION CATEGORIES:
//   Load/Store:    iload, istore, aload, astore (move between stack and locals)
//   Arithmetic:    iadd, isub, imul, idiv, irem
//   Type convert:  i2l, i2f, l2d (int to long, int to float, etc.)
//   Object:        new, getfield, putfield
//   Invoke:        invokevirtual, invokestatic, invokeinterface, invokespecial
//   Stack:         dup, pop, swap
//   Control:       ifeq, goto, tableswitch, lookupswitch
//   Return:        ireturn, areturn, return (void)
//
// =============================================================================

public class BytecodeInspector {

    // =========================================================================
    // Static field and instance field — shows getfield/putfield bytecodes
    // =========================================================================

    static int staticCounter = 0;
    int instanceValue;

    // =========================================================================
    // Simple arithmetic — demonstrates basic stack operations
    // =========================================================================

    /**
     * Bytecode for this method:
     *   iload_1          // Push parameter a (local variable 1) onto stack
     *   iload_2          // Push parameter b (local variable 2) onto stack
     *   iadd             // Pop both, add, push result
     *   iload_1          // Push a again
     *   iload_2          // Push b again
     *   isub             // Pop both, subtract, push result
     *   imul             // Pop both (sum and diff), multiply, push result
     *   ireturn          // Return the int on top of stack
     *
     * The operand stack at each step:
     *   []               -> iload_1 -> [a]
     *   [a]              -> iload_2 -> [a, b]
     *   [a, b]           -> iadd    -> [a+b]
     *   [a+b]            -> iload_1 -> [a+b, a]
     *   [a+b, a]         -> iload_2 -> [a+b, a, b]
     *   [a+b, a, b]      -> isub    -> [a+b, a-b]
     *   [a+b, a-b]       -> imul    -> [(a+b)*(a-b)]
     *   [(a+b)*(a-b)]    -> ireturn -> returned
     */
    public static int arithmetic(int a, int b) {
        int sum = a + b;
        int diff = a - b;
        return sum * diff;
    }

    // =========================================================================
    // Local variables and type widening
    // =========================================================================

    /**
     * Demonstrates type widening in bytecode:
     *   iload_1      // Load int parameter
     *   i2l          // Widen int to long (occupies 2 stack slots)
     *   lload_2      // Load long parameter (2 stack slots)
     *   ladd         // Long addition
     *   l2d          // Convert long to double
     *   dreturn      // Return double
     *
     * Notice: long and double use 2 slots on the operand stack and in
     * the local variable table. This is because the JVM was designed for
     * 32-bit systems where 64-bit values need two "words".
     */
    public static double typeWidening(int intVal, long longVal) {
        long combined = (long) intVal + longVal;
        return (double) combined;
    }

    // =========================================================================
    // Object creation — shows new, dup, invokespecial
    // =========================================================================

    /**
     * Bytecode for object creation:
     *   new #ClassName        // Allocate memory for object (returns reference)
     *   dup                   // Duplicate the reference (need one for init, one to keep)
     *   invokespecial <init>  // Call constructor (consumes one reference)
     *                         // The other reference stays on stack
     *
     * Why dup? Because invokespecial <init> consumes the reference,
     * but we need to keep a reference to the new object. So we duplicate
     * it first: one copy for the constructor call, one for us.
     */
    public static BytecodeInspector createObject(int value) {
        BytecodeInspector obj = new BytecodeInspector();
        obj.instanceValue = value;  // putfield
        return obj;
    }

    // =========================================================================
    // Virtual method call — shows invokevirtual
    // =========================================================================

    /**
     * Virtual method dispatch in bytecode:
     *   aload_0              // Push 'this' reference
     *   invokevirtual #method // Look up method in vtable, call it
     *
     * invokevirtual does:
     *   1. Pop the object reference from the stack
     *   2. Look up the ACTUAL class of the object (runtime type)
     *   3. Find the method in that class's vtable
     *   4. Jump to the method code
     *
     * This is how polymorphism works: the method called depends on the
     * runtime type, not the compile-time type.
     */
    public int getDoubledValue() {
        return this.instanceValue * 2;
    }

    // =========================================================================
    // Static method call — shows invokestatic
    // =========================================================================

    /**
     * Static dispatch in bytecode:
     *   invokestatic #method
     *
     * Unlike invokevirtual, there's no object reference to look up.
     * The target method is known at compile time and resolved directly.
     * This is slightly faster because there's no vtable lookup.
     */
    public static int staticHelper(int x) {
        return x * x;
    }

    // =========================================================================
    // Interface method call — shows invokeinterface
    // =========================================================================

    /**
     * Interface dispatch in bytecode:
     *   aload_1               // Push the interface reference
     *   invokeinterface #method // Look up in itable (interface table)
     *
     * invokeinterface is different from invokevirtual because:
     *   - A class can implement multiple interfaces
     *   - Interface methods aren't at fixed vtable offsets
     *   - The JVM must search the itable (slower than vtable lookup)
     *
     * The JIT compiler optimizes this with inline caches: if the same
     * concrete type is seen repeatedly, it caches the method address.
     */
    interface Computable {
        int compute(int input);
    }

    static class Doubler implements Computable {
        @Override
        public int compute(int input) {
            return input * 2;
        }
    }

    static class Squarer implements Computable {
        @Override
        public int compute(int input) {
            return input * input;
        }
    }

    public static int useInterface(Computable c, int val) {
        return c.compute(val);  // invokeinterface
    }

    // =========================================================================
    // String concatenation — shows StringBuilder usage
    // =========================================================================

    /**
     * String concatenation bytecode (before Java 9):
     *   new StringBuilder
     *   dup
     *   invokespecial StringBuilder.<init>
     *   ldc "Hello, "
     *   invokevirtual StringBuilder.append(String)
     *   aload_0
     *   invokevirtual StringBuilder.append(String)
     *   ldc "! Age: "
     *   invokevirtual StringBuilder.append(String)
     *   iload_1
     *   invokevirtual StringBuilder.append(int)
     *   invokevirtual StringBuilder.toString()
     *   areturn
     *
     * In Java 9+, this uses invokedynamic with StringConcatFactory
     * instead of explicit StringBuilder. The JVM can then optimize
     * the concatenation strategy at runtime (e.g., pre-compute buffer size).
     */
    public static String stringConcat(String name, int age) {
        return "Hello, " + name + "! Age: " + age;
    }

    // =========================================================================
    // Control flow — shows if/goto bytecodes
    // =========================================================================

    /**
     * Control flow bytecodes:
     *   iload_1         // Load n
     *   ifle label_neg  // If n <= 0, jump to label_neg
     *   ... positive path ...
     *   goto label_end  // Skip negative path
     *   label_neg:
     *   ... negative path ...
     *   label_end:
     *   ireturn
     *
     * The JVM has many comparison bytecodes:
     *   ifeq, ifne, iflt, ifge, ifgt, ifle  (compare with 0)
     *   if_icmpeq, if_icmpne, ...            (compare two ints)
     *   ifnull, ifnonnull                     (null checks)
     */
    public static String classify(int n) {
        if (n > 0) {
            return "positive";
        } else if (n < 0) {
            return "negative";
        } else {
            return "zero";
        }
    }

    // =========================================================================
    // Switch/case — shows tableswitch or lookupswitch
    // =========================================================================

    /**
     * Dense switch (consecutive values) → tableswitch:
     *   tableswitch 1 to 4:  // Jump table: O(1) lookup
     *     1: label_one
     *     2: label_two
     *     3: label_three
     *     4: label_four
     *     default: label_default
     *
     * Sparse switch (non-consecutive values) → lookupswitch:
     *   lookupswitch:         // Binary search: O(log n) lookup
     *     10: label_ten
     *     50: label_fifty
     *     100: label_hundred
     *     default: label_default
     */
    public static String denseSwitch(int day) {
        // This should generate a tableswitch (consecutive values)
        switch (day) {
            case 1: return "Monday";
            case 2: return "Tuesday";
            case 3: return "Wednesday";
            case 4: return "Thursday";
            case 5: return "Friday";
            case 6: return "Saturday";
            case 7: return "Sunday";
            default: return "Unknown";
        }
    }

    public static String sparseSwitch(int code) {
        // This should generate a lookupswitch (non-consecutive values)
        switch (code) {
            case 100: return "Continue";
            case 200: return "OK";
            case 404: return "Not Found";
            case 500: return "Server Error";
            default: return "Unknown";
        }
    }

    // =========================================================================
    // Array operations — shows newarray, iaload, iastore
    // =========================================================================

    /**
     * Array bytecodes:
     *   bipush 5         // Push size (5) onto stack
     *   newarray int     // Allocate int[5] on heap, push reference
     *   astore_1         // Store array reference in local 1
     *   aload_1          // Load array reference
     *   iconst_0         // Push index (0)
     *   bipush 10        // Push value (10)
     *   iastore          // array[0] = 10  (pops ref, index, value)
     *   ...
     *   aload_1          // Load array reference
     *   iconst_2         // Push index (2)
     *   iaload           // Push array[2] onto stack (pops ref, index)
     */
    public static int arrayDemo() {
        int[] arr = new int[5];      // newarray
        arr[0] = 10;                 // iastore
        arr[1] = 20;                 // iastore
        arr[2] = 30;                 // iastore
        return arr[0] + arr[2];      // iaload + iaload + iadd
    }

    // =========================================================================
    // Exception handling — shows try/catch in bytecode
    // =========================================================================

    /**
     * Exception handling bytecodes:
     *   The method has an "exception table" that maps code ranges to handlers:
     *     from   to    target  type
     *     0      10    11      ArithmeticException
     *
     *   This means: if an exception of type ArithmeticException is thrown
     *   between bytecode offsets 0 and 10, jump to offset 11 (the handler).
     *
     *   athrow: explicitly throws an exception (pops reference from stack)
     */
    public static int divideWithCatch(int a, int b) {
        try {
            return a / b;            // idiv — may throw ArithmeticException if b==0
        } catch (ArithmeticException e) {
            return -1;               // Handler: return -1 on division by zero
        }
    }

    // =========================================================================
    // Main: Exercise all the methods
    // =========================================================================

    public static void main(String[] args) {
        System.out.println("=== Module 8: Bytecode Inspector ===");
        System.out.println();

        // Call every method so they appear in the bytecode output
        System.out.println("--- Method Results ---");
        System.out.println("  arithmetic(7, 3)       = " + arithmetic(7, 3) + " ((7+3)*(7-3) = 40)");
        System.out.println("  typeWidening(42, 100L)  = " + typeWidening(42, 100L));
        System.out.println("  staticHelper(5)         = " + staticHelper(5) + " (5*5 = 25)");
        System.out.println("  classify(42)            = " + classify(42));
        System.out.println("  classify(-1)            = " + classify(-1));
        System.out.println("  classify(0)             = " + classify(0));
        System.out.println("  denseSwitch(3)          = " + denseSwitch(3));
        System.out.println("  sparseSwitch(404)       = " + sparseSwitch(404));
        System.out.println("  arrayDemo()             = " + arrayDemo() + " (10+30 = 40)");
        System.out.println("  divideWithCatch(10, 3)  = " + divideWithCatch(10, 3));
        System.out.println("  divideWithCatch(10, 0)  = " + divideWithCatch(10, 0) + " (caught!)");
        System.out.println("  stringConcat(\"World\", 25) = " + stringConcat("World", 25));
        System.out.println();

        // Object creation and virtual dispatch
        BytecodeInspector obj = createObject(42);
        System.out.println("  createObject(42).getDoubledValue() = " + obj.getDoubledValue());
        System.out.println();

        // Interface dispatch
        Computable doubler = new Doubler();
        Computable squarer = new Squarer();
        System.out.println("  useInterface(Doubler, 5)  = " + useInterface(doubler, 5) + " (invokevirtual via interface)");
        System.out.println("  useInterface(Squarer, 5)  = " + useInterface(squarer, 5) + " (invokevirtual via interface)");
        System.out.println();

        // Static counter
        staticCounter++;
        System.out.println("  staticCounter = " + staticCounter + " (getstatic/putstatic)");
        System.out.println();

        // -------------------------------------------------------------------
        // Exploration guide
        // -------------------------------------------------------------------
        System.out.println("--- Bytecode Exploration Guide ---");
        System.out.println();
        System.out.println("  1. Disassemble the bytecode:");
        System.out.println("       javap -c BytecodeInspector");
        System.out.println();
        System.out.println("  2. Full verbose output (constant pool, stack maps, line numbers):");
        System.out.println("       javap -c -verbose BytecodeInspector");
        System.out.println();
        System.out.println("  3. Include private methods:");
        System.out.println("       javap -c -p BytecodeInspector");
        System.out.println();
        System.out.println("  KEY BYTECODES TO FIND:");
        System.out.println("    invokevirtual   — getDoubledValue() call");
        System.out.println("    invokestatic    — staticHelper() call");
        System.out.println("    invokeinterface — Computable.compute() call");
        System.out.println("    invokespecial   — constructor calls (<init>)");
        System.out.println("    new + dup       — object allocation pattern");
        System.out.println("    tableswitch     — denseSwitch() method");
        System.out.println("    lookupswitch    — sparseSwitch() method");
        System.out.println("    i2l, l2d        — type widening in typeWidening()");
        System.out.println("    athrow          — exception table in divideWithCatch()");
    }
}
