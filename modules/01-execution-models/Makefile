# ==============================================================================
# Module 1: Execution Models — Makefile
# ==============================================================================
#
# This Makefile builds and runs all the execution model demos.
# Use `make help` to see all available targets.
#
# Targets:
#   make all            — Build all compiled programs (C, Java, Rust)
#   make c              — Compile the C hello world
#   make java           — Compile both Java files
#   make rust           — Compile both Rust programs
#   make run-all        — Run ALL programs in sequence with banners
#   make ruby-bytecode  — Run the Ruby bytecode explorer
#   make java-bytecode  — Compile Hello.java and show javap -c output
#   make compare        — Run all hello worlds showing execution model diffs
#   make clean          — Remove all build artifacts
#   make help           — Show this help message
# ==============================================================================

# Compiler flags: strict warnings to enforce clean C code
CFLAGS := -Wall -Wextra -pedantic

# Output binaries
C_HELLO        := c/hello
JAVA_HELLO     := java/Hello.class
JAVA_DEMO      := java/ExecutionModelDemo.class
RUST_HELLO     := rust/hello_rust
RUST_COMPARE   := rust/execution_compare

# ==============================================================================
# Phony targets (these don't correspond to files)
# ==============================================================================
.PHONY: all c java rust run-all ruby-bytecode java-bytecode compare clean help

# ==============================================================================
# Build targets
# ==============================================================================

# Build everything that needs compiling (Ruby doesn't need compilation)
all: c java rust
	@echo ""
	@echo "All programs built successfully."
	@echo "Run 'make run-all' to execute them all, or 'make help' for options."

# Compile the C hello world to a native ELF binary
c: $(C_HELLO)

$(C_HELLO): c/hello.c
	@echo "=== Compiling C (native binary) ==="
	@echo "Command: gcc $(CFLAGS) -o $@ $<"
	gcc $(CFLAGS) -o $@ $<
	@echo "Produced: $@ ($$(file $@ | cut -d: -f2-))"
	@echo ""

# Compile both Java files to .class bytecode files
java: $(JAVA_HELLO) $(JAVA_DEMO)

$(JAVA_HELLO): java/Hello.java
	@echo "=== Compiling Java (bytecode for JVM) ==="
	@echo "Command: javac $<"
	javac $<
	@echo "Produced: $@ ($$(file $@ | cut -d: -f2-))"
	@echo ""

$(JAVA_DEMO): java/ExecutionModelDemo.java
	@echo "=== Compiling Java ExecutionModelDemo ==="
	@echo "Command: javac $<"
	javac $<
	@echo "Produced: $@ ($$(file $@ | cut -d: -f2-))"
	@echo ""

# Compile both Rust files to native ELF binaries via LLVM
rust: $(RUST_HELLO) $(RUST_COMPARE)

$(RUST_HELLO): rust/hello.rs
	@echo "=== Compiling Rust (native binary via LLVM) ==="
	@echo "Command: rustc $< -o $@"
	rustc $< -o $@
	@echo "Produced: $@ ($$(file $@ | cut -d: -f2-))"
	@echo ""

$(RUST_COMPARE): rust/execution_compare.rs
	@echo "=== Compiling Rust execution_compare ==="
	@echo "Command: rustc $< -o $@"
	rustc $< -o $@
	@echo "Produced: $@ ($$(file $@ | cut -d: -f2-))"
	@echo ""

# ==============================================================================
# Run targets
# ==============================================================================

# Run ALL programs in sequence with explanatory banners
run-all: all
	@echo ""
	@echo "######################################################################"
	@echo "# MODULE 1: EXECUTION MODELS — Running All Demos                    #"
	@echo "######################################################################"
	@echo ""
	@echo "======================================================================"
	@echo " 1/6  C — Native Compiled (gcc → ELF binary → CPU)"
	@echo "======================================================================"
	@./$(C_HELLO)
	@echo ""
	@echo "======================================================================"
	@echo " 2/6  Ruby — Interpreted (source → MRI parser → YARV bytecode → VM)"
	@echo "======================================================================"
	@ruby ruby/hello.rb
	@echo ""
	@echo "======================================================================"
	@echo " 3/6  Ruby — Bytecode Explorer (see the YARV instructions)"
	@echo "======================================================================"
	@ruby ruby/bytecode_explorer.rb "1 + 2"
	@echo ""
	@echo "======================================================================"
	@echo " 4/6  Java — Bytecode + VM (javac → .class → JVM)"
	@echo "======================================================================"
	@cd java && java Hello
	@echo ""
	@echo "======================================================================"
	@echo " 5/6  Java — Execution Model Demo (JIT compilation)"
	@echo "======================================================================"
	@cd java && java ExecutionModelDemo
	@echo ""
	@echo "======================================================================"
	@echo " 6/6  Rust — Native Compiled via LLVM (rustc → LLVM → ELF → CPU)"
	@echo "======================================================================"
	@./$(RUST_COMPARE)
	@echo ""
	@echo "======================================================================"
	@echo " ALL DEMOS COMPLETE"
	@echo "======================================================================"

