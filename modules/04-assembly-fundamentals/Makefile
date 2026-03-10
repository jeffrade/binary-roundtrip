# ==============================================================================
# MODULE 4: Assembly Fundamentals (x86-64, read-only goal) — Makefile
# ==============================================================================
#
# This Makefile builds all exercises and provides targets for generating
# and comparing assembly output from C, Rust, and Java.
#
# Quick Start:
#   make all            — Build all binaries
#   make assembly       — Generate assembly (-O0 -S) for all C files
#   make annotated      — Generate verbose assembly with C variable names
#   make optimized      — Compare -O0 vs -O2 assembly
#   make calling-conv   — Examine calling convention register usage
#   make stack          — Examine stack frame prologue/epilogue
#   make control        — Examine control flow patterns
#   make disasm         — Disassemble compiled binaries with objdump
#   make rust-asm       — Generate Rust assembly and compare
#   make java-bytecode  — Compile Java and show bytecode (javap -c)
#   make compare        — Side-by-side: C assembly vs Rust assembly vs Java bytecode
#   make clean          — Remove all build artifacts
#
# ==============================================================================

CC      = gcc
CFLAGS  = -Wall -Wextra
RUSTC   = rustc
JAVAC   = javac

# Directories
C_DIR    = c
RUST_DIR = rust
JAVA_DIR = java

# C source files
C_SOURCES = $(C_DIR)/simple_math.c \
            $(C_DIR)/calling_convention.c \
            $(C_DIR)/stack_frames.c \
            $(C_DIR)/control_flow.c

# C binaries (compiled with -O0 for readable disassembly)
SIMPLE_MATH  = $(C_DIR)/simple_math
CALLING_CONV = $(C_DIR)/calling_convention
STACK_FRAMES = $(C_DIR)/stack_frames
CONTROL_FLOW = $(C_DIR)/control_flow

# Assembly output files
SIMPLE_MATH_S     = $(C_DIR)/simple_math.s
SIMPLE_MATH_VS    = $(C_DIR)/simple_math_verbose.s
SIMPLE_MATH_OPT   = $(C_DIR)/simple_math_opt.s
CALLING_CONV_S    = $(C_DIR)/calling_convention.s
CALLING_CONV_VS   = $(C_DIR)/calling_convention_verbose.s
STACK_FRAMES_S    = $(C_DIR)/stack_frames.s
STACK_FRAMES_VS   = $(C_DIR)/stack_frames_verbose.s
CONTROL_FLOW_S    = $(C_DIR)/control_flow.s
CONTROL_FLOW_VS   = $(C_DIR)/control_flow_verbose.s
CONTROL_FLOW_OPT  = $(C_DIR)/control_flow_opt.s

# Rust targets
RUST_ASM_BIN  = $(RUST_DIR)/assembly_compare
RUST_ASM_S    = $(RUST_DIR)/assembly_compare.s
RUST_ASM_OPT  = $(RUST_DIR)/assembly_compare_opt.s

# Java targets
JAVA_CLASS = $(JAVA_DIR)/StackVsRegister.class

# ==============================================================================
# Primary targets
# ==============================================================================

.PHONY: all clean assembly annotated optimized calling-conv stack control \
        disasm rust-asm java-bytecode compare

all: $(SIMPLE_MATH) $(CALLING_CONV) $(STACK_FRAMES) $(CONTROL_FLOW) \
     $(RUST_ASM_BIN) $(JAVA_CLASS)
	@echo ""
	@echo "=== Module 4: All targets built successfully ==="
	@echo "Run 'make assembly' to generate assembly files for exploration."

# ==============================================================================
# Build rules — binaries
# ==============================================================================

$(SIMPLE_MATH): $(C_DIR)/simple_math.c
	$(CC) $(CFLAGS) -O0 -o $@ $<
	@echo "Built: $@ (with -O0 for readable disassembly)"

$(CALLING_CONV): $(C_DIR)/calling_convention.c
	$(CC) $(CFLAGS) -O0 -o $@ $<
	@echo "Built: $@"

$(STACK_FRAMES): $(C_DIR)/stack_frames.c
	$(CC) $(CFLAGS) -O0 -o $@ $<
	@echo "Built: $@"

$(CONTROL_FLOW): $(C_DIR)/control_flow.c
	$(CC) $(CFLAGS) -O0 -o $@ $<
	@echo "Built: $@"

