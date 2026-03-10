# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Project Is

**Binary Roundtrip** — an interactive 10-module (~41h) course that goes from source code to running binary and back again through reverse engineering. Teaches systems-level concepts (compilation, linking, ELF, assembly, syscalls, debugging, RE) to application-layer developers. Each module has exercises in C, Rust, Java, and Ruby with Makefiles that are themselves teaching tools.

## Build System

Plain Makefiles at every level — no CMake, Gradle, or Cargo workspaces.

```bash
make check          # Verify all tools installed (runs setup.sh --check-only)
make setup          # Install missing tools
make module-NN      # Build a specific module (01-10)
make all            # Build all modules
make clean          # Clean all artifacts
```

Each module has its own Makefile with `make help` listing all targets. Module Makefiles are instructional — they echo commands, show analysis output, and include comparison targets across languages.

## Module Dependencies

Modules 1-6 are sequential (each builds on the previous). Modules 7-10 are independent after completing 1-6.

```
01 Execution Models → 02 C Pipeline → 03 ELF → 04 Assembly → 05 Linux Execution → 06 Debuggers
                                                                                      ↓
                                                                        07 Rust/LLVM | 08 JVM | 09 Ruby MRI | 10 RE
```

## Architecture

```
Makefile              # Dispatches to module Makefiles via: make -C modules/NN-name/ all
setup.sh              # Tool checker/installer (shellcheck-clean bash)
TEACHER.md            # Socratic teaching agent prompt with per-module guidance
syllabus.md           # Full curriculum spec with student assessment
modules/NN-name/
├── Makefile          # Per-module targets — each target demonstrates a concept
├── c/                # C source files
├── rust/             # Rust source files
├── java/             # Java source files
└── ruby/             # Ruby source files (Module 9 includes ruby/c_extension/)
```

Every source file has a 20-50 line header comment explaining what it teaches, what commands to run, and what to observe. This is intentional — do not strip or reduce these comments.

## Key Conventions

- **Multi-language comparison is central**: every concept is shown in multiple languages so students see how C, Rust, Java, and Ruby handle the same problem differently
- **Makefile targets ARE lessons**: each `make` target teaches a concept — they echo commands, pipe through analysis tools (readelf, objdump, nm, javap, strace), and print explanatory banners
- **Comment structure is consistent across languages**: `WHAT THIS FILE DEMONSTRATES`, `WHAT TO OBSERVE`, `KEY INSIGHT`, `COMPARE WITH` sections
- **C compilation flags**: use `gcc -Wall -Wextra` (some files add `-pedantic`). Module 6's `find_the_bug.c` uses `-Wno-uninitialized` intentionally
- **Module 10 crackme encoding**: `crackme.c` uses XOR+offset encoding (not strcmp) — do not simplify, it's designed to resist `strings`
- **Rust emit flags**: `rustc --emit=llvm-ir`, `--emit=asm`, `--emit=mir`, `--emit=obj` for pipeline exploration
- **Java analysis**: `javap -c` (bytecode), `javap -c -verbose` (full), `-XX:+PrintCompilation` (JIT)

## Tool Dependencies

Required: `gcc`, `g++`, `make`, `ld`, `as`, `readelf`, `objdump`, `nm`, `ldd`, `strings`, `file`, `strace`, `ltrace`, `hexdump`, `gdb`, `java`/`javac`/`javap`, `ruby`/`irb`, `rustc`/`cargo`. Optional: `ghidra`, `r2`, `nasm`. Run `bash setup.sh --check-only` to verify.

## Teaching Rules (When Acting as Instructor)

Follow TEACHER.md — use Socratic method: ask students to predict before running, run code before explaining theory, connect every concept to Ruby/Java that the student already knows. Never skip modules 1-6. The `make compare` targets in each module are the key "aha moment" drivers.

## Git Rules

Never run git write operations (`git add`, `git commit`, `git push`, `git reset`, etc.). Read-only git commands only (`git status`, `git log`, `git diff`, `git show`, `git branch`).

## Known Limitations

- **JRuby not in setup.sh**: Module 08's `make jruby` target requires JRuby, which is not installed by `setup.sh`. The target will fail gracefully with a message but won't run the comparison.
- **YJIT needs Ruby 3.1+**: Module 09's `make yjit` target requires Ruby 3.1+ with YJIT support. No version check exists — it will error on older Ruby.
- **no_std demo is inspect-only**: Module 07's `rust/no_std_demo.rs` includes a custom panic handler and raw syscalls for bare-metal Rust. It compiles for inspection (`rustc --emit=asm`) but the Makefile doesn't attempt to link/run it as a standalone binary.
- **`make all` only compiles**: Running `make all` at the top level or per-module only builds binaries. It does NOT run the instructional targets (`make pipeline`, `make strace`, etc.) — those must be run individually as part of the learning flow.

## Verification

No test suite — verify by compiling: `gcc -Wall -Wextra` for C, `rustc` for Rust, `javac` for Java; run `ruby <file>` for Module 9 Ruby scripts. Each module's `make all` target builds everything. Do not leave behind temporary verification scripts.

## Shell Scripts

Run `shellcheck` on any changes to shell scripts and resolve all issues before considering the change complete.
