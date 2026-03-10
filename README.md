# Binary Roundtrip

A hands-on course that takes you from source code to running binary — and back again through reverse engineering. 10 modules, ~41 hours, four languages (C, Rust, Java, Ruby), all interactive.

## Who This Is For

Application-layer developers who write Ruby/Rails or Java daily but have near-zero visibility into what happens below the surface — compilation, linking, ELF binaries, assembly, syscalls, memory layout, and debugging at the machine level.

## Quick Start

```bash
# 1. Check what tools you have
bash setup.sh --check-only

# 2. Install anything missing
bash setup.sh

# 3. Start Module 1
cd modules/01-execution-models
make help       # see what's available
make run-all    # run hello world in C, Ruby, Java, and Rust side by side
```

## Course Map

**Do modules 1-6 in order** — each builds on the previous. Modules 7-10 are independent after that.

| Module | Topic | Hours | What You'll Do |
|--------|-------|-------|----------------|
| **01** | Execution Models | 3h | Run the same program in 4 languages, see bytecode (YARV, JVM), native binary |
| **02** | C Compilation Pipeline | 4h | Watch a `.c` file transform through preprocessing → assembly → object → ELF |
| **03** | ELF Binary Format | 3h | Crack open a binary with `readelf`, build a shared library, compare static vs dynamic |
| **04** | Assembly Fundamentals | 5h | Read x86-64 disassembly, understand registers, calling conventions, stack frames |
| **05** | Linux Binary Execution | 4h | See what happens before `main()`, make raw syscalls, build a mini shell |
| **06** | Debuggers | 4h | Navigate GDB at the instruction level — find bugs, diagnose segfaults, inspect stripped binaries |
| **07** | Rust / LLVM Deep Dive | 6h | Trace `.rs` → MIR → LLVM IR → Assembly → ELF, prove zero-cost abstractions |
| **08** | JVM Essentials | 3h | Watch JIT warmup, inspect bytecode, see lazy class loading, measure GC costs |
| **09** | Ruby MRI Essentials | 3h | Explore YARV bytecode, hit the GIL, build a C extension, trace `require` overhead |
| **10** | Reverse Engineering | 6h | Crack a stripped binary, solve a crackme, run the full RE workflow |

## How Each Module Works

Every module has a `Makefile` with targets that ARE the lessons:

```bash
cd modules/02-c-compilation-pipeline
make help           # list all targets
make pipeline       # run all 4 compilation stages with explanatory output
make macros         # see conditional compilation with -DDEBUG
make multi          # see why linking exists (separate compilation units)
```

Every source file has extensive comments explaining what to observe. Read the comments, run the make target, see the output, understand the concept.

## Using the AI Teacher

`TEACHER.md` is a system prompt that turns Claude into a Socratic CS instructor for this course. It knows every module, every make target, and teaches by asking you to predict before explaining.

### Starting a Session w/ Claude Code (visit https://claude.ai to install)

Open Claude Code in this directory and reference the teacher prompt:

```
git clone git@github.com:jeffrade/binary-roundtrip
cd binary-roundtrip
claude

# Then in the Claude Code session:
> @TEACHER.md I'm ready to start the course. Teach me Module 1.
```

Claude will read the module's source files, walk you through exercises using the Makefile targets, ask you to predict what will happen, and explain what you observed — connecting everything back to Ruby/Rails/Java.

### Example First Prompts

```
# Start from the beginning
> @TEACHER.md Teach me Module 1 — Execution Models

# Jump to a specific module (after completing 1-6)
> @TEACHER.md I've finished modules 1-6. Start me on Module 7 — Rust/LLVM Deep Dive

# Ask about a specific concept
> @TEACHER.md I'm in Module 3. Explain what .plt and .got sections do and show me with make dynamic

# Get help with an exercise
> @TEACHER.md I'm stuck on the crackme in Module 10. Give me a hint using strings and objdump
```

### Continuing Between Sessions

Claude Code doesn't carry state between sessions, so when you start a new session, tell it where you left off:

```
> @TEACHER.md I completed modules 1-4 last session. I'm starting Module 5 — How Linux Runs a Binary.
```

The teacher will pick up from there, briefly recap what you've learned so far, and continue with the next module's exercises.

## Key Make Targets by Module

| Module | Must-Run Targets |
|--------|-----------------|
| 01 | `make run-all`, `make ruby-bytecode`, `make java-bytecode`, `make compare` |
| 02 | `make pipeline`, `make macros`, `make multi`, `make rust-pipeline` |
| 03 | `make elf-dump`, `make shared-lib`, `make static-link`, `make java-compare` |
| 04 | `make assembly`, `make annotated`, `make optimized`, `make compare` |
| 05 | `make strace`, `make memory`, `make entry`, `make mini-shell` |
| 06 | `make gdb-debug`, `make gdb-buggy`, `make gdb-segfault`, `make compare-stripped` |
| 07 | `make pipeline`, `make zero-cost`, `make compare`, `make llvm-ir` |
| 08 | `make jit`, `make bytecode`, `make classloading`, `make gc` |
| 09 | `make bytecode`, `make gil`, `make c-extension`, `make require-trace` |
| 10 | `make crackme`, `make mystery`, `make workflow` |

## Prerequisites

**Knowledge:** Strong application-layer development in Ruby or Java. No systems knowledge required — that's what this course teaches.

**Tools:** GCC, GDB, JDK, Ruby, Rust, binutils (readelf, objdump, nm, strings), strace, ltrace. Run `bash setup.sh` to install everything.

**Platform:** Linux x86-64. The course covers x86-64 assembly and Linux-specific concepts (ELF, `/proc`, `execve`).

## Reference Books

If you buy one book: **CS:APP** (Bryant & O'Hallaron, 3rd edition) — covers modules 2-6 in one coherent arc.

Other references per module are listed in `syllabus.md`.