$(RUST_ASM_BIN): $(RUST_DIR)/assembly_compare.rs
	$(RUSTC) -C opt-level=0 -o $@ $<
	@echo "Built: $@"

$(JAVA_CLASS): $(JAVA_DIR)/StackVsRegister.java
	$(JAVAC) -d $(JAVA_DIR) $<
	@echo "Built: $@"

# ==============================================================================
# Build rules — assembly files
# ==============================================================================

$(SIMPLE_MATH_S): $(C_DIR)/simple_math.c
	$(CC) $(CFLAGS) -O0 -S -o $@ $<

$(SIMPLE_MATH_VS): $(C_DIR)/simple_math.c
	$(CC) $(CFLAGS) -O0 -S -fverbose-asm -o $@ $<

$(SIMPLE_MATH_OPT): $(C_DIR)/simple_math.c
	$(CC) $(CFLAGS) -O2 -S -o $@ $<

$(CALLING_CONV_S): $(C_DIR)/calling_convention.c
	$(CC) $(CFLAGS) -O0 -S -o $@ $<

$(CALLING_CONV_VS): $(C_DIR)/calling_convention.c
	$(CC) $(CFLAGS) -O0 -S -fverbose-asm -o $@ $<

$(STACK_FRAMES_S): $(C_DIR)/stack_frames.c
	$(CC) $(CFLAGS) -O0 -S -o $@ $<

$(STACK_FRAMES_VS): $(C_DIR)/stack_frames.c
	$(CC) $(CFLAGS) -O0 -S -fverbose-asm -o $@ $<

$(CONTROL_FLOW_S): $(C_DIR)/control_flow.c
	$(CC) $(CFLAGS) -O0 -S -o $@ $<

$(CONTROL_FLOW_VS): $(C_DIR)/control_flow.c
	$(CC) $(CFLAGS) -O0 -S -fverbose-asm -o $@ $<

$(CONTROL_FLOW_OPT): $(C_DIR)/control_flow.c
	$(CC) $(CFLAGS) -O2 -S -o $@ $<

$(RUST_ASM_S): $(RUST_DIR)/assembly_compare.rs
	$(RUSTC) --emit=asm -C opt-level=0 -o $@ $<

$(RUST_ASM_OPT): $(RUST_DIR)/assembly_compare.rs
	$(RUSTC) --emit=asm -C opt-level=2 -o $@ $<

# ==============================================================================
# Exploration targets
# ==============================================================================

# --- assembly: Generate assembly for all C files (no optimization) ---
assembly: $(SIMPLE_MATH_S) $(CALLING_CONV_S) $(STACK_FRAMES_S) $(CONTROL_FLOW_S)
	@echo ""
	@echo "================================================================"
	@echo "  ASSEMBLY OUTPUT (-O0, no optimization)"
	@echo "================================================================"
	@echo ""
	@echo "Generated assembly files:"
	@echo "  $(SIMPLE_MATH_S)"
	@echo "  $(CALLING_CONV_S)"
	@echo "  $(STACK_FRAMES_S)"
	@echo "  $(CONTROL_FLOW_S)"
	@echo ""
	@echo "Open these .s files in your editor."
	@echo "Start with simple_math.s — find the 'add' function."
	@echo ""
	@echo "--- simple_math.s: The 'add' function ---"
	@echo ""
	@grep -A 15 '^add:' $(SIMPLE_MATH_S) 2>/dev/null || \
	 awk '/^[_]*add:/{found=1} found{print; if(/ret/) exit}' $(SIMPLE_MATH_S) || \
	 echo "(Could not extract 'add' function — open the .s file manually)"

# --- annotated: Verbose assembly with C variable names as comments ---
annotated: $(SIMPLE_MATH_VS)
	@echo ""
	@echo "================================================================"
	@echo "  VERBOSE ASSEMBLY (-O0 -fverbose-asm)"
	@echo "================================================================"
	@echo ""
	@echo "The -fverbose-asm flag adds comments showing which C variable"
	@echo "corresponds to which register/memory location."
	@echo ""
	@echo "Generated: $(SIMPLE_MATH_VS)"
	@echo ""
	@echo "--- First 80 lines of verbose assembly ---"
	@echo ""
	@head -80 $(SIMPLE_MATH_VS)
	@echo ""
	@echo "... (open $(SIMPLE_MATH_VS) to see the rest)"

