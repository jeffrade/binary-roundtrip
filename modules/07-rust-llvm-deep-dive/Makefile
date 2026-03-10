# ==============================================================================
# Module 7: Rust / LLVM Deep Dive — Makefile
# ==============================================================================
#
# Traces the Rust compilation pipeline: .rs -> MIR -> LLVM IR -> Assembly -> ELF
# Demonstrates monomorphization, zero-cost abstractions, closures, ownership,
# and bare-metal no_std programming.
#
# Use `make help` to see all available targets.
#
# Targets:
#   make all           — Build all examples
#   make generics      — Build generics_mono, show monomorphized LLVM IR
#   make zero-cost     — Build zero_cost at -O0 and -O2, compare assembly
#   make closures      — Build closures, show assembly
#   make ownership     — Build ownership_demo, show MIR
#   make llvm-ir       — Build llvm_ir_demo, show and annotate LLVM IR
#   make no-std        — Build the no_std bare-metal example
#   make compare       — Build C and Rust equivalents at -O2, diff assembly
#   make pipeline      — Show full pipeline: .rs -> MIR -> LLVM IR -> ASM -> ELF
#   make clean         — Remove all build artifacts
#   make help          — Show this help message
# ==============================================================================

# Compilers and flags
RUSTC   := rustc
CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic

# Directories
RUST_DIR := rust
C_DIR    := c
BUILD    := build

# Source files
GENERICS_SRC  := $(RUST_DIR)/generics_mono.rs
ZEROCOST_SRC  := $(RUST_DIR)/zero_cost.rs
CLOSURES_SRC  := $(RUST_DIR)/closures.rs
OWNERSHIP_SRC := $(RUST_DIR)/ownership_demo.rs
LLVMIR_SRC    := $(RUST_DIR)/llvm_ir_demo.rs
NOSTD_SRC     := $(RUST_DIR)/no_std_demo.rs
C_EQUIV_SRC   := $(C_DIR)/equivalent.c

# Build artifacts
GENERICS_BIN  := $(BUILD)/generics_mono
ZEROCOST_BIN  := $(BUILD)/zero_cost
CLOSURES_BIN  := $(BUILD)/closures
OWNERSHIP_BIN := $(BUILD)/ownership_demo
LLVMIR_BIN    := $(BUILD)/llvm_ir_demo
NOSTD_BIN     := $(BUILD)/no_std_demo
C_EQUIV_BIN   := $(BUILD)/equivalent

# ==============================================================================
# Phony targets
# ==============================================================================
.PHONY: all generics zero-cost closures ownership llvm-ir no-std compare \
        pipeline clean help

# ==============================================================================
# Build targets
# ==============================================================================

all: $(BUILD) $(GENERICS_BIN) $(ZEROCOST_BIN) $(CLOSURES_BIN) $(OWNERSHIP_BIN) \
     $(LLVMIR_BIN) $(C_EQUIV_BIN)
	@echo ""
	@echo "All standard programs built successfully in $(BUILD)/."
	@echo "(Run 'make no-std' separately for the bare-metal example.)"
	@echo "Run 'make help' for all available targets."

$(BUILD):
	mkdir -p $(BUILD)

$(GENERICS_BIN): $(GENERICS_SRC) | $(BUILD)
	$(RUSTC) --edition 2021 $< -o $@

$(ZEROCOST_BIN): $(ZEROCOST_SRC) | $(BUILD)
	$(RUSTC) --edition 2021 $< -o $@

$(CLOSURES_BIN): $(CLOSURES_SRC) | $(BUILD)
	$(RUSTC) --edition 2021 $< -o $@

$(OWNERSHIP_BIN): $(OWNERSHIP_SRC) | $(BUILD)
	$(RUSTC) --edition 2021 $< -o $@

$(LLVMIR_BIN): $(LLVMIR_SRC) | $(BUILD)
	$(RUSTC) --edition 2021 $< -o $@

