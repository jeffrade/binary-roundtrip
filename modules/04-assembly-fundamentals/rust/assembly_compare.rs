// assembly_compare.rs — Equivalent math functions to simple_math.c in Rust
// ============================================================================
//
// MODULE 4: Assembly Fundamentals (x86-64, read-only goal)
// EXERCISE: Compare Rust and C assembly output — they're nearly identical!
//
// CONCEPT:
//   Rust and C both compile to native machine code. At the assembly level,
//   simple arithmetic functions produce nearly identical output, especially
//   at higher optimization levels. The language differences (ownership,
//   lifetimes, etc.) are enforced at COMPILE TIME and add ZERO runtime cost.
//
//   This is Rust's core promise: zero-cost abstractions. The assembly proves it.
//
// BUILD:
//   rustc --emit=asm -C opt-level=0 -o assembly_compare.s assembly_compare.rs
//   rustc --emit=asm -C opt-level=2 -o assembly_compare_opt.s assembly_compare.rs
//
//   Or build a binary and disassemble:
//   rustc -C opt-level=0 -o assembly_compare assembly_compare.rs
//   objdump -d assembly_compare | grep -A 20 'add_values'
//
// COMPARE WITH C:
//   gcc -O2 -S -o simple_math_c.s simple_math.c
//   rustc --emit=asm -C opt-level=2 -o simple_math_rust.s assembly_compare.rs
//   diff simple_math_c.s simple_math_rust.s
//   # At -O2, the simple functions will be nearly identical!
//
// GODBOLT (Compiler Explorer):
//   Paste both the C and Rust code into https://godbolt.org/ side by side.
//   Select gcc and rustc with -O2. Compare the assembly output.
//   You'll see: identical or nearly identical instructions.
//
// WHAT TO OBSERVE:
//   1. Same calling convention — args in rdi/rsi, return in rax/eax
//   2. Same instructions — add, imul, cmp, jcc, ret
//   3. Same optimizations — cmov for conditionals, lea for add
//   4. Rust names are mangled — _ZN16assembly_compare3add... instead of 'add'
//   5. Rust may include more metadata sections (panic info, etc.)

// Prevent the compiler from inlining these functions.
// Without this, -O2 would inline everything into main, and there
// would be no separate function to examine in the disassembly.

/// add_values — Returns a + b
///
/// EXPECTED ASSEMBLY (at -O2):
///   lea    eax, [rdi+rsi]    ; eax = rdi + rsi
///   ret
///
/// This is IDENTICAL to the C version. The Rust compiler (LLVM) and
/// GCC both emit the same optimized instruction.
///
/// EXPECTED ASSEMBLY (at -O0):
///   push   rbp
///   mov    rbp, rsp
///   mov    [rbp-4], edi     ; Store arg a
///   mov    [rbp-8], esi     ; Store arg b
///   mov    eax, [rbp-4]     ; Load a
///   add    eax, [rbp-8]     ; Add b
///   pop    rbp
///   ret
///
/// Also identical to C at -O0. The language doesn't matter at this level.
#[inline(never)]
fn add_values(a: i32, b: i32) -> i32 {
    a + b
}

/// multiply_values — Returns a * b
///
/// EXPECTED ASSEMBLY (at -O2):
///   mov    eax, edi          ; eax = a
///   imul   eax, esi          ; eax *= b
///   ret
///
/// Same as C. 'imul' is 'imul' regardless of the source language.
#[inline(never)]
fn multiply_values(a: i32, b: i32) -> i32 {
    a * b
}

/// max_value — Returns the larger of a and b
///
/// EXPECTED ASSEMBLY (at -O2):
///   cmp    edi, esi          ; Compare a and b
///   mov    eax, esi          ; Assume result = b
///   cmovge eax, edi          ; If a >= b, result = a
///   ret
///
/// The compiler uses cmov (conditional move) to avoid a branch,
/// exactly as it does for the C version.
///
/// NOTE: In debug mode (opt-level=0), Rust adds overflow checks
/// (which C does not). You'll see extra instructions for panic
/// on overflow. These are removed at opt-level >= 1.
#[inline(never)]
fn max_value(a: i32, b: i32) -> i32 {
    if a > b { a } else { b }
}