# --- optimized: Compare -O0 vs -O2 assembly ---
optimized: $(SIMPLE_MATH_S) $(SIMPLE_MATH_OPT)
	@echo ""
	@echo "================================================================"
	@echo "  OPTIMIZATION COMPARISON: -O0 vs -O2"
	@echo "================================================================"
	@echo ""
	@echo "The compiler at -O2 is remarkably clever. Compare these files:"
	@echo "  $(SIMPLE_MATH_S)     (no optimization)"
	@echo "  $(SIMPLE_MATH_OPT)   (full optimization)"
	@echo ""
	@echo "--- Instruction count comparison ---"
	@echo ""
	@printf "  simple_math.s (-O0):     "
	@grep -c '^\s*[a-z]' $(SIMPLE_MATH_S) 2>/dev/null || echo "?"
	@printf "  simple_math_opt.s (-O2): "
	@grep -c '^\s*[a-z]' $(SIMPLE_MATH_OPT) 2>/dev/null || echo "?"
	@echo ""
	@echo "--- -O0 version of 'add' function ---"
	@echo ""
	@grep -A 15 '^add:' $(SIMPLE_MATH_S) 2>/dev/null || \
	 awk '/^[_]*add:/{found=1} found{print; if(/ret/) exit}' $(SIMPLE_MATH_S) || \
	 echo "(Open .s file manually)"
	@echo ""
	@echo "--- -O2 version of 'add' function ---"
	@echo ""
	@grep -A 8 '^add:' $(SIMPLE_MATH_OPT) 2>/dev/null || \
	 awk '/^[_]*add:/{found=1} found{print; if(/ret/) exit}' $(SIMPLE_MATH_OPT) || \
	 echo "(Open .s file manually)"
	@echo ""
	@echo "OBSERVE:"
	@echo "  - At -O0: args stored to stack, reloaded, computed, returned"
	@echo "  - At -O2: args stay in registers, one instruction does the work"
	@echo "  - At -O2: 'lea eax, [rdi+rsi]' replaces the entire add function"

# --- calling-conv: Examine calling convention register usage ---
calling-conv: $(CALLING_CONV_VS) $(CALLING_CONV)
	@echo ""
	@echo "================================================================"
	@echo "  CALLING CONVENTION: System V AMD64 ABI"
	@echo "================================================================"
	@echo ""
	@echo "--- Running the program ---"
	@echo ""
	@$(CALLING_CONV)
	@echo ""
	@echo "--- Verbose assembly output ---"
	@echo ""
	@echo "Generated: $(CALLING_CONV_VS)"
	@echo ""
	@echo "Key things to look for in the assembly:"
	@echo "  1. two_args: edi=arg1, esi=arg2"
	@echo "  2. six_args: all 6 register slots used"
	@echo "  3. eight_args: args 7-8 pushed onto stack!"
	@echo "  4. printf calls: eax=0 before call (0 float args)"
	@echo ""
	@echo "--- eight_args assembly (shows stack arguments) ---"
	@echo ""
	@awk '/^[_]*eight_args:/{found=1} found{print; if(/^[.]/ && found>1) exit; found++}' $(CALLING_CONV_VS) 2>/dev/null || \
	 echo "(Open $(CALLING_CONV_VS) and search for 'eight_args')"

# --- stack: Examine stack frame mechanics ---
stack: $(STACK_FRAMES_VS) $(STACK_FRAMES)
	@echo ""
	@echo "================================================================"
	@echo "  STACK FRAMES: Prologue, Epilogue, and Frame Chain"
	@echo "================================================================"
	@echo ""
	@echo "--- Running the program (shows stack address growth) ---"
	@echo ""
	@$(STACK_FRAMES)
	@echo ""
	@echo "--- Generated assembly with verbose annotations ---"
	@echo "  $(STACK_FRAMES_VS)"
	@echo ""
	@echo "Look for these patterns in every function:"
	@echo "  PROLOGUE: push rbp; mov rbp, rsp; sub rsp, N"
	@echo "  EPILOGUE: leave (= mov rsp, rbp; pop rbp) then ret"
	@echo "  LOCALS:   [rbp-4], [rbp-8], etc."

