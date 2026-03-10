# ==============================================================================
# Module 2: C Compilation Pipeline — Makefile
# ==============================================================================
#
# This Makefile demonstrates and exercises every stage of the C compilation
# pipeline: Preprocessing → Compilation → Assembly → Linking.
#
# Use `make help` to see all available targets.
#
# Targets:
#   make all           — Build all executables
#   make preprocess    — Run gcc -E on stages_demo.c (show preprocessed output)
#   make compile       — Run gcc -S on stages_demo.c (show assembly)
#   make assemble      — Run gcc -c on stages_demo.c (show object file info)
#   make link          — Full compilation of stages_demo.c
#   make pipeline      — Run ALL stages in sequence with explanatory banners
#   make multi         — Build multi-file project, show separate compilation
#   make macros        — Preprocess macro_demo.c with and without -DDEBUG
#   make rust-pipeline — Show Rust compilation stages
#   make clean         — Remove all build artifacts
#   make help          — Show this help message
# ==============================================================================

# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic
RUSTC   := rustc

# Directories
C_DIR    := c
RUST_DIR := rust
BUILD    := build

# Source files
STAGES_SRC   := $(C_DIR)/stages_demo.c
MACRO_SRC    := $(C_DIR)/macro_demo.c
MULTI_MAIN   := $(C_DIR)/multi_file_main.c
MULTI_UTILS  := $(C_DIR)/math_utils.c
HELLO_SRC    := $(C_DIR)/hello.c
RUST_SRC     := $(RUST_DIR)/pipeline_compare.rs

# Build artifacts
STAGES_PP    := $(BUILD)/stages_demo.i
STAGES_ASM   := $(BUILD)/stages_demo.s
STAGES_OBJ   := $(BUILD)/stages_demo.o
STAGES_BIN   := $(BUILD)/stages_demo
HELLO_BIN    := $(BUILD)/hello
MACRO_BIN    := $(BUILD)/macro_demo
MACRO_DBG    := $(BUILD)/macro_demo_debug
MULTI_BIN    := $(BUILD)/multi_file
MULTI_MAIN_O := $(BUILD)/multi_file_main.o
MULTI_UTILS_O:= $(BUILD)/math_utils.o
RUST_BIN     := $(BUILD)/pipeline_compare

# ==============================================================================
# Phony targets
# ==============================================================================
.PHONY: all preprocess compile assemble link pipeline multi macros \
        rust-pipeline clean help

# ==============================================================================
# Build targets
# ==============================================================================

# Build everything
all: $(BUILD) $(STAGES_BIN) $(HELLO_BIN) $(MACRO_BIN) $(MULTI_BIN) $(RUST_BIN)
	@echo ""
	@echo "All programs built successfully in $(BUILD)/."
	@echo "Run 'make pipeline' to see the full compilation pipeline demo."
	@echo "Run 'make help' for all available targets."

# Create build directory
$(BUILD):
	mkdir -p $(BUILD)

