# =============================================================================
# Makefile — Module 10: Reverse Engineering a C Binary
# =============================================================================
#
# This module builds programs in BOTH debug and stripped versions.
# The stripped versions are the "challenges" — analyze them without source!
# The debug versions are the "solutions" — check your answers.
#
# Usage:
#   make all              — build all stripped binaries (the challenges)
#   make crackme          — build and strip the crackme challenge
#   make mystery          — build and strip the mystery program
#   make function-finder  — build and strip the function finder
#   make anti-debug       — build the anti-debugging demo
#   make rust-stripped     — build and strip the Rust binary
#   make solutions        — build debug (unstripped) versions for checking answers
#   make strings-crackme  — run strings on stripped crackme
#   make strace-mystery   — run strace on mystery binary
#   make ltrace-mystery   — run ltrace on mystery binary
#   make disasm-crackme   — run objdump -d on stripped crackme
#   make workflow         — full RE workflow on mystery binary
#   make clean            — remove all compiled binaries
# =============================================================================

CC := gcc
CFLAGS := -Wall -Wextra
RUSTC := rustc

# Directories
C_DIR := c
RUST_DIR := rust
BUILD_DIR := build

# Output binaries
CRACKME_STRIPPED := $(BUILD_DIR)/crackme
CRACKME_DEBUG := $(BUILD_DIR)/crackme_debug
MYSTERY_STRIPPED := $(BUILD_DIR)/mystery
MYSTERY_DEBUG := $(BUILD_DIR)/mystery_debug
FUNCFINDER_STRIPPED := $(BUILD_DIR)/function_finder
FUNCFINDER_DEBUG := $(BUILD_DIR)/function_finder_debug
ANTIDEBUG := $(BUILD_DIR)/anti_debug
RUST_STRIPPED := $(BUILD_DIR)/stripped_demo
RUST_DEBUG := $(BUILD_DIR)/stripped_demo_debug

.PHONY: all crackme mystery function-finder anti-debug rust-stripped \
        solutions strings-crackme strace-mystery ltrace-mystery \
        disasm-crackme disasm-mystery disasm-function-finder \
        workflow clean help

# =============================================================================
# Default: build all stripped binaries
# =============================================================================
all: crackme mystery function-finder anti-debug rust-stripped
	@echo ""
	@echo "========================================================"
	@echo "  All binaries built in $(BUILD_DIR)/"
	@echo "========================================================"
	@echo ""
	@echo "  CHALLENGES (stripped — no debug symbols):"
	@ls -la $(CRACKME_STRIPPED) $(MYSTERY_STRIPPED) $(FUNCFINDER_STRIPPED) 2>/dev/null || true
	@echo ""
	@echo "  TOOLS (not stripped):"
	@ls -la $(ANTIDEBUG) 2>/dev/null || true
	@echo ""
	@if [ -f "$(RUST_STRIPPED)" ]; then \
		echo "  RUST (stripped):"; \
		ls -la $(RUST_STRIPPED); \
		echo ""; \
	fi
	@echo "  Try these commands:"
	@echo "    make strings-crackme     — look for strings in crackme"
	@echo "    make disasm-crackme      — disassemble crackme"
	@echo "    make workflow            — full RE workflow on mystery"
	@echo "    make solutions           — build unstripped versions"

# =============================================================================
# Build directory
# =============================================================================
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# =============================================================================
# CRACKME — The password challenge
# =============================================================================
# Build with -O0 (no optimization) so the disassembly is readable.
# Then strip to remove symbol names, making it a challenge.
# =============================================================================
crackme: $(BUILD_DIR)
	@echo "Building crackme (stripped)..."
	$(CC) $(CFLAGS) -O0 -o $(CRACKME_STRIPPED) $(C_DIR)/crackme.c
	strip $(CRACKME_STRIPPED)
	@echo "  Built: $(CRACKME_STRIPPED) (stripped)"
	@echo "  Size:  $$(stat -c%s $(CRACKME_STRIPPED) 2>/dev/null || stat -f%z $(CRACKME_STRIPPED) 2>/dev/null) bytes"
	@echo "  Try:   $(CRACKME_STRIPPED) <password>"
	@echo "  Hint:  The password has $$($(CRACKME_STRIPPED) wrong 2>&1 | grep -o '[0-9]* characters' || echo 'some') characters"

# =============================================================================
# MYSTERY — The "what does it do?" challenge
# =============================================================================
mystery: $(BUILD_DIR)
	@echo "Building mystery (stripped)..."
	$(CC) $(CFLAGS) -O0 -o $(MYSTERY_STRIPPED) $(C_DIR)/mystery.c
	strip $(MYSTERY_STRIPPED)
	@echo "  Built: $(MYSTERY_STRIPPED) (stripped)"
	@echo "  Try:   $(MYSTERY_STRIPPED)"
	@echo "  Then:  make strace-mystery"

