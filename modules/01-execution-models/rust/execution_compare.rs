// Module 1: Execution Models — Rust Execution Comparison
// =======================================================
//
// WHAT THIS FILE DEMONSTRATES:
//   This program proves it's a native binary by inspecting itself.
//   It shows its own binary path, runs `file` on itself to confirm it's
//   an ELF executable, and explains how native execution differs from
//   interpreted (Ruby) and VM-based (Java) execution.
//
// HOW THIS DIFFERS FROM RUBY AND JAVA:
//
//   RUBY:
//     $ ruby hello.rb
//     → The `ruby` binary (a C program) is what the CPU actually runs.
//     → hello.rb is just text data that `ruby` reads and processes.
//     → The .rb file is parsed → AST → YARV bytecode → interpreted.
//     → You can't run `./hello.rb` without the ruby interpreter.
//
//   JAVA:
//     $ java Hello
//     → The `java` binary (a C++ program, the JVM) is what the CPU runs.
//     → Hello.class is bytecode data that the JVM loads and processes.
//     → Bytecode is interpreted, then hot paths are JIT-compiled.
//     → You can't run `./Hello.class` directly.
//
//   RUST (and C):
//     $ ./execution_compare
//     → THIS BINARY ITSELF is what the CPU runs. No middleman.
//     → The binary contains native x86-64 machine instructions.
//     → The kernel loads it into memory and jumps to the entry point.
//     → There is no interpreter, no VM, no separate runtime process.
//
// WHAT TO OBSERVE:
//   - Compile:  rustc execution_compare.rs -o execution_compare
//   - Run:      ./execution_compare
//   - Watch it inspect itself and prove it's a native ELF binary.

use std::env;
use std::process::Command;

fn main() {
    println!("=== Rust Execution Model: Native Binary Self-Inspection ===");
    println!();

    // ---------------------------------------------------------------
    // 1. Show our own binary path
    // ---------------------------------------------------------------
    // std::env::current_exe() returns the absolute path of the running
    // binary. This is possible because we ARE the binary — there's no
    // interpreter between us and the OS.
    println!("[1] Binary Path:");
    match env::current_exe() {
        Ok(path) => {
            println!("    This executable: {}", path.display());
            println!("    (This IS the program. Not an interpreter running a script.)");
            println!();

            // ---------------------------------------------------------------
            // 2. Run `file` on ourselves to show we're an ELF binary
            // ---------------------------------------------------------------
            // The `file` command examines the magic bytes and structure of a
            // file to determine its type. For native binaries, it will show
            // "ELF 64-bit LSB" — the Executable and Linkable Format used by
            // Linux for native executables.
            println!("[2] File Type (via `file` command):");
            match Command::new("file").arg(&path).output() {
                Ok(output) => {
                    let stdout = String::from_utf8_lossy(&output.stdout);
                    println!("    {}", stdout.trim());
                    println!();
                    // Point out key parts of the `file` output
                    if stdout.contains("ELF") {
                        println!("    Key observations:");
                        println!("    - 'ELF' = Executable and Linkable Format (Linux native binary)");
                    }
                    if stdout.contains("64-bit") {
                        println!("    - '64-bit' = compiled for x86-64 architecture");
                    }
                    if stdout.contains("dynamically linked") {
                        println!("    - 'dynamically linked' = uses shared libraries (.so files)");
                    }
                    if stdout.contains("pie") || stdout.contains("PIE") {
                        println!("    - 'PIE' = Position Independent Executable (ASLR compatible)");
                    }
                }
                Err(e) => println!("    Could not run `file`: {}", e),
            }
            println!();

            // ---------------------------------------------------------------
            // 3. Show the binary size
            // ---------------------------------------------------------------
            println!("[3] Binary Size:");
            match std::fs::metadata(&path) {
                Ok(meta) => {
                    let size = meta.len();
                    println!("    {} bytes ({:.1} KB)",
                             size, size as f64 / 1024.0);
                    println!("    (This includes Rust's standard library code,");
                    println!("     panic handling, and formatting machinery.)");
                }
                Err(e) => println!("    Could not stat: {}", e),
            }
            println!();

            // ---------------------------------------------------------------
            // 4. Show shared library dependencies
            // ---------------------------------------------------------------
            println!("[4] Shared Library Dependencies (via `ldd`):");
            match Command::new("ldd").arg(&path).output() {
                Ok(output) => {
                    let stdout = String::from_utf8_lossy(&output.stdout);
                    for line in stdout.lines() {
                        println!("    {}", line.trim());
                    }
                    println!();
                    println!("    These are the only external dependencies.");
                    println!("    Compare: `ldd $(which ruby)` to see Ruby's deps.");
                    println!("    The Ruby interpreter is a much larger native binary");
                    println!("    that must be present to run any .rb file.");
                }
                Err(e) => println!("    Could not run `ldd`: {}", e),
            }
        }
        Err(e) => println!("    Could not determine binary path: {}", e),
    }

    println!();

    // ---------------------------------------------------------------
    // 5. Summary comparison
    // ---------------------------------------------------------------
    println!("[5] Execution Model Comparison:");
    println!();
    println!("    +-----------+-------------------+------------------+---------------+");
    println!("    | Language  | What CPU Runs     | Your Code Is     | Persistent?   |");
    println!("    +-----------+-------------------+------------------+---------------+");
    println!("    | C         | Your binary       | Machine code     | Yes (.o, ELF) |");
    println!("    | Rust      | Your binary       | Machine code     | Yes (.o, ELF) |");
    println!("    | Java      | JVM (C++ binary)  | JVM bytecode     | Yes (.class)  |");
    println!("    | Ruby      | MRI (C binary)    | YARV bytecode    | No (in memory)|");
    println!("    +-----------+-------------------+------------------+---------------+");
    println!();
    println!("    Native (C/Rust): Source → Compiler → Machine Code → CPU runs it.");
    println!("    VM (Java):       Source → Compiler → Bytecode → JVM interprets/JITs it.");
    println!("    Interpreted (Ruby): Source → Parser → Bytecode (in RAM) → VM interprets it.");
}
