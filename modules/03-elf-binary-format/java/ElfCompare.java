/*
 * ElfCompare.java — Demonstrates that .class files are NOT ELF
 * ============================================================================
 *
 * MODULE 3: ELF Binary Format
 * EXERCISE: Understand the fundamental difference between JVM bytecode and ELF
 *
 * CONCEPT:
 *   Java compiles to BYTECODE (.class files), not native machine code.
 *   .class files have their own binary format defined by the JVM specification.
 *   They are NOT ELF files — they cannot be executed directly by the OS.
 *
 *   However, the JVM itself (the `java` binary) IS a native ELF executable!
 *   It's a C/C++ program compiled to machine code, just like any other binary.
 *
 *   So there are TWO levels:
 *     1. ElfCompare.class — JVM bytecode format (NOT ELF, NOT native)
 *     2. /usr/bin/java     — ELF executable (native x86-64 machine code)
 *
 * BUILD & RUN:
 *   javac ElfCompare.java       # Produces ElfCompare.class (bytecode)
 *   java ElfCompare              # JVM interprets/JITs the bytecode
 *
 * EXPLORE:
 *   file ElfCompare.class        # "compiled Java class data" (NOT ELF!)
 *   file $(which java)           # "ELF 64-bit LSB executable" or "shared object"
 *   hexdump -C ElfCompare.class | head -4   # Magic: CA FE BA BE
 *   hexdump -C $(which java) | head -4      # Magic: 7F 45 4C 46 (ELF)
 *   javap -c ElfCompare          # Disassemble the bytecode (not x86 assembly!)
 *   readelf -h $(which java)     # ELF header of the JVM binary
 *   ldd $(which java)            # Shared libraries the JVM depends on
 *
 * KEY INSIGHT:
 *   The .class file's magic bytes are CA FE BA BE ("cafe babe") — a famous
 *   Easter egg by James Gosling. ELF files start with 7F 45 4C 46 (".ELF").
 *   These magic bytes are how the OS and tools identify file formats.
 */
public class ElfCompare {

    /*
     * A simple method to demonstrate Java bytecode.
     *
     * OBSERVE with `javap -c ElfCompare`:
     *   This compiles to JVM stack-based instructions like:
     *     iload_0        // Push 'a' onto operand stack
     *     iload_1        // Push 'b' onto operand stack
     *     iadd           // Pop both, add, push result
     *     ireturn        // Return top of stack
     *
     *   Compare this to x86 assembly (register-based):
     *     mov eax, edi   // arg1 already in edi register
     *     add eax, esi   // Add arg2 (in esi) to eax
     *     ret            // Return value in eax
     *
     *   JVM bytecode is STACK-based (push, push, operate).
     *   x86 assembly is REGISTER-based (move to register, operate on register).
     */
    public static int add(int a, int b) {
        return a + b;
    }

    public static void main(String[] args) {
        System.out.println("=== Java .class vs ELF Binary Comparison ===\n");

        System.out.println("FUNDAMENTAL DIFFERENCE:");
        System.out.println("  .class files are JVM bytecode — NOT native machine code");
        System.out.println("  The JVM (java binary) IS a native ELF executable\n");

        // Demonstrate a simple computation
        int result = add(3, 4);
        System.out.println("add(3, 4) = " + result);
        System.out.println("  (Run `javap -c ElfCompare` to see the bytecode for this)\n");

        // Find the java binary path
        String javaHome = System.getProperty("java.home");
        String javaBinary = javaHome + "/bin/java";

        System.out.println("Java runtime info:");
        System.out.println("  java.home = " + javaHome);
        System.out.println("  JVM binary = " + javaBinary);
        System.out.println("  Java version = " + System.getProperty("java.version"));
        System.out.println("  JVM name = " + System.getProperty("java.vm.name"));
        System.out.println();

        // Use `file` command to show the difference
        System.out.println("--- Running `file` on both the .class and the JVM binary ---\n");

        try {
            // Check the .class file
            System.out.println("ElfCompare.class:");
            runCommand("file", "ElfCompare.class");

            // Check the java binary
            System.out.println("\n" + javaBinary + ":");
            runCommand("file", javaBinary);

            // Show magic bytes of both files
            System.out.println("\n--- Magic bytes comparison ---\n");

            System.out.println("ElfCompare.class (first 16 bytes):");
            runCommand("hexdump", "-C", "-n", "16", "ElfCompare.class");

            System.out.println("\n" + javaBinary + " (first 16 bytes):");
            runCommand("hexdump", "-C", "-n", "16", javaBinary);

        } catch (Exception e) {
            System.out.println("  (Could not run external command: " + e.getMessage() + ")");
            System.out.println("  Try running these manually:");
            System.out.println("    file ElfCompare.class");
            System.out.println("    file " + javaBinary);
        }

        System.out.println("\n=== Key Differences Summary ===\n");
        System.out.println("  Property          | .class file       | java binary (JVM)");
        System.out.println("  ------------------|-------------------|-------------------");
        System.out.println("  Format            | Java class format | ELF");
        System.out.println("  Magic bytes       | CA FE BA BE       | 7F 45 4C 46");
        System.out.println("  Contains          | JVM bytecode      | x86-64 machine code");
        System.out.println("  Executed by       | JVM               | OS kernel directly");
        System.out.println("  Platform-specific | No (portable)     | Yes (x86-64 Linux)");
        System.out.println("  Has ELF sections  | No                | Yes (.text, .data, etc.)");
        System.out.println("  `readelf` works   | No                | Yes");
        System.out.println("  `javap -c` works  | Yes               | No");

        System.out.println("\n=== Exploration Commands ===\n");
        System.out.println("Examine the .class file (NOT ELF):");
        System.out.println("  file ElfCompare.class                    # 'compiled Java class data'");
        System.out.println("  javap -c ElfCompare                      # Disassemble bytecode");
        System.out.println("  javap -v ElfCompare                      # Verbose: constant pool, etc.");
        System.out.println("  hexdump -C ElfCompare.class | head -8    # See CA FE BA BE magic");
        System.out.println("  readelf -h ElfCompare.class              # FAILS — not an ELF file!\n");

        System.out.println("Examine the JVM binary (IS ELF):");
        System.out.println("  file " + javaBinary);
        System.out.println("  readelf -h " + javaBinary);
        System.out.println("  readelf -S " + javaBinary + " | head -30");
        System.out.println("  ldd " + javaBinary);
        System.out.println("  nm -D " + javaBinary + " | head -20");
        System.out.println("  hexdump -C " + javaBinary + " | head -4  # See 7F 45 4C 46 magic");
    }

    /**
     * Helper to run a system command and print its output.
     * Demonstrates Runtime.exec() — the JVM delegates to the OS kernel
     * using fork+exec, which loads an ELF binary to run the command.
     */
    private static void runCommand(String... command) throws Exception {
        ProcessBuilder pb = new ProcessBuilder(command);
        pb.redirectErrorStream(true);
        Process process = pb.start();

        byte[] output = process.getInputStream().readAllBytes();
        System.out.print("  " + new String(output).replace("\n", "\n  "));
        if (!new String(output).endsWith("\n")) {
            System.out.println();
        }

        process.waitFor();
    }
}