# =============================================================================
# FUNCTION FINDER — Identify the functions challenge
# =============================================================================
function-finder: $(BUILD_DIR)
	@echo "Building function_finder (stripped)..."
	$(CC) $(CFLAGS) -O0 -o $(FUNCFINDER_STRIPPED) $(C_DIR)/function_finder.c
	strip $(FUNCFINDER_STRIPPED)
	@echo "  Built: $(FUNCFINDER_STRIPPED) (stripped)"
	@echo "  Try:   $(FUNCFINDER_STRIPPED)"
	@echo "  Then:  strings $(FUNCFINDER_STRIPPED)"

# =============================================================================
# ANTI-DEBUG — Educational, not stripped (you need to read the output)
# =============================================================================
anti-debug: $(BUILD_DIR)
	@echo "Building anti_debug..."
	$(CC) $(CFLAGS) -O0 -o $(ANTIDEBUG) $(C_DIR)/anti_debug.c
	@echo "  Built: $(ANTIDEBUG)"
	@echo "  Try:   $(ANTIDEBUG)"
	@echo "  Then:  gdb $(ANTIDEBUG)  (and watch it detect you!)"

# =============================================================================
# RUST — Stripped Rust binary for comparison
# =============================================================================
rust-stripped: $(BUILD_DIR)
	@echo "Building Rust stripped demo..."
	@if command -v $(RUSTC) >/dev/null 2>&1; then \
		$(RUSTC) -o $(RUST_STRIPPED) $(RUST_DIR)/stripped_demo.rs 2>&1 && \
		echo "  Pre-strip size: $$(stat -c%s $(RUST_STRIPPED) 2>/dev/null || stat -f%z $(RUST_STRIPPED) 2>/dev/null) bytes" && \
		strip $(RUST_STRIPPED) && \
		echo "  Post-strip size: $$(stat -c%s $(RUST_STRIPPED) 2>/dev/null || stat -f%z $(RUST_STRIPPED) 2>/dev/null) bytes" && \
		echo "  Built: $(RUST_STRIPPED) (stripped)" && \
		echo "  Compare: C crackme is much smaller!"; \
	else \
		echo "  SKIPPED: rustc not found (install Rust to build this)"; \
	fi

