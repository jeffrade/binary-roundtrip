# TODO — Binary Roundtrip

Actionable improvements for the course. Check items off as completed.

## High Priority — Test All Instructional Targets

`make all` only compiles. Each instructional `make` target below needs to be run manually to verify it produces correct, useful output. Targets that require interactive tools (GDB) should be verified by inspecting the commands they print.

### Module 01 — Execution Models
- [ ] `make run-all` — runs hello world in C, Ruby, Java, Rust
- [ ] `make ruby-bytecode` — shows YARV bytecode via `ruby --dump insns`
- [ ] `make java-bytecode` — shows JVM bytecode via `javap -c`
- [ ] `make compare` — side-by-side comparison of all four execution models

### Module 02 — C Compilation Pipeline
- [ ] `make pipeline` — full 4-stage walkthrough (preprocess → compile → assemble → link)
- [ ] `make preprocess` / `make compile` / `make assemble` / `make link` — individual stages
- [ ] `make macros` — conditional compilation with/without -DDEBUG
- [ ] `make multi` — separate compilation units and linking
- [ ] `make rust-pipeline` — Rust equivalent of the C pipeline

### Module 03 — ELF Binary Format
- [ ] `make elf-dump` — full readelf output
- [ ] `make sections` / `make symbols` / `make disasm` — focused ELF analysis
- [ ] `make dynamic` — static vs dynamic linking comparison
- [ ] `make shared-lib` — build and use a .so
- [ ] `make static-link` — static linking demo
- [ ] `make rust-elf` — Rust ELF compared to C
- [ ] `make java-compare` — .class file is NOT ELF
- [ ] `make hexdump` — raw hex view of binary

### Module 04 — Assembly Fundamentals
- [ ] `make assembly` — generate .s files from all C sources
- [ ] `make annotated` — gcc-annotated assembly (C lines interleaved)
- [ ] `make optimized` — O0 vs O2 comparison
- [ ] `make calling-conv` — System V AMD64 ABI demo
- [ ] `make stack` — stack frame visualization
- [ ] `make control` — control flow in assembly
- [ ] `make rust-asm` — Rust assembly compared to C
- [ ] `make java-bytecode` — JVM stack-based vs x86 register-based
- [ ] `make compare` — C vs Rust vs Java comparison

### Module 05 — Linux Binary Execution
- [ ] `make syscall` — direct syscall demo
- [ ] `make exec` — execve demonstration
- [ ] `make memory` — /proc/self/maps virtual address space
- [ ] `make entry` — _start → __libc_start_main → main chain
- [ ] `make mini-shell` — fork/exec mini shell (interactive — verify it builds and starts)
- [ ] `make strace` / `make strace-startup` — strace output
- [ ] `make maps` — memory layout inspection
- [ ] `make dynamic-linker` — manual ld-linux invocation
- [ ] `make rust-syscall` — Rust raw syscall

### Module 06 — Debuggers
- [ ] `make debug` / `make buggy` / `make segfault` / `make stripped` — build targets
- [ ] `make gdb-debug` — prints GDB commands for Exercise 1
- [ ] `make gdb-buggy` — prints GDB commands for Exercise 2 (find the bug)
- [ ] `make gdb-segfault` — prints GDB commands for Exercise 3 (backtrace)
- [ ] `make compare-stripped` — stripped vs unstripped nm/objdump comparison
- [ ] `make rust-debug` — Rust debug binary
- [ ] Verify `gdb_exercises.txt` instructions match actual binary behavior

### Module 07 — Rust/LLVM Deep Dive
- [ ] `make pipeline` — full .rs → MIR → LLVM IR → ASM → ELF chain
- [ ] `make zero-cost` — iterator vs hand-written loop assembly comparison
- [ ] `make compare` — Rust vs C equivalent assembly
- [ ] `make llvm-ir` — LLVM IR output and explanation
- [ ] `make generics` — monomorphization demo
- [ ] `make closures` — closure assembly output
- [ ] `make ownership` — ownership/borrowing at the assembly level
- [ ] `make no-std` — bare-metal Rust (inspect-only, does not link a runnable binary)

### Module 08 — JVM Essentials
- [ ] `make classloading` — lazy class loading demo
- [ ] `make jit` — JIT warmup curve timing
- [ ] `make bytecode` — javap -c output with explanation
- [ ] `make gc` — GC pause demonstration
- [ ] `make native-compare` — JVM vs native comparison
- [ ] `make jruby` — JRuby comparison (requires JRuby — will fail without it)
- [ ] `make decompile` — decompile .class back to source

### Module 09 — Ruby MRI Essentials
- [ ] `make bytecode` — YARV bytecode dump
- [ ] `make ast` — Ruby AST exploration
- [ ] `make gil` — GIL/GVL threading demo
- [ ] `make yjit` — YJIT demo (requires Ruby 3.1+)
- [ ] `make internals` — MRI internals exploration
- [ ] `make require-trace` — require chain tracing
- [ ] `make c-extension` — build the C extension (.so)
- [ ] `make test-extension` — run the C extension tests
- [ ] `make benchmark` — pure Ruby vs C extension benchmark

### Module 10 — Reverse Engineering
- [ ] `make crackme` — build crackme challenge binary
- [ ] `make mystery` — build mystery binary
- [ ] `make function-finder` — build function identification challenge
- [ ] `make anti-debug` — build anti-debug demo
- [ ] `make rust-stripped` — stripped Rust binary
- [ ] `make workflow` — full RE workflow on mystery binary
- [ ] `make strings-crackme` / `make strace-mystery` / `make ltrace-mystery` — analysis targets
- [ ] `make disasm-crackme` / `make disasm-mystery` / `make disasm-function-finder` — disassembly
- [ ] `make solutions` — verify solutions output matches expectations
- [ ] Manually solve the crackme to verify it works (password is XOR+offset encoded, not plaintext)

---

## Medium Priority — Missing Tool Support

- [ ] **Add JRuby to setup.sh**: Module 08's `make jruby` requires JRuby but `setup.sh` doesn't install it. Add a check and install via `apt-get install jruby` or document manual install.
- [ ] **Add Ruby version check for YJIT**: Module 09's `make yjit` needs Ruby 3.1+. Add a runtime version check in the Makefile target or the Ruby script that prints a clear error on older Ruby.
- [ ] **Verify no_std_demo.rs Makefile target**: Module 07's `make no-std` should emit assembly for inspection without trying to link a runnable binary. Verify the Makefile handles this correctly.

---

## Low Priority — Enhancements

- [ ] **Student progress checklist**: Add a `PROGRESS.md` template students can copy and check off as they complete modules/targets.
- [ ] **End-to-end smoke test script**: A script that runs every non-interactive `make` target across all modules and reports pass/fail. Would catch regressions if source files are edited.
- [ ] **Ghidra walkthrough for Module 10**: The RE module mentions Ghidra but has no guided exercise. Could add a `ghidra_walkthrough.txt` similar to `gdb_exercises.txt` in Module 06.
- [ ] **YJIT before/after benchmark**: Module 09 could show a concrete performance comparison with YJIT enabled vs disabled on the same workload.
