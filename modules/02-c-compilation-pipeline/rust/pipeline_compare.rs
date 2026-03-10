// Module 2: C Compilation Pipeline — Rust Pipeline Comparison
// ============================================================
//
// WHAT THIS FILE DEMONSTRATES:
//   Rust has a compilation pipeline that is analogous to C's, but with
//   additional intermediate representations and safety analysis stages.
//
// C PIPELINE:
//   .c → [Preprocessor] → .i → [Compiler] → .s → [Assembler] → .o → [Linker] → ELF
//
// RUST PIPELINE:
//   .rs → [Parser] → AST
//        → [Name Resolution, Macro Expansion] → AST
//        → [Lowering] → HIR (High-level IR)
//        → [Type Checking, Trait Resolution] → Typed HIR
//        → [Lowering] → MIR (Mid-level IR)
//        → [Borrow Checking] → Verified MIR       ← Rust's key safety step!
//        → [Lowering] → LLVM IR
//        → [LLVM Optimization Passes] → Optimized LLVM IR
//        → [LLVM Code Generation] → Assembly (.s)
//        → [Assembler] → Object File (.o)
//        → [Linker] → ELF Binary
//
// KEY DIFFERENCES FROM C:
//   1. NO PREPROCESSOR: Rust has a macro system, but it operates on the AST,
//      not on raw text. Rust macros are hygienic and type-aware.
//
//   2. HIR (High-level IR): A desugared version of Rust. `for` loops become
//      `loop` + `match`, `if let` becomes `match`, etc.
//
//   3. MIR (Mid-level IR): A simplified control-flow graph used for:
//      - BORROW CHECKING (Rust's memory safety analysis)
//      - Optimizations (constant propagation, dead code elimination)
//      - Code generation
//      This is the stage where Rust catches use-after-free, data races, etc.
//
//   4. LLVM IR: Both Rust (rustc) and C (clang) lower to LLVM IR. From this
//      point forward, the pipeline is IDENTICAL. The same LLVM optimization
//      passes and code generator produce the final binary.
//
// HOW TO VIEW EACH STAGE:
//   rustc --emit=mir pipeline_compare.rs         # MIR (borrow-checked IR)
//   rustc --emit=llvm-ir pipeline_compare.rs     # LLVM IR (shared with C/clang)
//   rustc --emit=asm pipeline_compare.rs         # x86-64 Assembly
//   rustc --emit=obj pipeline_compare.rs         # Object file (.o)
//   rustc pipeline_compare.rs -o pipeline_compare # Final ELF binary
//
//   Or use the Makefile: `make rust-pipeline`

/// A non-trivial function to make intermediate representations interesting.
///
/// In the MIR output, you'll see:
///   - Explicit borrow checking annotations
///   - Control flow as a graph of basic blocks (bb0, bb1, bb2, ...)
///   - Explicit drops (Rust's deterministic destruction)
///
/// In the LLVM IR output, you'll see:
///   - SSA (Static Single Assignment) form
///   - LLVM types (i32, i64, etc.)
///   - Function calls and control flow as br/switch instructions
///
/// In the assembly output, you'll see:
///   - x86-64 instructions (mov, add, cmp, jle, call, ret)
///   - Stack frame management
///   - The loop structure as compare + conditional jump
fn sum_of_squares(n: u64) -> u64 {
    let mut sum: u64 = 0;
    let mut i: u64 = 1;
    while i <= n {
        // In MIR, this multiplication and addition become explicit
        // temporary variables with borrow-checking annotations.
        // In LLVM IR, these become SSA values (%1, %2, %3, ...).
        // In assembly, these become mov/imul/add instructions.
        sum += i * i;
        i += 1;
    }
    sum
}

/// This function demonstrates ownership — Rust's unique feature.
///
/// In MIR output, you'll see explicit "move" and "drop" operations.
/// The borrow checker verifies that:
///   - `s` is moved into this function (caller can't use it after)
///   - The String is dropped (freed) at the end of this function
///   - No dangling references exist
///
/// C has no equivalent of this analysis — it's the programmer's
/// responsibility to free memory and avoid dangling pointers.
fn demonstrate_ownership(s: String) -> usize {
    let len = s.len();
    // MIR will show: _1 = move _2  (ownership transfer)
    // At the end of this function, MIR shows: drop(_1) (String freed)
    println!("  Borrowed string: '{}', length: {}", s, len);
    len
    // `s` is dropped here — memory freed deterministically.
    // In MIR: StorageDead(_1) followed by drop.
}

fn main() {
    println!("=== Rust Compilation Pipeline ===");
    println!();

    // ------------------------------------------------------------------
    // Explain the pipeline stages
    // ------------------------------------------------------------------
    println!("Rust's compilation pipeline (compared to C):");
    println!();
    println!("  C Pipeline:");
    println!("    .c → Preprocessor → .i → Compiler → .s → Assembler → .o → Linker → ELF");
    println!();
    println!("  Rust Pipeline:");
    println!("    .rs → Parser → AST");
    println!("        → HIR (High-level IR) — desugared Rust");
    println!("        → MIR (Mid-level IR) — borrow checking happens here!");
    println!("        → LLVM IR — same as clang/C from here on");
    println!("        → Assembly (.s)");
    println!("        → Object (.o)");
    println!("        → ELF binary");
    println!();

    // ------------------------------------------------------------------
    // Run the non-trivial function
    // ------------------------------------------------------------------
    let n = 10;
    let result = sum_of_squares(n);
    println!("sum_of_squares({}) = {}", n, result);
    println!("  (Look at the MIR/LLVM IR/ASM for this function — it's a good");
    println!("   example of how high-level code transforms through each stage.)");
    println!();

    // ------------------------------------------------------------------
    // Demonstrate ownership (unique to Rust's MIR stage)
    // ------------------------------------------------------------------
    let greeting = String::from("Hello from Rust's pipeline!");
    println!("Demonstrating ownership (visible in MIR output):");
    let len = demonstrate_ownership(greeting);
    // If you uncomment the next line, it won't compile:
    // println!("{}", greeting);  // ERROR: value moved into demonstrate_ownership
    println!("  String length was: {} (string has been freed — ownership moved)", len);
    println!();

    // ------------------------------------------------------------------
    // Print commands to view each stage
    // ------------------------------------------------------------------
    println!("To explore each stage, run:");
    println!();
    println!("  # MIR — Rust's mid-level IR (shows borrow checking, drops)");
    println!("  rustc --emit=mir pipeline_compare.rs");
    println!("  # Look for: _1 = move, drop(_1), StorageLive, StorageDead");
    println!();
    println!("  # LLVM IR — Shared with Clang/C (SSA form, LLVM types)");
    println!("  rustc --emit=llvm-ir pipeline_compare.rs");
    println!("  # Look for: define, i32, i64, br, call, ret");
    println!();
    println!("  # Assembly — x86-64 instructions (same output format as gcc -S)");
    println!("  rustc --emit=asm pipeline_compare.rs");
    println!("  # Look for: mov, add, imul, cmp, jle, call, ret");
    println!();
    println!("  # Object file — Relocatable ELF (same as gcc -c)");
    println!("  rustc --emit=obj pipeline_compare.rs");
    println!("  file pipeline_compare.o   # 'ELF 64-bit LSB relocatable'");
    println!("  nm pipeline_compare.o     # shows symbols (T, U, etc.)");
    println!();
    println!("Or use: make rust-pipeline  (runs all of the above)");
}
