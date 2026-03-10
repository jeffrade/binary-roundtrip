// Module 1: Execution Models — Rust (Native Compiled via LLVM)
// =============================================================
//
// WHAT THIS FILE DEMONSTRATES:
//   Rust compiles to native machine code, just like C. However, Rust uses
//   LLVM as its compilation backend, and provides memory safety guarantees
//   at compile time through its ownership/borrowing system.
//
// THE COMPILATION MODEL:
//   hello.rs → [rustc frontend] → HIR → MIR → [LLVM] → Assembly → Object → ELF
//
//   1. Parsing:      rustc parses the .rs file into an AST.
//   2. HIR:          High-level Intermediate Representation (desugared Rust).
//   3. MIR:          Mid-level IR (used for borrow checking and optimization).
//   4. LLVM IR:      Rust's MIR is lowered to LLVM's IR (shared with Clang/C).
//   5. Optimization: LLVM runs its optimization passes (same as clang -O2!).
//   6. Code gen:     LLVM generates native assembly for your CPU.
//   7. Assembly:     The assembler produces an object file (.o).
//   8. Linking:      The linker produces the final ELF executable.
//
// WHAT TO OBSERVE:
//   - Compile:  rustc hello.rs -o hello_rust
//   - Run:      ./hello_rust
//   - Inspect:  file hello_rust      → "ELF 64-bit LSB pie executable"
//   - Inspect:  ldd hello_rust       → shows shared library dependencies
//   - Size:     ls -la hello_rust    → larger than C (Rust includes more runtime)
//   - Inspect:  objdump -d hello_rust | head -100  → native x86-64 instructions
//
// KEY INSIGHT:
//   Like C, the CPU runs Rust code directly — no VM, no interpreter.
//   Unlike C, Rust's compiler enforces memory safety at compile time.
//   The resulting binary is just as "native" as a C binary. Both Rust
//   and Clang (C compiler) share the same backend: LLVM.
//
// COMPARE WITH:
//   - C:     Also native compiled, but with a simpler pipeline (no LLVM by default with gcc).
//   - Java:  Compiles to bytecode, not native code. JVM runs the bytecode.
//   - Ruby:  Interpreted. MRI parses and runs source at runtime.

fn main() {
    println!("Hello from Rust!");
    println!();
    println!("Execution model: NATIVE COMPILED (via LLVM)");
    println!("  - rustc compiled this .rs file through multiple stages:");
    println!("    Source → HIR → MIR → LLVM IR → Assembly → Object → ELF");
    println!("  - The resulting binary is native machine code, like C.");
    println!("  - No VM, no interpreter — the CPU runs this directly.");
    println!();
    println!("Rust and C share a key property: native compilation.");
    println!("But Rust adds compile-time safety guarantees:");
    println!("  - No null pointer dereferences");
    println!("  - No use-after-free");
    println!("  - No data races");
    println!("  - No buffer overflows (checked at compile time)");
    println!();
    println!("Try these commands to explore:");
    println!("  file ./hello_rust          # See it's an ELF binary");
    println!("  ldd ./hello_rust           # See shared library deps");
    println!("  objdump -d ./hello_rust    # See x86-64 instructions");
    println!("  rustc --emit=llvm-ir hello.rs  # See the LLVM IR");
    println!("  rustc --emit=asm hello.rs      # See the assembly");
}