# --- control: Examine control flow assembly patterns ---
control: $(CONTROL_FLOW_VS) $(CONTROL_FLOW_OPT) $(CONTROL_FLOW)
	@echo ""
	@echo "================================================================"
	@echo "  CONTROL FLOW PATTERNS in Assembly"
	@echo "================================================================"
	@echo ""
	@echo "--- Running the program ---"
	@echo ""
	@$(CONTROL_FLOW)
	@echo ""
	@echo "--- Generated files ---"
	@echo "  $(CONTROL_FLOW_VS)   (-O0, verbose)"
	@echo "  $(CONTROL_FLOW_OPT)  (-O2, optimized)"
	@echo ""
	@echo "Find these patterns in the assembly:"
	@echo "  classify_number: cmp + jle/jge (if/else chain)"
	@echo "  sum_range:       cmp + jl + jmp (for loop)"
	@echo "  collatz_steps:   test + je + sar (while + if/else)"
	@echo "  day_name:        ja + jump table (dense switch)"
	@echo "  http_status_category: cmp + je chain (sparse switch)"

# --- disasm: Disassemble compiled binaries ---
disasm: $(SIMPLE_MATH) $(CALLING_CONV) $(STACK_FRAMES) $(CONTROL_FLOW)
	@echo ""
	@echo "================================================================"
	@echo "  DISASSEMBLY with objdump -d"
	@echo "================================================================"
	@echo ""
	@echo "objdump shows the binary's machine code as assembly."
	@echo "Unlike .s files (compiler output), this is from the LINKED binary."
	@echo ""
	@echo "--- simple_math: add function disassembly ---"
	@echo ""
	@objdump -d $(SIMPLE_MATH) | awk '/<add>:/{found=1} found{print; if(/ret/ && found>1) exit}'
	@echo ""
	@echo "--- simple_math: max function disassembly (with conditional) ---"
	@echo ""
	@objdump -d $(SIMPLE_MATH) | awk '/<max>:/{found=1} found{print; if(/ret/ && found>1) exit}'
	@echo ""
	@echo "--- calling_convention: eight_args disassembly ---"
	@echo ""
	@objdump -d $(CALLING_CONV) | awk '/<eight_args>:/{found=1} found{print; if(/ret/ && found>1) exit}'
	@echo ""
	@echo "For full disassembly of any binary:"
	@echo "  objdump -d $(SIMPLE_MATH)"
	@echo "  objdump -d $(CALLING_CONV)"
	@echo "  objdump -d $(STACK_FRAMES)"
	@echo "  objdump -d $(CONTROL_FLOW)"

# --- rust-asm: Compile Rust and compare assembly to C ---
rust-asm: $(RUST_ASM_S) $(RUST_ASM_OPT) $(SIMPLE_MATH_S) $(SIMPLE_MATH_OPT)
	@echo ""
	@echo "================================================================"
	@echo "  RUST ASSEMBLY vs C ASSEMBLY"
	@echo "================================================================"
	@echo ""
	@echo "Generated Rust assembly files:"
	@echo "  $(RUST_ASM_S)      (-O0, no optimization)"
	@echo "  $(RUST_ASM_OPT)    (-O2, optimized)"
	@echo ""
	@echo "Generated C assembly files for comparison:"
	@echo "  $(SIMPLE_MATH_S)   (-O0)"
	@echo "  $(SIMPLE_MATH_OPT) (-O2)"
	@echo ""
	@echo "--- Line count comparison ---"
	@echo ""
	@printf "  Rust -O0:  "
	@wc -l < $(RUST_ASM_S)
	@printf "  Rust -O2:  "
	@wc -l < $(RUST_ASM_OPT)
	@printf "  C -O0:     "
	@wc -l < $(SIMPLE_MATH_S)
	@printf "  C -O2:     "
	@wc -l < $(SIMPLE_MATH_OPT)
	@echo ""
	@echo "Rust assembly is larger due to libstd and panic infrastructure."
	@echo "But the INDIVIDUAL FUNCTIONS compile to the same instructions."
	@echo ""
	@echo "--- Rust -O2 'add_values' function ---"
	@echo ""
	@grep -A 10 'add_values' $(RUST_ASM_OPT) | head -15 || echo "(search manually in the .s file)"
	@echo ""
	@echo "--- C -O2 'add' function ---"
	@echo ""
	@grep -A 8 '^add:' $(SIMPLE_MATH_OPT) 2>/dev/null || \
	 awk '/^[_]*add:/{found=1} found{print; if(/ret/) exit}' $(SIMPLE_MATH_OPT) || \
	 echo "(search manually in the .s file)"
	@echo ""
	@echo "At -O2, both should be: lea eax, [rdi+rsi]; ret"
	@echo "The source language doesn't matter — LLVM and GCC optimize to the same code."

