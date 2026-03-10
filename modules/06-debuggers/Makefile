# ============================================================================
# Module 06: Debuggers at the Binary Level — Makefile
# ============================================================================
#
# This Makefile builds the exercises for Module 6 (GDB practice).
# All C files are compiled with -g -O0 for maximum debug info and
# no optimizations (so the code maps cleanly to source lines).
#
# Usage:
#   make all             — Build all programs
#   make debug           — Build debug_me with debug symbols
#   make buggy           — Build find_the_bug with debug symbols
#   make segfault        — Build segfault with debug symbols
#   make stripped        — Build two versions: with symbols and stripped
#   make rust-debug      — Build Rust debug_me with debug info
#   make gdb-debug       — Launch GDB with debug_me
#   make gdb-buggy       — Launch GDB with find_the_bug
#   make gdb-segfault    — Launch GDB with segfault
#   make compare-stripped — Show what GDB sees with symbols vs without
#   make clean           — Remove all build artifacts
# ============================================================================

CC      := gcc
CFLAGS  := -Wall -Wextra -g -O0
RUSTC   := rustc

# find_the_bug.c intentionally uses an uninitialized variable
BUGGY_CFLAGS := -Wall -Wextra -g -O0 -Wno-uninitialized

# Output directory for binaries
BINDIR  := bin

# Targets
C_BINS    := $(BINDIR)/debug_me $(BINDIR)/find_the_bug $(BINDIR)/segfault \
             $(BINDIR)/with_symbols $(BINDIR)/stripped
RUST_BINS := $(BINDIR)/debug_me_rs

# ============================================================================
# Primary targets
# ============================================================================

.PHONY: all clean debug buggy segfault stripped rust-debug \
        gdb-debug gdb-buggy gdb-segfault compare-stripped help

all: $(C_BINS) $(RUST_BINS)
	@echo ""
	@echo "=== All Module 06 binaries built in $(BINDIR)/ ==="
	@echo ""
	@echo "Next steps:"
	@echo "  1. Read gdb_cheatsheet.txt for the command reference"
	@echo "  2. Follow gdb_exercises.txt for guided practice"
	@echo "  3. Or jump in: make gdb-debug"

help:
	@echo "Module 06: Debuggers at the Binary Level"
	@echo "========================================="
	@echo ""
	@echo "Build targets:"
	@echo "  make all              — Build everything"
	@echo "  make debug            — Build debug_me (no bugs, for GDB practice)"
	@echo "  make buggy            — Build find_the_bug (3 bugs to find)"
	@echo "  make segfault         — Build segfault (crashes — diagnose with GDB)"
	@echo "  make stripped         — Build with_symbols and stripped versions"
	@echo "  make rust-debug       — Build Rust debug_me"
	@echo "  make clean            — Remove build artifacts"
	@echo ""
	@echo "GDB launch targets:"
	@echo "  make gdb-debug        — Open debug_me in GDB"
	@echo "  make gdb-buggy        — Open find_the_bug in GDB"
	@echo "  make gdb-segfault     — Open segfault in GDB"
	@echo "  make compare-stripped — Compare debugging with/without symbols"

# ============================================================================
# Build rules
# ============================================================================

$(BINDIR):
	mkdir -p $(BINDIR)

# --- debug_me: No bugs, designed for GDB practice ---
# Compiled with -g (debug symbols) and -O0 (no optimization)
# so that variables aren't optimized away and source lines map cleanly.
$(BINDIR)/debug_me: c/debug_me.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< -lm
	@echo "Built: $@ (with debug symbols, for GDB practice)"

debug: $(BINDIR)/debug_me

# --- find_the_bug: 3 intentional bugs ---
# Uses -Wno-uninitialized because Bug 2 is an uninitialized variable.
# We don't want the compiler warning to spoil the exercise!
$(BINDIR)/find_the_bug: c/find_the_bug.c | $(BINDIR)
	$(CC) $(BUGGY_CFLAGS) -o $@ $<
	@echo "Built: $@ (with 3 bugs — find them with GDB!)"

buggy: $(BINDIR)/find_the_bug

# --- segfault: Crashes with SIGSEGV ---
$(BINDIR)/segfault: c/segfault.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $<
	@echo "Built: $@ (will crash — diagnose with GDB)"

segfault: $(BINDIR)/segfault

# --- stripped_vs_unstripped: Two versions of the same program ---
# Version 1: Full debug symbols (-g)
# Version 2: All symbols removed (strip)
$(BINDIR)/with_symbols: c/stripped_vs_unstripped.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< -lm
	@echo "Built: $@ (with full debug symbols)"

$(BINDIR)/stripped: c/stripped_vs_unstripped.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< -lm
	strip $@
	@echo "Built: $@ (all symbols removed)"