# Run the Ruby bytecode explorer with the default expressions
ruby-bytecode:
	@echo "======================================================================"
	@echo " Ruby Bytecode Explorer"
	@echo " This shows the AST and YARV bytecode for Ruby expressions."
	@echo "======================================================================"
	@echo ""
	ruby ruby/bytecode_explorer.rb

# Compile Hello.java and show the bytecode disassembly with javap
java-bytecode: $(JAVA_HELLO)
	@echo "======================================================================"
	@echo " Java Bytecode Disassembly (javap -c Hello)"
	@echo "======================================================================"
	@echo ""
	@echo "This shows the JVM bytecode instructions in Hello.class."
	@echo "Compare this with YARV bytecode (make ruby-bytecode) — both are"
	@echo "stack-based VMs but with different instruction sets."
	@echo ""
	cd java && javap -c Hello
	@echo ""
	@echo "======================================================================"
	@echo " Key bytecode instructions above:"
	@echo "   getstatic     — Load System.out (a static field)"
	@echo "   ldc           — Load a constant (the string to print)"
	@echo "   invokevirtual — Call println() method"
	@echo "   return        — Return from main()"
	@echo "======================================================================"

# Run all hello worlds side-by-side, showing execution model differences
compare: all
	@echo ""
	@echo "######################################################################"
	@echo "# EXECUTION MODEL COMPARISON                                        #"
	@echo "######################################################################"
	@echo ""
	@echo "--- C (Native Compiled) ---"
	@echo "Binary: $$(file $(C_HELLO))"
	@echo "Size:   $$(ls -la $(C_HELLO) | awk '{print $$5}') bytes"
	@echo "Output:"
	@./$(C_HELLO) 2>&1 | head -3
	@echo ""
	@echo "--- Ruby (Interpreted + YARV VM) ---"
	@echo "Source: $$(file ruby/hello.rb)"
	@echo "Size:   $$(ls -la ruby/hello.rb | awk '{print $$5}') bytes (just text!)"
	@echo "Output:"
	@ruby ruby/hello.rb 2>&1 | head -3
	@echo ""
	@echo "--- Java (Bytecode + JVM) ---"
	@echo "Class:  $$(file java/Hello.class)"
	@echo "Size:   $$(ls -la java/Hello.class | awk '{print $$5}') bytes"
	@echo "Output:"
	@cd java && java Hello 2>&1 | head -3
	@echo ""
	@echo "--- Rust (Native Compiled via LLVM) ---"
	@echo "Binary: $$(file $(RUST_HELLO))"
	@echo "Size:   $$(ls -la $(RUST_HELLO) | awk '{print $$5}') bytes"
	@echo "Output:"
	@./$(RUST_HELLO) 2>&1 | head -3
	@echo ""
	@echo "======================================================================"
	@echo " Summary:"
	@echo "   C + Rust:  Compile to native machine code. CPU runs them directly."
	@echo "   Java:      Compiles to bytecode (.class). JVM interprets/JITs it."
	@echo "   Ruby:      Source parsed at runtime. YARV bytecode lives in memory."
	@echo "======================================================================"

# ==============================================================================
# Cleanup
# ==============================================================================

# Remove all compiled artifacts, leaving only source files
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(C_HELLO)
	rm -f java/*.class
	rm -f $(RUST_HELLO) $(RUST_COMPARE)
	@echo "Clean complete."

# ==============================================================================
# Help
# ==============================================================================

help:
	@echo "Module 1: Execution Models — Available Targets"
	@echo ""
	@echo "  Build:"
	@echo "    make all            Build everything (C, Java, Rust)"
	@echo "    make c              Compile C hello world"
	@echo "    make java           Compile Java files"
	@echo "    make rust           Compile Rust files"
	@echo ""
	@echo "  Run:"
	@echo "    make run-all        Run all programs with banners"
	@echo "    make ruby-bytecode  Run Ruby bytecode explorer"
	@echo "    make java-bytecode  Show Java bytecode disassembly"
	@echo "    make compare        Compare all execution models side by side"
	@echo ""
	@echo "  Maintenance:"
	@echo "    make clean          Remove all build artifacts"
	@echo "    make help           Show this help message"
