// elf_sections.rs — Rust program exploring ELF sections
// ============================================================================
//
// MODULE 3: ELF Binary Format
// EXERCISE: See that Rust produces standard ELF binaries, same sections as C
//
// CONCEPT:
//   Rust compiles to native code via LLVM, producing standard ELF binaries.
//   The same sections you saw in C (.text, .data, .bss, .rodata) appear in
//   Rust binaries too. Rust is not special at the binary level — it's just
//   another language that targets ELF.
//
// BUILD:
//   rustc -o elf_sections elf_sections.rs
//
// COMPARE TO C:
//   After building both this and elf_explorer.c, compare:
//     readelf -S elf_sections    # Rust binary sections
//     readelf -S elf_explorer    # C binary sections
//   You'll see the same fundamental sections: .text, .data, .bss, .rodata
//
//   However, Rust binaries are typically LARGER because:
//     1. Rust statically links its standard library (libstd) by default
//     2. Rust includes more metadata (panic info, type info for error messages)
//     3. Rust includes unwinding tables for proper cleanup
//
//   Compare sizes:
//     ls -lh elf_sections elf_explorer
//     nm elf_sections | wc -l     # Many more symbols (libstd is included!)
//     nm elf_explorer | wc -l     # Fewer symbols (libc is dynamically linked)
//
// WHAT TO OBSERVE:
//   readelf -h elf_sections         # ELF header — same format as C
//   readelf -S elf_sections         # Sections — .text, .rodata, .data, .bss
//   nm elf_sections | head -40      # Symbols — notice Rust name mangling
//   nm elf_sections | grep 'T main' # Find our main function
//   readelf -d elf_sections         # Dynamic section — fewer deps than C?

// ========================================================================
// .rodata — Constant data
// ========================================================================
// `static` with a constant initializer goes to .rodata (read-only data),
// just like `const char*` in C.
//
// OBSERVE: `nm elf_sections | grep GREETING`
//   The Rust name will be mangled, but you'll find it in the symbol table.
//   Alternatively: `objdump -s -j .rodata elf_sections | grep -i hello`
static GREETING: &str = "Hello from Rust's .rodata section!";

// `const` in Rust is a compile-time constant — it gets INLINED at every
// usage site. It may or may not appear in the binary as a distinct symbol.
// The compiler is free to embed the value directly in instructions.
//
// OBSERVE: `nm elf_sections | grep MAGIC` → might not appear!
//   Unlike C's `const int`, Rust's `const` is truly a compile-time literal.
const MAGIC_NUMBER: u64 = 0xDEADBEEF;

// ========================================================================
// .data — Initialized mutable global data
// ========================================================================
// `static mut` in Rust goes to .data, just like initialized globals in C.
// Accessing static mut is `unsafe` because Rust enforces thread safety.
//
// OBSERVE: `nm elf_sections | grep -i counter`
//   Look for a 'D' or 'd' symbol (initialized data section).
static mut GLOBAL_COUNTER: i32 = 42;

// ========================================================================
// .bss — Zero-initialized mutable global data
// ========================================================================
// This zero-initialized static goes to .bss — no disk space used.
// Same as `int uninitialized_buffer[1024]` in C.
//
// OBSERVE: `nm elf_sections | grep -i buffer`
//   Look for a 'B' or 'b' symbol (BSS section).
static mut ZERO_BUFFER: [u8; 4096] = [0u8; 4096];

// ========================================================================
// .text — Executable code
// ========================================================================
// Functions compile to machine code in .text, same as C.

/// A public function — will be a global symbol in .text
///
/// OBSERVE: `nm elf_sections | grep compute_sum`
///   Rust mangles names, so it might appear as something like:
///   _ZN12elf_sections11compute_sum...
///   The 'T' means global text (same as C).
///
/// To see demangled names:
///   nm elf_sections | rustfilt     # If you have rustfilt installed
///   nm --demangle elf_sections     # Recent nm versions can demangle Rust
fn compute_sum(a: i64, b: i64) -> i64 {
    a + b
}

/// Rust doesn't have `static` functions like C, but functions not marked `pub`
/// in a library crate are effectively local. In a binary crate like this,
/// all functions are local to the binary anyway.
///
/// However, the #[inline(never)] attribute prevents the compiler from
/// inlining this function, so it will definitely appear in the symbol table.
#[inline(never)]
fn helper_function(x: i64) -> i64 {
    x * x + 1
}

/// Demonstrates match (Rust's equivalent of switch), which may produce
/// a jump table in the assembly, just like C's switch statement.
#[inline(never)]
fn describe_number(n: i32) -> &'static str {
    match n {
        0 => "zero",
        1 => "one",
        2 => "two",
        3 => "three",
        _ => "other",
    }
}

fn main() {
    println!("=== Rust ELF Section Explorer ===\n");

    // Use the static/const data so the compiler keeps it
    println!("[.rodata] GREETING = \"{}\"", GREETING);
    println!("[.rodata] MAGIC_NUMBER = 0x{:X}", MAGIC_NUMBER);

    // Access mutable statics (unsafe in Rust)
    unsafe {
        println!("[.data]   GLOBAL_COUNTER = {}", GLOBAL_COUNTER);
        GLOBAL_COUNTER += 1;
        println!("[.data]   GLOBAL_COUNTER after increment = {}", GLOBAL_COUNTER);

        println!("[.bss]    ZERO_BUFFER[0] = {} (always 0)", ZERO_BUFFER[0]);
        println!("[.bss]    ZERO_BUFFER size = {} bytes (0 bytes on disk!)",
                 ZERO_BUFFER.len());
    }

    // Call functions (.text section)
    println!("[.text]   compute_sum(10, 20) = {}", compute_sum(10, 20));
    println!("[.text]   helper_function(7) = {}", helper_function(7));
    println!("[.text]   describe_number(2) = \"{}\"", describe_number(2));

    println!("\n=== Rust vs C ELF Comparison ===");
    println!("Both produce standard ELF binaries with the same sections.");
    println!("Key differences:");
    println!("  1. Rust statically links libstd → larger binary");
    println!("  2. Rust name-mangles symbols → harder to read in nm output");
    println!("  3. Rust includes unwinding tables → .eh_frame is larger");
    println!("  4. Rust's const is compile-time → may not appear in .rodata");
    println!("  5. Rust's static mut requires unsafe → .data access is guarded");

    // Get the path to our own binary for self-inspection
    let exe_path = std::env::current_exe().unwrap_or_default();
    let exe = exe_path.to_string_lossy();

    println!("\n=== Exploration Commands ===");
    println!("Run these on the compiled binary ({}):\n", exe);
    println!("  readelf -h {}           # ELF header", exe);
    println!("  readelf -S {}           # Section headers", exe);
    println!("  readelf -s {} | head -30 # Symbol table (lots of symbols!)", exe);
    println!("  nm {}                    # All symbols", exe);
    println!("  nm --demangle {}         # Demangled Rust names", exe);
    println!("  objdump -d {} | head -80 # Disassembly", exe);
    println!("  size {}                  # text/data/bss summary", exe);

    println!("\nCompare with C:");
    println!("  size elf_sections elf_explorer   # Side-by-side size comparison");
    println!("  nm elf_sections | wc -l          # Count Rust symbols");
    println!("  nm elf_explorer | wc -l          # Count C symbols");
    println!("  # Rust will have MANY more symbols (libstd is statically linked)");
}