# =============================================================================
# SOLUTIONS — Debug (unstripped) versions for checking answers
# =============================================================================
solutions: $(BUILD_DIR)
	@echo "Building debug (unstripped) versions..."
	$(CC) $(CFLAGS) -O0 -g -o $(CRACKME_DEBUG) $(C_DIR)/crackme.c
	$(CC) $(CFLAGS) -O0 -g -o $(MYSTERY_DEBUG) $(C_DIR)/mystery.c
	$(CC) $(CFLAGS) -O0 -g -o $(FUNCFINDER_DEBUG) $(C_DIR)/function_finder.c
	@if command -v $(RUSTC) >/dev/null 2>&1; then \
		$(RUSTC) -g -o $(RUST_DEBUG) $(RUST_DIR)/stripped_demo.rs 2>&1; \
	fi
	@echo ""
	@echo "  Debug binaries (with symbols):"
	@ls -la $(BUILD_DIR)/*_debug 2>/dev/null
	@echo ""
	@echo "  Use these to check your RE answers:"
	@echo "    objdump -d $(CRACKME_DEBUG) | less     — see function names"
	@echo "    nm $(FUNCFINDER_DEBUG)                  — list all symbols"
	@echo "    gdb $(MYSTERY_DEBUG)                    — step with source info"

# =============================================================================
# ANALYSIS TARGETS — Run RE tools on the challenges
# =============================================================================

strings-crackme: crackme
	@echo "========================================================"
	@echo "  Running 'strings' on stripped crackme"
	@echo "  Look for: readable text, format strings, interesting constants"
	@echo "========================================================"
	strings $(CRACKME_STRIPPED)
	@echo ""
	@echo "  NOTE: The password is NOT in plaintext — it's encoded."
	@echo "  But you can see format strings, error messages, and hints."

strace-mystery: mystery
	@echo "========================================================"
	@echo "  Running 'strace' on mystery binary"
	@echo "  Look for: file access, network calls, system interactions"
	@echo "========================================================"
	strace $(MYSTERY_STRIPPED) 2>&1 || echo "(strace may need to be installed: sudo apt install strace)"

ltrace-mystery: mystery
	@echo "========================================================"
	@echo "  Running 'ltrace' on mystery binary"
	@echo "  Look for: printf, strlen, memset, putchar — library calls"
	@echo "  These reveal what functions the program uses internally"
	@echo "========================================================"
	ltrace $(MYSTERY_STRIPPED) 2>&1 || echo "(ltrace may need to be installed: sudo apt install ltrace)"

disasm-crackme: crackme
	@echo "========================================================"
	@echo "  Disassembling stripped crackme with objdump"
	@echo "  Look for: XOR operations, comparison loops, constants"
	@echo "========================================================"
	objdump -d $(CRACKME_STRIPPED) | head -200
	@echo ""
	@echo "  (showing first 200 lines — pipe to less for full output)"
	@echo "  Full command: objdump -d $(CRACKME_STRIPPED) | less"

disasm-mystery: mystery
	@echo "========================================================"
	@echo "  Disassembling stripped mystery with objdump"
	@echo "========================================================"
	objdump -d $(MYSTERY_STRIPPED) | head -200
	@echo ""
	@echo "  Full command: objdump -d $(MYSTERY_STRIPPED) | less"

disasm-function-finder: function-finder
	@echo "========================================================"
	@echo "  Disassembling stripped function_finder with objdump"
	@echo "========================================================"
	objdump -d $(FUNCFINDER_STRIPPED) | head -200
	@echo ""
	@echo "  Full command: objdump -d $(FUNCFINDER_STRIPPED) | less"

# =============================================================================
# WORKFLOW — Full RE workflow on the mystery binary
# =============================================================================
# This runs the standard reverse engineering process:
#   1. file — identify the binary type
#   2. strings — extract readable strings
#   3. strace — trace system calls
#   4. ltrace — trace library calls
#   5. objdump — disassemble
# =============================================================================
workflow: mystery
	@echo "========================================================"
	@echo "  FULL REVERSE ENGINEERING WORKFLOW"
	@echo "  Target: $(MYSTERY_STRIPPED)"
	@echo "========================================================"
	@echo ""
	@echo "=== STEP 1: file (identify binary type) ==="
	file $(MYSTERY_STRIPPED)
	@echo ""
	@echo "=== STEP 2: strings (extract readable text) ==="
	@echo "(first 30 interesting strings)"
	strings $(MYSTERY_STRIPPED) | head -30
	@echo "..."
	@echo "(total strings: $$(strings $(MYSTERY_STRIPPED) | wc -l))"
	@echo ""
	@echo "=== STEP 3: strace (system calls) ==="
	@echo "(running with 2>&1 to capture output)"
	strace -c $(MYSTERY_STRIPPED) >/dev/null 2>&1 && strace -c $(MYSTERY_STRIPPED) 2>&1 | tail -20 || echo "(strace not available)"
	@echo ""
	@echo "=== STEP 4: ltrace (library calls) ==="
	@echo "(running with -c for summary)"
	ltrace -c $(MYSTERY_STRIPPED) 2>&1 | tail -20 || echo "(ltrace not available)"
	@echo ""
	@echo "=== STEP 5: objdump (disassembly excerpt) ==="
	@echo "(showing main and first few functions)"
	objdump -d $(MYSTERY_STRIPPED) | head -100
	@echo ""
	@echo "========================================================"
	@echo "  WORKFLOW COMPLETE"
	@echo "  Now you have clues to figure out what the program does!"
	@echo "  Next steps:"
	@echo "    - Analyze string constants for algorithm clues"
	@echo "    - Map library calls to understand data flow"
	@echo "    - Read the disassembly to understand logic"
	@echo "    - Use gdb to set breakpoints and inspect memory"
	@echo "========================================================"

# =============================================================================
# Clean
# =============================================================================
clean:
	@echo "Cleaning up compiled binaries..."
	rm -rf $(BUILD_DIR)
	@echo "Clean complete."

# =============================================================================
# Help
# =============================================================================
help:
	@echo "Module 10: Reverse Engineering a C Binary"
	@echo ""
	@echo "BUILD TARGETS:"
	@echo "  make all              — build all stripped challenge binaries"
	@echo "  make crackme          — build stripped crackme (password challenge)"
	@echo "  make mystery          — build stripped mystery program"
	@echo "  make function-finder  — build stripped function finder"
	@echo "  make anti-debug       — build anti-debugging demo"
	@echo "  make rust-stripped    — build stripped Rust binary"
	@echo "  make solutions        — build unstripped versions (answers)"
	@echo ""
	@echo "ANALYSIS TARGETS:"
	@echo "  make strings-crackme       — run strings on crackme"
	@echo "  make strace-mystery        — run strace on mystery"
	@echo "  make ltrace-mystery        — run ltrace on mystery"
	@echo "  make disasm-crackme        — disassemble crackme"
	@echo "  make disasm-mystery        — disassemble mystery"
	@echo "  make disasm-function-finder — disassemble function finder"
	@echo "  make workflow              — full RE workflow on mystery"
	@echo ""
	@echo "MAINTENANCE:"
	@echo "  make clean            — remove all compiled binaries"
	@echo "  make help             — show this message"
	@echo ""
	@echo "SUGGESTED ORDER:"
	@echo "  1. make crackme && ./build/crackme test"
	@echo "  2. make strings-crackme (look for clues)"
	@echo "  3. make disasm-crackme (find the password check)"
	@echo "  4. make mystery && make workflow"
	@echo "  5. make function-finder && ./build/function_finder"
	@echo "  6. make anti-debug && ./build/anti_debug"