/// subtract_values — Returns a - b
///
/// EXPECTED ASSEMBLY (at -O2):
///   mov    eax, edi
///   sub    eax, esi
///   ret
#[inline(never)]
fn subtract_values(a: i32, b: i32) -> i32 {
    a - b
}

/// absolute_value — Returns |x|
///
/// EXPECTED ASSEMBLY (at -O2):
///   mov    eax, edi
///   neg    eax               ; eax = -eax
///   cmovs  eax, edi          ; If result was negative, use original
///   ret
///
/// Or the compiler might use:
///   mov    eax, edi
///   cdq                      ; Sign-extend eax into edx
///   xor    eax, edx          ; Flip bits if negative
///   sub    eax, edx          ; Add 1 if was negative (two's complement trick)
///   ret
///
/// NOTE: In debug mode, Rust's .abs() on i32 panics on i32::MIN
/// because |i32::MIN| overflows. In release mode, it wraps.
/// This is an intentional Rust safety feature.
#[inline(never)]
fn absolute_value(x: i32) -> i32 {
    if x < 0 { -x } else { x }
}

/// sum_range — Sum of 1..=n using a for loop
///
/// At -O2, the compiler may recognize this pattern and emit:
///   n * (n + 1) / 2
/// instead of an actual loop. Same optimization as C.
///
/// At -O0, you'll see a loop with cmp and jle, same as C.
#[inline(never)]
fn sum_range(n: i32) -> i32 {
    let mut total = 0i32;
    for i in 1..=n {
        total += i;
    }
    total
}

fn main() {
    println!("=== Rust vs C Assembly Comparison ===\n");

    let a = 10i32;
    let b = 3i32;

    println!("add_values({}, {}) = {}", a, b, add_values(a, b));
    println!("subtract_values({}, {}) = {}", a, b, subtract_values(a, b));
    println!("multiply_values({}, {}) = {}", a, b, multiply_values(a, b));
    println!("max_value({}, {}) = {}", a, b, max_value(a, b));
    println!("absolute_value({}) = {}", -7, absolute_value(-7));
    println!("sum_range({}) = {}", 10, sum_range(10));

    println!("\n=== Key Observation ===");
    println!("At -O2, the assembly for these functions is IDENTICAL in C and Rust.");
    println!("Rust's safety features (bounds checks, overflow checks) are compile-time.");
    println!("They produce ZERO runtime overhead in release builds for basic math.\n");

    println!("=== Differences You WILL See ===");
    println!("1. Name mangling: Rust symbols are longer (e.g., _ZN16assembly_compare...)");
    println!("2. At -O0 only: Rust adds overflow checks (panic on integer overflow)");
    println!("3. Rust binary is larger (statically linked libstd)");
    println!("4. Rust includes more .eh_frame data (for unwinding on panic)\n");

    println!("=== Exploration Commands ===\n");
    println!("Build assembly for comparison:");
    println!("  rustc --emit=asm -C opt-level=0 -o rust_O0.s assembly_compare.rs");
    println!("  rustc --emit=asm -C opt-level=2 -o rust_O2.s assembly_compare.rs");
    println!("  gcc -O0 -S -o c_O0.s ../c/simple_math.c");
    println!("  gcc -O2 -S -o c_O2.s ../c/simple_math.c\n");
    println!("Compare side by side:");
    println!("  # Look at the add function in each file:");
    println!("  grep -A 10 'add_values' rust_O2.s");
    println!("  grep -A 10 '^add:' c_O2.s\n");
    println!("Interactive exploration:");
    println!("  Visit https://godbolt.org/ (Compiler Explorer)");
    println!("  Paste C code in one pane, Rust code in another");
    println!("  Set both to -O2 and compare the assembly output");
}