$(C_EQUIV_BIN): $(C_EQUIV_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -O2 -o $@ $<

# ==============================================================================
# Demonstration targets
# ==============================================================================

# --------------------------------------------------------------------------
# Generics & Monomorphization
# --------------------------------------------------------------------------
generics: $(BUILD)
	@echo "======================================================================"
	@echo " GENERICS & MONOMORPHIZATION"
	@echo "======================================================================"
	@echo ""
	@echo "Building generics_mono.rs..."
	$(RUSTC) --edition 2021 $(GENERICS_SRC) -o $(GENERICS_BIN)
	@echo ""
	@echo "--- Running the program ---"
	@./$(GENERICS_BIN)
	@echo ""
	@echo "--- Generating LLVM IR to see monomorphized functions ---"
	$(RUSTC) --edition 2021 --emit=llvm-ir $(GENERICS_SRC) -o $(BUILD)/generics_mono.ll
	@echo ""
	@echo "--- Monomorphized function definitions in LLVM IR ---"
	@echo "(Each generic function generates one copy per concrete type)"
	@echo ""
	@grep 'define.*double\|define.*add_and_scale\|define.*Wrapper' $(BUILD)/generics_mono.ll 2>/dev/null | head -20 || \
		echo "  (Functions may be mangled or inlined. Try: grep 'define' $(BUILD)/generics_mono.ll)"
	@echo ""
	@echo "LLVM IR types used:"
	@grep -o 'i32\|i64\|double\|float' $(BUILD)/generics_mono.ll | sort | uniq -c | sort -rn | head -10
	@echo ""
	@echo "Full LLVM IR saved to: $(BUILD)/generics_mono.ll"
	@echo "Explore: less $(BUILD)/generics_mono.ll"

# --------------------------------------------------------------------------
# Zero-Cost Abstractions
# --------------------------------------------------------------------------
zero-cost: $(BUILD)
	@echo "======================================================================"
	@echo " ZERO-COST ABSTRACTIONS"
	@echo "======================================================================"
	@echo ""
	@echo "Building zero_cost.rs at -O2 (optimized)..."
	$(RUSTC) --edition 2021 -C opt-level=2 $(ZEROCOST_SRC) -o $(ZEROCOST_BIN)
	@echo ""
	@echo "--- Running the program (optimized) ---"
	@./$(ZEROCOST_BIN)
	@echo ""
	@echo "--- Generating assembly at -O0 (unoptimized) ---"
	$(RUSTC) --edition 2021 --emit=asm -C opt-level=0 $(ZEROCOST_SRC) -o $(BUILD)/zero_cost_O0.s
	@echo "  Unoptimized assembly: $$(wc -l < $(BUILD)/zero_cost_O0.s) lines"
	@echo ""
	@echo "--- Generating assembly at -O2 (optimized) ---"
	$(RUSTC) --edition 2021 --emit=asm -C opt-level=2 $(ZEROCOST_SRC) -o $(BUILD)/zero_cost_O2.s
	@echo "  Optimized assembly: $$(wc -l < $(BUILD)/zero_cost_O2.s) lines"
	@echo ""
	@echo "--- Comparing imperative vs iterator (optimized) ---"
	@echo ""
	@echo "sum_of_squares_imperative assembly (-O2):"
	@grep -A 25 '^sum_of_squares_imperative:' $(BUILD)/zero_cost_O2.s 2>/dev/null | head -25 || \
		echo "  (Function may be present with different label format)"
	@echo ""
	@echo "sum_of_squares_iterator assembly (-O2):"
	@grep -A 25 '^sum_of_squares_iterator:' $(BUILD)/zero_cost_O2.s 2>/dev/null | head -25 || \
		echo "  (Function may be present with different label format)"
	@echo ""
	@echo "If both produce the same assembly, that's ZERO-COST ABSTRACTION."
	@echo ""
	@echo "Files saved:"
	@echo "  $(BUILD)/zero_cost_O0.s  — Unoptimized (iterator chain visible)"
	@echo "  $(BUILD)/zero_cost_O2.s  — Optimized (iterator chain collapsed)"

# --------------------------------------------------------------------------
# Closures
# --------------------------------------------------------------------------
closures: $(BUILD)
	@echo "======================================================================"
	@echo " HOW CLOSURES COMPILE"
	@echo "======================================================================"
	@echo ""
	@echo "Building closures.rs..."
	$(RUSTC) --edition 2021 $(CLOSURES_SRC) -o $(CLOSURES_BIN)
	@echo ""
	@echo "--- Running the program ---"
	@./$(CLOSURES_BIN)
	@echo ""
	@echo "--- Generating assembly at -O2 ---"
	$(RUSTC) --edition 2021 --emit=asm -C opt-level=2 $(CLOSURES_SRC) -o $(BUILD)/closures.s
	@echo ""
	@echo "Assembly size: $$(wc -l < $(BUILD)/closures.s) lines"
	@echo ""
	@echo "--- Looking for closure-related symbols ---"
	@grep 'closure\|apply_static\|apply_dynamic' $(BUILD)/closures.s 2>/dev/null | head -15 || \
		echo "  (Closures are anonymous — look for {{closure}} in LLVM IR instead)"
	@echo ""
	@echo "--- Generating LLVM IR to see closure structs ---"
	$(RUSTC) --edition 2021 --emit=llvm-ir $(CLOSURES_SRC) -o $(BUILD)/closures.ll
	@echo ""
	@echo "Closure types in LLVM IR:"
	@grep -i 'closure' $(BUILD)/closures.ll 2>/dev/null | head -15 || \
		echo "  (Try: grep -i 'closure' $(BUILD)/closures.ll)"
	@echo ""
	@echo "Files saved:"
	@echo "  $(BUILD)/closures.s   — Assembly (closures inlined at -O2)"
	@echo "  $(BUILD)/closures.ll  — LLVM IR (closure structs visible)"

# --------------------------------------------------------------------------
# Ownership
# --------------------------------------------------------------------------
ownership: $(BUILD)
	@echo "======================================================================"
	@echo " OWNERSHIP AT THE COMPILER LEVEL"
	@echo "======================================================================"
	@echo ""
	@echo "Building ownership_demo.rs..."
	$(RUSTC) --edition 2021 $(OWNERSHIP_SRC) -o $(OWNERSHIP_BIN)
	@echo ""
	@echo "--- Running the program ---"
	@./$(OWNERSHIP_BIN)
	@echo ""
	@echo "--- Generating MIR (where borrow checking happens) ---"
	$(RUSTC) --edition 2021 --emit=mir $(OWNERSHIP_SRC) -o $(BUILD)/ownership_demo.mir
	@echo "  MIR output: $$(wc -l < $(BUILD)/ownership_demo.mir) lines"
	@echo ""
	@echo "--- Key MIR constructs (ownership in action) ---"
	@echo ""
	@echo "Move operations:"
	@grep -c 'move ' $(BUILD)/ownership_demo.mir 2>/dev/null || echo "  0"
	@echo ""
	@echo "Borrow operations:"
	@grep -c '&.*_[0-9]' $(BUILD)/ownership_demo.mir 2>/dev/null || echo "  0"
	@echo ""
	@echo "Drop operations (destructors):"
	@grep -c 'drop(' $(BUILD)/ownership_demo.mir 2>/dev/null || echo "  0"
	@echo ""
	@echo "StorageLive/Dead (scope tracking):"
	@grep -c 'StorageLive\|StorageDead' $(BUILD)/ownership_demo.mir 2>/dev/null || echo "  0"
	@echo ""
	@echo "--- Generating LLVM IR (ownership ERASED) ---"
	$(RUSTC) --edition 2021 --emit=llvm-ir $(OWNERSHIP_SRC) -o $(BUILD)/ownership_demo.ll
	@echo "  LLVM IR: $$(wc -l < $(BUILD)/ownership_demo.ll) lines"
	@echo ""
	@echo "  In the LLVM IR, there are NO move/borrow annotations."
	@echo "  The ownership system has been completely erased."
	@echo ""
	@echo "Files saved:"
	@echo "  $(BUILD)/ownership_demo.mir — MIR (ownership visible)"
	@echo "  $(BUILD)/ownership_demo.ll  — LLVM IR (ownership erased)"

# --------------------------------------------------------------------------
# LLVM IR
# --------------------------------------------------------------------------
llvm-ir: $(BUILD)
	@echo "======================================================================"
	@echo " UNDERSTANDING LLVM IR"
	@echo "======================================================================"
	@echo ""
	@echo "Building llvm_ir_demo.rs..."
	$(RUSTC) --edition 2021 $(LLVMIR_SRC) -o $(LLVMIR_BIN)
	@echo ""
	@echo "--- Running the program ---"
	@./$(LLVMIR_BIN)
	@echo ""
	@echo "--- Generating unoptimized LLVM IR ---"
	$(RUSTC) --edition 2021 --emit=llvm-ir $(LLVMIR_SRC) -o $(BUILD)/llvm_ir_demo.ll
	@echo "  Unoptimized: $$(wc -l < $(BUILD)/llvm_ir_demo.ll) lines"
	@echo ""
	@echo "--- Generating optimized LLVM IR (-O2) ---"
	$(RUSTC) --edition 2021 --emit=llvm-ir -C opt-level=2 $(LLVMIR_SRC) -o $(BUILD)/llvm_ir_demo_opt.ll
	@echo "  Optimized:   $$(wc -l < $(BUILD)/llvm_ir_demo_opt.ll) lines"
	@echo ""
	@echo "--- Key functions in unoptimized LLVM IR ---"
	@echo ""
	@echo "Functions defined:"
	@grep 'define.*@arithmetic_ssa\|define.*@branching\|define.*@loop_sum\|define.*@stack_allocation\|define.*@callee\|define.*@caller_demo\|define.*@struct_demo\|define.*@match_demo' \
		$(BUILD)/llvm_ir_demo.ll 2>/dev/null | head -10 || echo "  (Check the .ll file directly)"
	@echo ""
	@echo "--- LLVM IR instruction counts ---"
	@echo ""
	@echo "  alloca (stack allocations):   $$(grep -c 'alloca' $(BUILD)/llvm_ir_demo.ll 2>/dev/null || echo 0)"
	@echo "  load (memory reads):          $$(grep -c '= load' $(BUILD)/llvm_ir_demo.ll 2>/dev/null || echo 0)"
	@echo "  store (memory writes):        $$(grep -c 'store' $(BUILD)/llvm_ir_demo.ll 2>/dev/null || echo 0)"
	@echo "  add/sub/mul (arithmetic):     $$(grep -c ' add \| sub \| mul ' $(BUILD)/llvm_ir_demo.ll 2>/dev/null || echo 0)"
	@echo "  icmp (integer comparisons):   $$(grep -c 'icmp' $(BUILD)/llvm_ir_demo.ll 2>/dev/null || echo 0)"
	@echo "  br (branches):                $$(grep -c '  br ' $(BUILD)/llvm_ir_demo.ll 2>/dev/null || echo 0)"
	@echo "  call (function calls):        $$(grep -c '  call' $(BUILD)/llvm_ir_demo.ll 2>/dev/null || echo 0)"
	@echo "  phi (SSA merge nodes):        $$(grep -c 'phi' $(BUILD)/llvm_ir_demo.ll 2>/dev/null || echo 0)"
	@echo "  getelementptr (field access): $$(grep -c 'getelementptr' $(BUILD)/llvm_ir_demo.ll 2>/dev/null || echo 0)"
	@echo "  switch (match statements):    $$(grep -c 'switch' $(BUILD)/llvm_ir_demo.ll 2>/dev/null || echo 0)"
	@echo ""
	@echo "--- Same counts for optimized version ---"
	@echo ""
	@echo "  alloca:          $$(grep -c 'alloca' $(BUILD)/llvm_ir_demo_opt.ll 2>/dev/null || echo 0)"
	@echo "  load:            $$(grep -c '= load' $(BUILD)/llvm_ir_demo_opt.ll 2>/dev/null || echo 0)"
	@echo "  store:           $$(grep -c 'store' $(BUILD)/llvm_ir_demo_opt.ll 2>/dev/null || echo 0)"
	@echo "  (Notice how the optimizer eliminates most alloca/load/store!)"
	@echo ""
	@echo "Files saved:"
	@echo "  $(BUILD)/llvm_ir_demo.ll     — Unoptimized (educational: see all constructs)"
	@echo "  $(BUILD)/llvm_ir_demo_opt.ll — Optimized (real-world: what LLVM produces)"

# --------------------------------------------------------------------------
# No-std bare-metal
# --------------------------------------------------------------------------
no-std: $(BUILD)
	@echo "======================================================================"
	@echo " NO_STD BARE-METAL EXAMPLE"
	@echo "======================================================================"
	@echo ""
	@echo "Building no_std_demo.rs (bare metal, no libc)..."
	@echo "  Flags: -C link-arg=-nostartfiles -C panic=abort"
	$(RUSTC) --edition 2021 -C link-arg=-nostartfiles -C panic=abort \
		$(NOSTD_SRC) -o $(NOSTD_BIN)
	@echo ""
	@echo "--- Binary comparison ---"
	@echo "  no_std binary:  $$(stat -c%s $(NOSTD_BIN) 2>/dev/null || stat -f%z $(NOSTD_BIN) 2>/dev/null) bytes"
	@echo "  Normal binary:  $$(stat -c%s $(ZEROCOST_BIN) 2>/dev/null || echo '(build zero_cost first)')"
	@echo ""
	@echo "--- File type ---"
	@file $(NOSTD_BIN)
	@echo ""
	@echo "--- Shared library dependencies ---"
	@ldd $(NOSTD_BIN) 2>&1 || true
	@echo "  (A truly bare-metal binary has NO library dependencies.)"
	@echo ""
	@echo "--- Running the bare-metal program ---"
	@echo ""
	@./$(NOSTD_BIN)

# --------------------------------------------------------------------------
# Compare C and Rust assembly
# --------------------------------------------------------------------------
compare: $(BUILD)
	@echo "======================================================================"
	@echo " C vs RUST ASSEMBLY COMPARISON"
	@echo "======================================================================"
	@echo ""
	@echo "--- Compiling C equivalent at -O2 ---"
	$(CC) $(CFLAGS) -O2 -S $(C_EQUIV_SRC) -o $(BUILD)/equivalent_c.s
	@echo "  C assembly: $$(wc -l < $(BUILD)/equivalent_c.s) lines"
	@echo ""
	@echo "--- Compiling Rust (imperative) at -O2 ---"
	$(RUSTC) --edition 2021 --emit=asm -C opt-level=2 $(ZEROCOST_SRC) -o $(BUILD)/zero_cost_O2.s
	@echo "  Rust assembly: $$(wc -l < $(BUILD)/zero_cost_O2.s) lines"
	@echo ""
	@echo "--- C sum_of_squares_imperative ---"
	@grep -A 30 '^sum_of_squares_imperative:' $(BUILD)/equivalent_c.s 2>/dev/null | head -30 || \
		echo "  (Function label format may differ. Check: grep 'sum_of_squares' $(BUILD)/equivalent_c.s)"
	@echo ""
	@echo "--- Rust sum_of_squares_imperative ---"
	@grep -A 30 '^sum_of_squares_imperative:' $(BUILD)/zero_cost_O2.s 2>/dev/null | head -30 || \
		echo "  (Function label format may differ. Check the .s file directly.)"
	@echo ""
	@echo "--- Rust sum_of_squares_iterator ---"
	@grep -A 30 '^sum_of_squares_iterator:' $(BUILD)/zero_cost_O2.s 2>/dev/null | head -30 || \
		echo "  (Function label format may differ. Check the .s file directly.)"
	@echo ""
	@echo "  Compare the three functions above."
	@echo "  All three should have the same core loop structure:"
	@echo "    test/and for evenness check"
	@echo "    imul for squaring"
	@echo "    add for accumulation"
	@echo ""
	@echo "  This proves: Rust iterators = C loops = same machine code."
	@echo ""
	@echo "--- Running both programs to verify correctness ---"
	@echo ""
	@echo "C version:"
	$(CC) $(CFLAGS) -O2 -o $(C_EQUIV_BIN) $(C_EQUIV_SRC)
	@./$(C_EQUIV_BIN) 2>/dev/null | head -6
	@echo ""
	@echo "Rust version:"
	$(RUSTC) --edition 2021 -C opt-level=2 $(ZEROCOST_SRC) -o $(ZEROCOST_BIN)
	@./$(ZEROCOST_BIN) 2>/dev/null | head -6
	@echo ""
	@echo "Files saved:"
	@echo "  $(BUILD)/equivalent_c.s   — C assembly"
	@echo "  $(BUILD)/zero_cost_O2.s   — Rust assembly"

# --------------------------------------------------------------------------
# Full pipeline demonstration
# --------------------------------------------------------------------------
pipeline: $(BUILD)
	@echo "######################################################################"
	@echo "#                                                                    #"
	@echo "#  MODULE 7: THE FULL RUST COMPILATION PIPELINE                      #"
	@echo "#                                                                    #"
	@echo "#  .rs -> HIR -> MIR -> LLVM IR -> Assembly -> Object -> ELF         #"
	@echo "#                                                                    #"
	@echo "######################################################################"
	@echo ""
	@echo "Using llvm_ir_demo.rs as the subject."
	@echo ""
	@echo "======================================================================"
	@echo " STAGE 1: SOURCE CODE (.rs)"
	@echo "======================================================================"
	@echo "  File: $(LLVMIR_SRC)"
	@echo "  Lines: $$(wc -l < $(LLVMIR_SRC))"
	@echo "  This is what the programmer writes."
	@echo ""
	@echo "======================================================================"
	@echo " STAGE 2: MIR (Mid-level Intermediate Representation)"
	@echo "======================================================================"
	@echo "  This is where BORROW CHECKING happens."
	@echo "  MIR tracks ownership, moves, borrows, and drop points."
	$(RUSTC) --edition 2021 --emit=mir $(LLVMIR_SRC) -o $(BUILD)/pipeline_mir.mir
	@echo "  Output: $(BUILD)/pipeline_mir.mir ($$(wc -l < $(BUILD)/pipeline_mir.mir) lines)"
	@echo ""
	@echo "  Sample MIR (first function):"
	@head -30 $(BUILD)/pipeline_mir.mir
	@echo "  ..."
	@echo ""
	@echo "======================================================================"
	@echo " STAGE 3: LLVM IR (Low-level Virtual Machine IR)"
	@echo "======================================================================"
	@echo "  Shared with C/C++ (Clang also emits LLVM IR)."
	@echo "  Ownership annotations are ERASED at this point."
	@echo "  SSA form, typed, platform-independent."
	$(RUSTC) --edition 2021 --emit=llvm-ir $(LLVMIR_SRC) -o $(BUILD)/pipeline_llvm.ll
	@echo "  Output: $(BUILD)/pipeline_llvm.ll ($$(wc -l < $(BUILD)/pipeline_llvm.ll) lines)"
	@echo ""
	@echo "  Functions defined:"
	@grep 'define.*@' $(BUILD)/pipeline_llvm.ll 2>/dev/null | head -10
	@echo "  ..."
	@echo ""
	@echo "======================================================================"
	@echo " STAGE 4: ASSEMBLY (.s)"
	@echo "======================================================================"
	@echo "  Platform-specific (x86-64). Human-readable mnemonics."
	@echo "  This is what the CPU will execute (after one more translation)."
	$(RUSTC) --edition 2021 --emit=asm -C opt-level=2 $(LLVMIR_SRC) -o $(BUILD)/pipeline_asm.s
	@echo "  Output: $(BUILD)/pipeline_asm.s ($$(wc -l < $(BUILD)/pipeline_asm.s) lines)"
	@echo ""
	@echo "  Function labels:"
	@grep '^[a-zA-Z_][a-zA-Z_0-9]*:' $(BUILD)/pipeline_asm.s 2>/dev/null | head -10
	@echo "  ..."
	@echo ""
	@echo "======================================================================"
	@echo " STAGE 5: OBJECT FILE (.o)"
	@echo "======================================================================"
	@echo "  Binary machine code, but not yet linked."
	$(RUSTC) --edition 2021 --emit=obj $(LLVMIR_SRC) -o $(BUILD)/pipeline_obj.o
	@file $(BUILD)/pipeline_obj.o
	@echo "  Size: $$(stat -c%s $(BUILD)/pipeline_obj.o 2>/dev/null || stat -f%z $(BUILD)/pipeline_obj.o 2>/dev/null) bytes"
	@echo ""
	@echo "  Symbols:"
	@nm $(BUILD)/pipeline_obj.o 2>/dev/null | grep -E 'T |U ' | head -10
	@echo "  ..."
	@echo ""
	@echo "======================================================================"
	@echo " STAGE 6: ELF EXECUTABLE"
	@echo "======================================================================"
	@echo "  Fully linked, ready to run."
	$(RUSTC) --edition 2021 -C opt-level=2 $(LLVMIR_SRC) -o $(LLVMIR_BIN)
	@file $(LLVMIR_BIN)
	@echo "  Size: $$(stat -c%s $(LLVMIR_BIN) 2>/dev/null || stat -f%z $(LLVMIR_BIN) 2>/dev/null) bytes"
	@echo ""
	@echo "  Dependencies:"
	@ldd $(LLVMIR_BIN) 2>/dev/null || echo "  (statically linked)"
	@echo ""
	@echo "======================================================================"
	@echo " PIPELINE COMPLETE — Artifact Summary"
	@echo "======================================================================"
	@echo ""
	@echo "  Source:    $(LLVMIR_SRC)             ($$(wc -l < $(LLVMIR_SRC)) lines)"
	@echo "  MIR:       $(BUILD)/pipeline_mir.mir  ($$(wc -l < $(BUILD)/pipeline_mir.mir) lines)"
	@echo "  LLVM IR:   $(BUILD)/pipeline_llvm.ll  ($$(wc -l < $(BUILD)/pipeline_llvm.ll) lines)"
	@echo "  Assembly:  $(BUILD)/pipeline_asm.s    ($$(wc -l < $(BUILD)/pipeline_asm.s) lines)"
	@echo "  Object:    $(BUILD)/pipeline_obj.o    ($$(stat -c%s $(BUILD)/pipeline_obj.o 2>/dev/null || stat -f%z $(BUILD)/pipeline_obj.o 2>/dev/null) bytes)"
	@echo "  ELF:       $(LLVMIR_BIN)        ($$(stat -c%s $(LLVMIR_BIN) 2>/dev/null || stat -f%z $(LLVMIR_BIN) 2>/dev/null) bytes)"
	@echo ""
	@echo "  Each stage transforms the code closer to what the CPU executes."
	@echo "  Rust adds MIR (borrow checking) before reaching LLVM IR."
	@echo "  After MIR, the pipeline is identical to C/C++ compiled with Clang."

# ==============================================================================
# Cleanup
# ==============================================================================

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD)
	@echo "Clean complete."

# ==============================================================================
# Help
# ==============================================================================

help:
	@echo "Module 7: Rust / LLVM Deep Dive — Available Targets"
	@echo ""
	@echo "  Individual demos:"
	@echo "    make generics    Monomorphization: generic fn -> one copy per type"
	@echo "    make zero-cost   Zero-cost abstractions: iterator == hand-written loop"
	@echo "    make closures    How closures compile: anonymous struct + trait impl"
	@echo "    make ownership   Ownership in MIR: moves, borrows, drops"
	@echo "    make llvm-ir     LLVM IR explained: SSA, basic blocks, phi nodes"
	@echo "    make no-std      Bare-metal Rust: no libc, raw syscalls"
	@echo ""
	@echo "  Comparisons:"
	@echo "    make compare     C vs Rust assembly at -O2"
	@echo ""
	@echo "  Pipeline:"
	@echo "    make pipeline    Full pipeline: .rs -> MIR -> LLVM IR -> ASM -> ELF"
	@echo ""
	@echo "  Build/Clean:"
	@echo "    make all         Build all standard examples"
	@echo "    make clean       Remove all build artifacts"
	@echo "    make help        Show this help message"