stripped: $(BINDIR)/with_symbols $(BINDIR)/stripped
	@echo ""
	@echo "--- Size comparison ---"
	@ls -la $(BINDIR)/with_symbols $(BINDIR)/stripped
	@echo ""
	@echo "The stripped version is smaller — all debug info has been removed."
	@echo "Try: make compare-stripped"

# --- Rust debug binary ---
$(BINDIR)/debug_me_rs: rust/debug_me.rs | $(BINDIR)
	$(RUSTC) -g -o $@ $<
	@echo "Built: $@ (Rust with debug symbols)"

rust-debug: $(BINDIR)/debug_me_rs

# ============================================================================
# GDB launch targets
# ============================================================================

# --- Launch GDB with debug_me ---
# Follow Exercise 1 in gdb_exercises.txt
gdb-debug: $(BINDIR)/debug_me
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Launching GDB with debug_me"
	@echo ""
	@echo " Quick start:"
	@echo "   (gdb) break main"
	@echo "   (gdb) run"
	@echo "   (gdb) next        — step one line"
	@echo "   (gdb) step        — step into function"
	@echo "   (gdb) print var   — inspect variable"
	@echo "   (gdb) backtrace   — show call stack"
	@echo "   (gdb) quit        — exit GDB"
	@echo ""
	@echo " Full guide: gdb_exercises.txt (Exercise 1)"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	gdb -q ./$(BINDIR)/debug_me

# --- Launch GDB with find_the_bug ---
# Follow Exercise 2 in gdb_exercises.txt
gdb-buggy: $(BINDIR)/find_the_bug
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Launching GDB with find_the_bug"
	@echo ""
	@echo " This program has 3 bugs. Find them!"
	@echo "   Bug 1: Off-by-one array access"
	@echo "   Bug 2: Uninitialized variable"
	@echo "   Bug 3: Integer overflow"
	@echo ""
	@echo " Quick start:"
	@echo "   (gdb) break compute_average"
	@echo "   (gdb) break find_maximum"
	@echo "   (gdb) break compute_product"
	@echo "   (gdb) run"
	@echo ""
	@echo " Full guide: gdb_exercises.txt (Exercise 2)"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	gdb -q ./$(BINDIR)/find_the_bug

# --- Launch GDB with segfault ---
# Follow Exercise 3 in gdb_exercises.txt
gdb-segfault: $(BINDIR)/segfault
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Launching GDB with segfault"
	@echo ""
	@echo " This program WILL crash. Your job: find WHY."
	@echo ""
	@echo " Quick start:"
	@echo "   (gdb) run"
	@echo "   ... SIGSEGV ..."
	@echo "   (gdb) backtrace"
	@echo "   (gdb) frame 0"
	@echo "   (gdb) print config"
	@echo ""
	@echo " Full guide: gdb_exercises.txt (Exercise 3)"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	gdb -q ./$(BINDIR)/segfault

# --- Compare stripped vs unstripped in GDB ---
# Follow Exercise 4 in gdb_exercises.txt
compare-stripped: $(BINDIR)/with_symbols $(BINDIR)/stripped
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Comparing: Debug Symbols vs Stripped"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	@echo "--- File sizes ---"
	@ls -la $(BINDIR)/with_symbols $(BINDIR)/stripped
	@echo ""
	@echo "--- Symbol table (with_symbols) ---"
	@echo "  Functions found by nm:"
	@nm $(BINDIR)/with_symbols 2>/dev/null | grep -c ' T ' || true
	@echo ""
	@echo "--- Symbol table (stripped) ---"
	@nm $(BINDIR)/stripped 2>/dev/null || echo "  nm: no symbols (as expected!)"
	@echo ""
	@echo "--- Debug sections (with_symbols) ---"
	@readelf -S $(BINDIR)/with_symbols 2>/dev/null | grep -c debug || echo "  0 debug sections"
	@echo " debug sections found"
	@echo ""
	@echo "--- Debug sections (stripped) ---"
	@readelf -S $(BINDIR)/stripped 2>/dev/null | grep -c debug || echo "  0"
	@echo " debug sections found"
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Now try debugging each one:"
	@echo ""
	@echo "   gdb -q ./$(BINDIR)/with_symbols"
	@echo "   (gdb) break main           ← works!"
	@echo "   (gdb) run"
	@echo "   (gdb) list                  ← shows source!"
	@echo "   (gdb) info locals           ← shows variables!"
	@echo "   (gdb) quit"
	@echo ""
	@echo "   gdb -q ./$(BINDIR)/stripped"
	@echo "   (gdb) break main           ← FAILS: no symbols"
	@echo "   (gdb) info functions         ← nothing useful"
	@echo "   (gdb) quit"
	@echo ""
	@echo " Full guide: gdb_exercises.txt (Exercise 4)"
	@echo "════════════════════════════════════════════════════════"

# ============================================================================
# Clean
# ============================================================================

clean:
	rm -rf $(BINDIR)
	@echo "Cleaned Module 06 build artifacts."