# Build stages_demo
$(STAGES_BIN): $(STAGES_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<

# Build hello
$(HELLO_BIN): $(HELLO_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<

# Build macro_demo (without DEBUG)
$(MACRO_BIN): $(MACRO_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<

# Build multi-file project
$(MULTI_MAIN_O): $(MULTI_MAIN) $(C_DIR)/math_utils.h | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(MULTI_UTILS_O): $(MULTI_UTILS) $(C_DIR)/math_utils.h | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(MULTI_BIN): $(MULTI_MAIN_O) $(MULTI_UTILS_O)
	$(CC) $(CFLAGS) -o $@ $^

# Build Rust pipeline_compare
$(RUST_BIN): $(RUST_SRC) | $(BUILD)
	$(RUSTC) $< -o $@

# ==============================================================================
# Pipeline demonstration targets
# ==============================================================================

# Stage 1: PREPROCESSING — expand includes, replace macros
preprocess: $(BUILD)
	@echo "======================================================================"
	@echo " STAGE 1: PREPROCESSING (gcc -E)"
	@echo "======================================================================"
	@echo ""
	@echo "The preprocessor handles: #include, #define, #ifdef, __FILE__, etc."
	@echo "It produces pure C code with no # directives remaining."
	@echo ""
	@echo "Command: $(CC) -E $(STAGES_SRC) -o $(STAGES_PP)"
	$(CC) -E $(STAGES_SRC) -o $(STAGES_PP)
	@echo ""
	@echo "--- Preprocessed output stats ---"
	@echo "Original source:    $$(wc -l < $(STAGES_SRC)) lines"
	@echo "After preprocessing: $$(wc -l < $(STAGES_PP)) lines"
	@echo ""
	@echo "That expansion is mostly from #include <stdio.h>, <stdlib.h>, <string.h>"
	@echo "which pull in hundreds of declarations from system headers."
	@echo ""
	@echo "--- Last 40 lines of preprocessed output (your actual code) ---"
	@tail -40 $(STAGES_PP)
	@echo ""
	@echo "Full output saved to: $(STAGES_PP)"
	@echo "Explore: less $(STAGES_PP)"

# Stage 2: COMPILATION — translate C to assembly language
compile: $(BUILD)
	@echo "======================================================================"
	@echo " STAGE 2: COMPILATION (gcc -S)"
	@echo "======================================================================"
	@echo ""
	@echo "The compiler translates C source into assembly language."
	@echo "Assembly is human-readable but CPU-specific (x86-64 in our case)."
	@echo ""
	@echo "Command: $(CC) $(CFLAGS) -S $(STAGES_SRC) -o $(STAGES_ASM)"
	$(CC) $(CFLAGS) -S $(STAGES_SRC) -o $(STAGES_ASM)
	@echo ""
	@echo "--- Assembly output stats ---"
	@echo "Assembly file: $$(wc -l < $(STAGES_ASM)) lines"
	@echo ""
	@echo "--- Key sections in the assembly ---"
	@echo ""
	@echo "String literals (in .rodata section):"
	@grep -n '\.string\|\.ascii' $(STAGES_ASM) | head -10
	@echo ""
	@echo "Function labels:"
	@grep -n '^[a-zA-Z_][a-zA-Z_0-9]*:' $(STAGES_ASM) | head -10
	@echo ""
	@echo "Function calls:"
	@grep -n 'call' $(STAGES_ASM) | head -10
	@echo ""
	@echo "Full output saved to: $(STAGES_ASM)"
	@echo "Explore: less $(STAGES_ASM)"

# Stage 3: ASSEMBLY — convert assembly to object file (machine code)
assemble: $(BUILD)
	@echo "======================================================================"
	@echo " STAGE 3: ASSEMBLY (gcc -c)"
	@echo "======================================================================"
	@echo ""
	@echo "The assembler converts assembly mnemonics to binary machine code,"
	@echo "producing an object file (.o). This is real machine code, but it's"
	@echo "NOT yet an executable — it has unresolved symbol references."
	@echo ""
	@echo "Command: $(CC) $(CFLAGS) -c $(STAGES_SRC) -o $(STAGES_OBJ)"
	$(CC) $(CFLAGS) -c $(STAGES_SRC) -o $(STAGES_OBJ)
	@echo ""
	@echo "--- Object file type ---"
	@file $(STAGES_OBJ)
	@echo "  (Notice: 'relocatable' — not yet a final executable)"
	@echo ""
	@echo "--- Symbol table (nm) ---"
	@echo "  T = defined in Text (code) section"
	@echo "  D = defined in Data section (initialized globals)"
	@echo "  B = defined in BSS section (uninitialized globals)"
	@echo "  U = Undefined (needs linking to resolve)"
	@echo ""
	@nm $(STAGES_OBJ) 2>/dev/null || echo "  (nm not available or no symbols)"
	@echo ""
	@echo "--- ELF sections (objdump -h) ---"
	@objdump -h $(STAGES_OBJ) 2>/dev/null | head -20
	@echo ""
	@echo "Full object file: $(STAGES_OBJ)"

# Stage 4: LINKING — resolve symbols, produce final executable
link: $(BUILD)
	@echo "======================================================================"
	@echo " STAGE 4: LINKING (gcc -o)"
	@echo "======================================================================"
	@echo ""
	@echo "The linker combines object file(s) with libraries (libc) to produce"
	@echo "the final ELF executable. It resolves all symbol references:"
	@echo "  - printf (defined in libc.so)"
	@echo "  - malloc (defined in libc.so)"
	@echo "  - _start (the real entry point, from crt1.o)"
	@echo ""
	@echo "Command: $(CC) $(CFLAGS) -o $(STAGES_BIN) $(STAGES_SRC)"
	$(CC) $(CFLAGS) -o $(STAGES_BIN) $(STAGES_SRC)
	@echo ""
	@echo "--- Executable file type ---"
	@file $(STAGES_BIN)
	@echo "  (Notice: 'executable' — fully linked, ready to run)"
	@echo ""
	@echo "--- Shared library dependencies (ldd) ---"
	@ldd $(STAGES_BIN) 2>/dev/null || echo "  (ldd not available)"
	@echo ""
	@echo "--- String literals embedded in the binary ---"
	@strings $(STAGES_BIN) | grep -E "(Stage|Section|Preprocessing|compilation)" | head -10
	@echo ""
	@echo "--- Running the program ---"
	@./$(STAGES_BIN)

# Run ALL four stages in sequence with full explanatory output
pipeline: $(BUILD)
	@echo "######################################################################"
	@echo "#                                                                    #"
	@echo "#  MODULE 2: THE COMPLETE C COMPILATION PIPELINE                     #"
	@echo "#                                                                    #"
	@echo "#  .c → Preprocessor → .i → Compiler → .s → Assembler → .o → Linker → ELF  #"
	@echo "#                                                                    #"
	@echo "######################################################################"
	@echo ""
	@$(MAKE) --no-print-directory preprocess
	@echo ""
	@echo ""
	@$(MAKE) --no-print-directory compile
	@echo ""
	@echo ""
	@$(MAKE) --no-print-directory assemble
	@echo ""
	@echo ""
	@$(MAKE) --no-print-directory link
	@echo ""
	@echo ""
	@echo "======================================================================"
	@echo " PIPELINE COMPLETE — Summary of Artifacts"
	@echo "======================================================================"
	@echo ""
	@echo "  Source:        $(STAGES_SRC)    ($$(wc -l < $(STAGES_SRC)) lines)"
	@echo "  Preprocessed:  $(STAGES_PP)     ($$(wc -l < $(STAGES_PP)) lines)"
	@echo "  Assembly:      $(STAGES_ASM)    ($$(wc -l < $(STAGES_ASM)) lines)"
	@echo "  Object:        $(STAGES_OBJ)   ($$(stat -c%s $(STAGES_OBJ) 2>/dev/null || stat -f%z $(STAGES_OBJ) 2>/dev/null) bytes)"
	@echo "  Executable:    $(STAGES_BIN)   ($$(stat -c%s $(STAGES_BIN) 2>/dev/null || stat -f%z $(STAGES_BIN) 2>/dev/null) bytes)"
	@echo ""
	@echo "  Each stage transforms the code closer to what the CPU understands."
	@echo "  The preprocessor handles text. The compiler understands C semantics."
	@echo "  The assembler converts to binary. The linker resolves references."

# ==============================================================================
# Multi-file compilation demo
# ==============================================================================

# Show separate compilation and linking for a multi-file project
multi: $(BUILD)
	@echo "======================================================================"
	@echo " MULTI-FILE COMPILATION: Separate Compilation + Linking"
	@echo "======================================================================"
	@echo ""
	@echo "In a real project, each .c file is compiled independently."
	@echo "The linker then combines the object files into one executable."
	@echo ""
	@echo "--- Step 1: Compile multi_file_main.c → multi_file_main.o ---"
	@echo "Command: $(CC) $(CFLAGS) -c $(MULTI_MAIN) -o $(MULTI_MAIN_O)"
	$(CC) $(CFLAGS) -c $(MULTI_MAIN) -o $(MULTI_MAIN_O)
	@echo ""
	@echo "Symbols in multi_file_main.o:"
	@nm $(MULTI_MAIN_O) 2>/dev/null | grep -E 'T |U ' | sort
	@echo "  Notice: add, multiply, factorial are 'U' (Undefined) — used but not defined here."
	@echo "  Notice: main is 'T' (Text) — defined here."
	@echo ""
	@echo "--- Step 2: Compile math_utils.c → math_utils.o ---"
	@echo "Command: $(CC) $(CFLAGS) -c $(MULTI_UTILS) -o $(MULTI_UTILS_O)"
	$(CC) $(CFLAGS) -c $(MULTI_UTILS) -o $(MULTI_UTILS_O)
	@echo ""
	@echo "Symbols in math_utils.o:"
	@nm $(MULTI_UTILS_O) 2>/dev/null | grep -E 'T |U ' | sort
	@echo "  Notice: add, multiply, factorial are 'T' (Text) — defined here."
	@echo "  Notice: NO main — this file doesn't have one."
	@echo ""
	@echo "--- Step 3: Link both object files → multi_file ---"
	@echo "Command: $(CC) $(CFLAGS) -o $(MULTI_BIN) $(MULTI_MAIN_O) $(MULTI_UTILS_O)"
	$(CC) $(CFLAGS) -o $(MULTI_BIN) $(MULTI_MAIN_O) $(MULTI_UTILS_O)
	@echo ""
	@echo "All symbols resolved:"
	@nm $(MULTI_BIN) 2>/dev/null | grep -E 'add|multiply|factorial|main' | sort
	@echo ""
	@echo "--- Running the linked program ---"
	@echo ""
	@./$(MULTI_BIN)

# ==============================================================================
# Macro preprocessing demo
# ==============================================================================

# Show how macros and conditional compilation work
macros: $(BUILD)
	@echo "======================================================================"
	@echo " MACRO PREPROCESSING DEMO"
	@echo "======================================================================"
	@echo ""
	@echo "--- Without -DDEBUG ---"
	@echo "Command: $(CC) -E $(MACRO_SRC) | tail -30"
	@echo ""
	@$(CC) -E $(MACRO_SRC) 2>/dev/null | tail -30
	@echo ""
	@echo "--- With -DDEBUG ---"
	@echo "Command: $(CC) -E -DDEBUG $(MACRO_SRC) | tail -30"
	@echo ""
	@$(CC) -E -DDEBUG $(MACRO_SRC) 2>/dev/null | tail -30
	@echo ""
	@echo "Notice the differences:"
	@echo "  - Without DEBUG: DEBUG_PRINT calls expand to ((void)0) (nothing)"
	@echo "  - With DEBUG: DEBUG_PRINT calls expand to printf() statements"
	@echo ""
	@echo "--- Building and running both versions ---"
	@echo ""
	@echo "Without DEBUG:"
	$(CC) $(CFLAGS) -o $(MACRO_BIN) $(MACRO_SRC)
	@./$(MACRO_BIN) 2>&1 | grep -A1 "Conditional"
	@echo ""
	@echo "With DEBUG:"
	$(CC) $(CFLAGS) -DDEBUG -o $(MACRO_DBG) $(MACRO_SRC)
	@./$(MACRO_DBG) 2>&1 | grep -A3 "Conditional\|DEBUG"

# ==============================================================================
# Rust pipeline comparison
# ==============================================================================

# Show Rust's compilation stages for comparison with C
rust-pipeline: $(BUILD)
	@echo "======================================================================"
	@echo " RUST COMPILATION PIPELINE (for comparison with C)"
	@echo "======================================================================"
	@echo ""
	@echo "Rust pipeline: .rs → AST → HIR → MIR → LLVM IR → ASM → OBJ → ELF"
	@echo "C pipeline:    .c  → .i (preprocessed) → .s (asm) → .o → ELF"
	@echo ""
	@echo "--- Stage: MIR (Mid-level IR — unique to Rust, used for borrow checking) ---"
	@echo "Command: $(RUSTC) --emit=mir $(RUST_SRC) -o $(BUILD)/pipeline_compare.mir"
	$(RUSTC) --emit=mir $(RUST_SRC) -o $(BUILD)/pipeline_compare.mir
	@echo "MIR output: $$(wc -l < $(BUILD)/pipeline_compare.mir) lines"
	@echo "First 20 lines of MIR:"
	@head -20 $(BUILD)/pipeline_compare.mir
	@echo "..."
	@echo ""
	@echo "--- Stage: LLVM IR (shared with Clang/C — both use LLVM backend) ---"
	@echo "Command: $(RUSTC) --emit=llvm-ir $(RUST_SRC) -o $(BUILD)/pipeline_compare.ll"
	$(RUSTC) --emit=llvm-ir $(RUST_SRC) -o $(BUILD)/pipeline_compare.ll
	@echo "LLVM IR output: $$(wc -l < $(BUILD)/pipeline_compare.ll) lines"
	@echo "First function definition:"
	@grep -A 5 'define.*sum_of_squares' $(BUILD)/pipeline_compare.ll 2>/dev/null | head -10 || echo "  (function may be inlined or mangled)"
	@echo "..."
	@echo ""
	@echo "--- Stage: Assembly (x86-64 — same format as gcc -S output) ---"
	@echo "Command: $(RUSTC) --emit=asm $(RUST_SRC) -o $(BUILD)/pipeline_compare.s"
	$(RUSTC) --emit=asm $(RUST_SRC) -o $(BUILD)/pipeline_compare.s
	@echo "Assembly output: $$(wc -l < $(BUILD)/pipeline_compare.s) lines"
	@echo ""
	@echo "--- Stage: Object file (same as gcc -c output) ---"
	@echo "Command: $(RUSTC) --emit=obj $(RUST_SRC) -o $(BUILD)/pipeline_compare.o"
	$(RUSTC) --emit=obj $(RUST_SRC) -o $(BUILD)/pipeline_compare.o
	@file $(BUILD)/pipeline_compare.o
	@echo ""
	@echo "--- Stage: Final binary (ELF executable) ---"
	@echo "Command: $(RUSTC) $(RUST_SRC) -o $(RUST_BIN)"
	$(RUSTC) $(RUST_SRC) -o $(RUST_BIN)
	@file $(RUST_BIN)
	@echo ""
	@echo "--- Running the Rust pipeline demo ---"
	@echo ""
	@./$(RUST_BIN)
	@echo ""
	@echo "======================================================================"
	@echo " Artifacts produced:"
	@echo "   $(BUILD)/pipeline_compare.mir  — MIR (Rust-specific, borrow-checked)"
	@echo "   $(BUILD)/pipeline_compare.ll   — LLVM IR (shared with Clang/C)"
	@echo "   $(BUILD)/pipeline_compare.s    — Assembly (x86-64)"
	@echo "   $(BUILD)/pipeline_compare.o    — Object file (relocatable ELF)"
	@echo "   $(RUST_BIN)                    — Executable (linked ELF)"
	@echo "======================================================================"

# ==============================================================================
# Cleanup
# ==============================================================================

# Remove all build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD)
	@echo "Clean complete."

# ==============================================================================
# Help
# ==============================================================================

help:
	@echo "Module 2: C Compilation Pipeline — Available Targets"
	@echo ""
	@echo "  Pipeline stages (run individually):"
	@echo "    make preprocess    Stage 1: gcc -E  (macro expansion, include resolution)"
	@echo "    make compile       Stage 2: gcc -S  (C → assembly)"
	@echo "    make assemble      Stage 3: gcc -c  (assembly → object file)"
	@echo "    make link          Stage 4: gcc -o  (object → executable)"
	@echo ""
	@echo "  Combined:"
	@echo "    make all           Build all executables"
	@echo "    make pipeline      Run all 4 stages with explanatory banners"
	@echo ""
	@echo "  Special demos:"
	@echo "    make multi         Multi-file separate compilation + linking"
	@echo "    make macros        Preprocess macro_demo.c with/without -DDEBUG"
	@echo "    make rust-pipeline Show Rust compilation stages for comparison"
	@echo ""
	@echo "  Maintenance:"
	@echo "    make clean         Remove all build artifacts"
	@echo "    make help          Show this help message"
