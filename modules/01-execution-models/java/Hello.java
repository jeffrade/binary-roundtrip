/*
 * Module 1: Execution Models — Java (Bytecode + Virtual Machine)
 * ==============================================================
 *
 * WHAT THIS FILE DEMONSTRATES:
 *   Java uses a two-step execution model:
 *     1. javac compiles .java source to .class bytecode files (NOT native code).
 *     2. java (the JVM) loads and executes the .class bytecode.
 *
 * THE EXECUTION MODEL:
 *   Hello.java → [javac] → Hello.class (bytecode) → [JVM] → Output
 *
 *   Unlike C:    No native machine code is produced by javac.
 *   Unlike Ruby: The bytecode IS saved to disk (the .class file).
 *   Like Ruby:   A virtual machine interprets/JIT-compiles the bytecode.
 *
 * WHAT TO OBSERVE:
 *   - Compile:  javac Hello.java        → produces Hello.class
 *   - Run:      java Hello              → JVM loads and runs Hello.class
 *   - Inspect:  file Hello.class        → "compiled Java class data"
 *   - Inspect:  xxd Hello.class | head  → starts with magic bytes CA FE BA BE
 *   - Inspect:  javap -c Hello          → disassemble the bytecode!
 *
 * BYTECODE INSTRUCTIONS (you'll see these with `javap -c`):
 *   getstatic     — Load a static field (like System.out)
 *   ldc           — Load a constant (like a string literal)
 *   invokevirtual — Call a virtual method (like println)
 *   return        — Return from the method
 *
 * KEY INSIGHT:
 *   The .class file contains JVM bytecode, not x86 machine code. The JVM
 *   is the native program that actually runs on your CPU. The JVM
 *   initially interprets the bytecode, then JIT-compiles hot methods to
 *   native code at runtime for performance.
 *
 * COMPARE WITH:
 *   - C/Rust: Compiler produces native machine code directly.
 *   - Ruby:   Bytecode exists only in memory, never written to disk.
 *   - Java:   Bytecode is saved to .class files, then loaded by the JVM.
 */

public class Hello {
    public static void main(String[] args) {
        System.out.println("Hello from Java!");
        System.out.println();
        System.out.println("Execution model: BYTECODE + VIRTUAL MACHINE (JVM)");
        System.out.println("  - javac compiled Hello.java -> Hello.class (bytecode)");
        System.out.println("  - The JVM loaded Hello.class and is executing the bytecode.");
        System.out.println("  - The bytecode is NOT native machine code — it's JVM instructions.");
        System.out.println();
        System.out.println("The .class file starts with magic bytes: CA FE BA BE");
        System.out.println("  (Try: xxd Hello.class | head -1)");
        System.out.println();
        System.out.println("To see the bytecode:");
        System.out.println("  javap -c Hello");
        System.out.println();
        System.out.println("To see JIT compilation happening:");
        System.out.println("  java -XX:+PrintCompilation Hello");
    }
}
