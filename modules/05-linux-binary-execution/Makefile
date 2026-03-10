# ============================================================================
# Module 05: How Linux Runs a Binary — Makefile
# ============================================================================
#
# This Makefile builds and runs the exercises for Module 5.
# Each target demonstrates a different aspect of how Linux executes binaries.
#
# Usage:
#   make all            — Build all C and Rust programs
#   make syscall        — Build and run the direct syscall demo
#   make exec           — Build the execve demo
#   make memory         — Build and run the memory layout explorer
#   make entry          — Build and run the entry point chain demo
#   make mini-shell     — Build and run the mini shell
#   make strace         — Run strace on a simple program
#   make strace-startup — Trace only the startup syscalls (execve, mmap, etc.)
#   make maps           — Show /proc/PID/maps for the memory layout program
#   make dynamic-linker — Run ld-linux manually on a binary
#   make rust-syscall   — Build and run the Rust syscall demo
#   make clean          — Remove all build artifacts
# ============================================================================

CC      := gcc
CFLAGS  := -Wall -Wextra -g -O0
RUSTC   := rustc

# Output directory for binaries
BINDIR  := bin

# C sources and targets
C_SRCS  := c/syscall_direct.c c/exec_demo.c c/memory_layout.c \
           c/entry_point.c c/fork_exec.c
C_BINS  := $(BINDIR)/syscall_direct $(BINDIR)/exec_demo $(BINDIR)/memory_layout \
           $(BINDIR)/entry_point $(BINDIR)/mini_shell

# Rust sources and targets
RUST_SRCS := rust/syscall_demo.rs
RUST_BINS := $(BINDIR)/syscall_demo_rs

# ============================================================================
# Primary targets
# ============================================================================

.PHONY: all clean syscall exec memory entry mini-shell strace strace-startup \
        maps dynamic-linker rust-syscall help

all: $(C_BINS) $(RUST_BINS)
	@echo ""
	@echo "=== All Module 05 binaries built in $(BINDIR)/ ==="
	@echo "Run 'make help' to see available targets."

help:
	@echo "Module 05: How Linux Runs a Binary"
	@echo "==================================="
	@echo ""
	@echo "Build targets:"
	@echo "  make all            — Build everything"
	@echo "  make clean          — Remove build artifacts"
	@echo ""
	@echo "Run targets:"
	@echo "  make syscall        — Direct syscall via inline asm vs libc"
	@echo "  make exec           — execve() demo: fork + exec + wait"
	@echo "  make memory         — Explore process memory layout"
	@echo "  make entry          — Startup chain: constructors → main → destructors"
	@echo "  make mini-shell     — A tiny shell (fork+exec loop)"
	@echo "  make strace         — Trace ALL syscalls of a program"
	@echo "  make strace-startup — Trace only startup syscalls (execve, mmap, ...)"
	@echo "  make maps           — Show /proc/PID/maps for memory layout"
	@echo "  make dynamic-linker — Run the dynamic linker (ld-linux) manually"
	@echo "  make rust-syscall   — Rust version of the syscall demo"

# ============================================================================
# Build rules
# ============================================================================

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/syscall_direct: c/syscall_direct.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $<
	@echo "Built: $@"

$(BINDIR)/exec_demo: c/exec_demo.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $<
	@echo "Built: $@"

$(BINDIR)/memory_layout: c/memory_layout.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $<
	@echo "Built: $@"

$(BINDIR)/entry_point: c/entry_point.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $<
	@echo "Built: $@"

$(BINDIR)/mini_shell: c/fork_exec.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $<
	@echo "Built: $@"

$(BINDIR)/syscall_demo_rs: rust/syscall_demo.rs | $(BINDIR)
	$(RUSTC) -g -o $@ $<
	@echo "Built: $@"

# ============================================================================
# Run targets
# ============================================================================

# --- Direct syscall demo ---
# Shows the difference between libc's write() and a raw syscall instruction.
# OBSERVE: Both produce identical results at the kernel level.
syscall: $(BINDIR)/syscall_direct
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Running: Direct Syscall Demo"
	@echo " What to observe: libc write() vs raw syscall — same result!"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	./$(BINDIR)/syscall_direct

# --- execve demo ---
# Shows fork() + execve() — how the kernel replaces a process image.
# OBSERVE: Child's code is completely replaced after execve().
exec: $(BINDIR)/exec_demo
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Running: execve() Demonstration"
	@echo " What to observe: execve replaces the child's entire memory"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	./$(BINDIR)/exec_demo

# --- Memory layout explorer ---
# Prints addresses of variables in each memory region.
# OBSERVE: .text at low addresses, stack at high addresses.
memory: $(BINDIR)/memory_layout
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Running: Memory Layout Explorer"
	@echo " What to observe: address ordering and /proc/self/maps"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	./$(BINDIR)/memory_layout