# --- java-bytecode: Show JVM bytecode (javap -c) ---
java-bytecode: $(JAVA_CLASS)
	@echo ""
	@echo "================================================================"
	@echo "  JAVA BYTECODE (stack-based) vs x86 Assembly (register-based)"
	@echo "================================================================"
	@echo ""
	@echo "--- JVM Bytecode Disassembly (javap -c) ---"
	@echo ""
	@cd $(JAVA_DIR) && javap -c StackVsRegister
	@echo ""
	@echo "--- Running the Java program ---"
	@echo ""
	@cd $(JAVA_DIR) && java StackVsRegister

# --- compare: Side-by-side comparison of all three ---
compare: $(SIMPLE_MATH_OPT) $(RUST_ASM_OPT) $(JAVA_CLASS)
	@echo ""
	@echo "================================================================"
	@echo "  THREE-WAY COMPARISON: C Assembly vs Rust Assembly vs Java Bytecode"
	@echo "================================================================"
	@echo ""
	@echo "All three implement add(a, b) = a + b. Compare the instruction sequences:"
	@echo ""
	@echo "╔══════════════════════════════════════════════════════════════════════╗"
	@echo "║  C (x86-64 assembly, -O2)                                          ║"
	@echo "╠══════════════════════════════════════════════════════════════════════╣"
	@echo "║  lea    eax, [rdi+rsi]    ; eax = arg1 + arg2 (one instruction!)   ║"
	@echo "║  ret                       ; return value in eax                    ║"
	@echo "╠══════════════════════════════════════════════════════════════════════╣"
	@echo "║  Rust (x86-64 assembly, -O2) — virtually identical to C            ║"
	@echo "╠══════════════════════════════════════════════════════════════════════╣"
	@echo "║  lea    eax, [rdi+rsi]    ; Same instruction! Same ABI!            ║"
	@echo "║  ret                       ; Same return convention                 ║"
	@echo "╠══════════════════════════════════════════════════════════════════════╣"
	@echo "║  Java (JVM bytecode) — stack-based, not register-based             ║"
	@echo "╠══════════════════════════════════════════════════════════════════════╣"
	@echo "║  iload_0                  ; Push arg 'a' onto operand stack        ║"
	@echo "║  iload_1                  ; Push arg 'b' onto operand stack        ║"
	@echo "║  iadd                     ; Pop both, add, push result             ║"
	@echo "║  ireturn                  ; Pop result and return it               ║"
	@echo "╚══════════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "KEY TAKEAWAYS:"
	@echo "  1. C and Rust produce IDENTICAL x86-64 assembly at -O2"
	@echo "  2. Java bytecode is a completely different paradigm (stack-based)"
	@echo "  3. The JVM's JIT compiler converts bytecode → x86 at runtime"
	@echo "  4. After JIT, Java's native code looks similar to C/Rust assembly"
	@echo ""
	@echo "--- Actual files for deeper comparison ---"
	@echo "  C:    $(SIMPLE_MATH_OPT)"
	@echo "  Rust: $(RUST_ASM_OPT)"
	@echo "  Java: cd $(JAVA_DIR) && javap -c StackVsRegister"
	@echo ""
	@echo "--- Actual C -O2 'add' function from compiler output ---"
	@echo ""
	@grep -A 8 '^add:' $(SIMPLE_MATH_OPT) 2>/dev/null || \
	 awk '/^[_]*add:/{found=1} found{print; if(/ret/) exit}' $(SIMPLE_MATH_OPT) || \
	 echo "(Open the .s files manually)"
	@echo ""
	@echo "--- Actual Java bytecode for 'add' method ---"
	@echo ""
	@cd $(JAVA_DIR) && javap -c StackVsRegister 2>/dev/null | awk '/static int add/{found=1} found{print; if(/^\s*$$/ || /^[^ ]/) {if(found>1) exit} found++}' || echo "(Run javap -c manually)"

# ==============================================================================
# Clean
# ==============================================================================

clean:
	@echo "Cleaning Module 4 build artifacts..."
	rm -f $(C_DIR)/simple_math $(C_DIR)/calling_convention
	rm -f $(C_DIR)/stack_frames $(C_DIR)/control_flow
	rm -f $(C_DIR)/*.s
	rm -f $(RUST_DIR)/assembly_compare
	rm -f $(RUST_DIR)/*.s
	rm -f $(JAVA_DIR)/*.class
	@echo "Clean complete."