# --- Entry point chain demo ---
# Shows that main() is NOT the first thing that runs.
# OBSERVE: constructors → main → atexit → destructors
entry: $(BINDIR)/entry_point
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Running: Entry Point Chain Demo"
	@echo " What to observe: constructors run BEFORE main!"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	./$(BINDIR)/entry_point
	@echo ""
	@echo "--- Now let's see the REAL entry point in the ELF header ---"
	@echo ""
	readelf -h $(BINDIR)/entry_point 2>/dev/null | grep -E "Entry point|Type" || true
	@echo ""
	@echo "--- .init_array (constructor table) and .fini_array (destructor table) ---"
	@echo ""
	readelf -S $(BINDIR)/entry_point 2>/dev/null | grep -E "init_array|fini_array" || true

# --- Mini shell ---
# A minimal fork+exec shell. Type commands interactively.
# OBSERVE: This is the fundamental loop that bash uses.
mini-shell: $(BINDIR)/mini_shell
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Running: Mini Shell (fork+exec loop)"
	@echo " Try: ls, echo hello, cd /tmp, exit"
	@echo " For insight: strace -f ./$(BINDIR)/mini_shell"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	./$(BINDIR)/mini_shell

# --- strace: Trace ALL syscalls ---
# Shows every syscall a program makes, from start to finish.
# OBSERVE: The sheer number of syscalls even a simple program makes.
strace: $(BINDIR)/entry_point
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Running: strace on entry_point"
	@echo " What to observe: every syscall from start to exit"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	strace ./$(BINDIR)/entry_point 2>&1 || echo "(Install strace: sudo apt install strace)"

# --- strace: Focus on startup sequence ---
# Filters for just the syscalls that happen during program startup.
# OBSERVE: execve, mmap (for loading), mprotect, etc.
strace-startup: $(BINDIR)/entry_point
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Running: strace filtered for startup syscalls"
	@echo " What to observe: execve → mmap (loading) → the program"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	strace -e trace=execve,mmap,mprotect,openat,read,close,arch_prctl,set_tid_address \
	    ./$(BINDIR)/entry_point 2>&1 || echo "(Install strace: sudo apt install strace)"

# --- /proc/PID/maps ---
# Shows the memory map of the running process.
# We run memory_layout which prints its own maps, but also demonstrate
# reading maps from outside the process.
maps: $(BINDIR)/memory_layout
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Running: memory_layout (it reads its own /proc/self/maps)"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	./$(BINDIR)/memory_layout
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Bonus: Showing maps of a sleeping process from outside"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	@sleep 1000 & SLEEP_PID=$$!; \
	echo "Started 'sleep 1000' as PID $$SLEEP_PID"; \
	echo ""; \
	cat /proc/$$SLEEP_PID/maps 2>/dev/null || echo "Could not read maps"; \
	kill $$SLEEP_PID 2>/dev/null; \
	echo ""; \
	echo "Killed sleep process."

# --- Dynamic linker: Run ld-linux manually ---
# The dynamic linker (ld-linux-x86-64.so.2) is what ACTUALLY starts
# dynamically linked programs. The kernel runs ld-linux, which then
# loads your binary. You can invoke it directly!
# OBSERVE: ld-linux can run a binary just like ./binary does.
dynamic-linker: $(BINDIR)/entry_point
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Running: Dynamic linker (ld-linux) manually"
	@echo " What to observe: ld-linux can run binaries directly!"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	@echo "--- Finding the dynamic linker ---"
	@LDLINUX=$$(readelf -l $(BINDIR)/entry_point 2>/dev/null \
	    | grep 'interpreter' \
	    | sed 's/.*: \(.*\)]/\1/'); \
	if [ -n "$$LDLINUX" ] && [ -x "$$LDLINUX" ]; then \
	    echo "Dynamic linker: $$LDLINUX"; \
	    echo ""; \
	    echo "--- Running entry_point via the dynamic linker ---"; \
	    echo "  Command: $$LDLINUX ./$(BINDIR)/entry_point"; \
	    echo ""; \
	    $$LDLINUX ./$(BINDIR)/entry_point; \
	    echo ""; \
	    echo "--- What just happened ---"; \
	    echo "Normally: kernel sees PT_INTERP → loads ld-linux → ld-linux loads your binary"; \
	    echo "We did:   ran ld-linux directly → ld-linux loaded entry_point → ran it"; \
	    echo "Same result! The dynamic linker is the REAL program loader."; \
	else \
	    echo "Could not find dynamic linker. Is this a static binary?"; \
	    echo "Interpreter field: $$LDLINUX"; \
	fi

# --- Rust syscall demo ---
# Same concept as syscall_direct.c but in Rust.
# OBSERVE: Rust's std wraps libc, which wraps syscalls — same stack.
rust-syscall: $(BINDIR)/syscall_demo_rs
	@echo ""
	@echo "════════════════════════════════════════════════════════"
	@echo " Running: Rust Syscall Demo"
	@echo " What to observe: Rust's std → libc → syscall, same as C"
	@echo "════════════════════════════════════════════════════════"
	@echo ""
	./$(BINDIR)/syscall_demo_rs

# ============================================================================
# Clean
# ============================================================================

clean:
	rm -rf $(BINDIR)
	@echo "Cleaned Module 05 build artifacts."
