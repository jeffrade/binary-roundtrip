# ==============================================================================
# MODULE 3: ELF Binary Format — Makefile
# ==============================================================================
#
# This Makefile builds all exercises and provides targets for exploring
# the ELF binary format using readelf, nm, objdump, and other tools.
#
# Quick Start:
#   make all          — Build everything
#   make elf-dump     — Compile elf_explorer and examine its ELF structure
#   make sections     — Show all ELF sections
#   make symbols      — Show symbol table (local vs global)
#   make disasm       — Disassemble the .text section
#   make dynamic      — Explore dynamic linking (PLT/GOT)
#   make shared-lib   — Build and use a shared library
#   make static-link  — Compare static vs dynamic binary sizes
#   make rust-elf     — Build Rust binary and compare ELF structure
#   make java-compare — Show .class is NOT ELF
#   make hexdump      — Raw hex of ELF header
#   make clean        — Remove all build artifacts
#
# ==============================================================================

CC      = gcc
CFLAGS  = -Wall -Wextra
RUSTC   = rustc
JAVAC   = javac

# Output directories (build in-place for simplicity)
C_DIR    = c
RUST_DIR = rust
JAVA_DIR = java

# C targets
ELF_EXPLORER     = $(C_DIR)/elf_explorer
STATIC_VS_DYN    = $(C_DIR)/static_vs_dynamic
DYNAMIC_BINARY   = $(C_DIR)/dynamic_binary
STATIC_BINARY    = $(C_DIR)/static_binary
LIBMATH_SO       = $(C_DIR)/libmath.so
LIBMATH_OBJ      = $(C_DIR)/libmath.o
USE_LIBMATH      = $(C_DIR)/use_libmath

# Rust targets
RUST_ELF_SECTIONS = $(RUST_DIR)/elf_sections

# Java targets
JAVA_CLASS = $(JAVA_DIR)/ElfCompare.class

# ==============================================================================
# Primary targets
# ==============================================================================

.PHONY: all clean elf-dump sections symbols disasm dynamic shared-lib \
        static-link rust-elf java-compare hexdump

all: $(ELF_EXPLORER) $(STATIC_VS_DYN) $(LIBMATH_SO) $(USE_LIBMATH) \
     $(RUST_ELF_SECTIONS) $(JAVA_CLASS)
	@echo ""
	@echo "=== Module 3: All targets built successfully ==="
	@echo "Run 'make elf-dump' to start exploring ELF structure."

# ==============================================================================
# Build rules
# ==============================================================================

$(ELF_EXPLORER): $(C_DIR)/elf_explorer.c
	$(CC) $(CFLAGS) -o $@ $<
	@echo "Built: $@"

$(STATIC_VS_DYN): $(C_DIR)/static_vs_dynamic.c
	$(CC) $(CFLAGS) -o $@ $< -lm
	@echo "Built: $@ (dynamically linked, with -lm)"

$(DYNAMIC_BINARY): $(C_DIR)/static_vs_dynamic.c
	$(CC) $(CFLAGS) -o $@ $< -lm
	@echo "Built: $@ (dynamically linked)"

$(STATIC_BINARY): $(C_DIR)/static_vs_dynamic.c
	$(CC) $(CFLAGS) -static -o $@ $< -lm
	@echo "Built: $@ (statically linked — much larger!)"

$(LIBMATH_OBJ): $(C_DIR)/libmath.c $(C_DIR)/libmath.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
	@echo "Built: $@ (Position Independent Code object)"

$(LIBMATH_SO): $(LIBMATH_OBJ)
	$(CC) -shared -o $@ $<
	@echo "Built: $@ (shared library)"

$(USE_LIBMATH): $(C_DIR)/use_libmath.c $(C_DIR)/libmath.h $(LIBMATH_SO)
	$(CC) $(CFLAGS) -o $@ $< -L$(C_DIR) -lmath -Wl,-rpath,$(C_DIR)
	@echo "Built: $@ (linked against libmath.so with rpath)"

$(RUST_ELF_SECTIONS): $(RUST_DIR)/elf_sections.rs
	$(RUSTC) -o $@ $<
	@echo "Built: $@"

$(JAVA_CLASS): $(JAVA_DIR)/ElfCompare.java
	$(JAVAC) -d $(JAVA_DIR) $<
	@echo "Built: $@"

# ==============================================================================
# Exploration targets
# ==============================================================================

# --- elf-dump: Compile elf_explorer and show full ELF analysis ---
elf-dump: $(ELF_EXPLORER)
	@echo ""
	@echo "================================================================"
	@echo "  ELF DUMP: Full readelf analysis of elf_explorer"
	@echo "================================================================"
	@echo ""
	@echo "--- Running the program first ---"
	@echo ""
	$(ELF_EXPLORER)
	@echo ""
	@echo "--- readelf -a (full ELF analysis) ---"
	@echo ""
	readelf -a $(ELF_EXPLORER)

# --- sections: Show ELF section headers ---
sections: $(ELF_EXPLORER)
	@echo ""
	@echo "================================================================"
	@echo "  ELF SECTIONS"
	@echo "================================================================"
	@echo ""
	@echo "Each section has a name, type, address, size, and flags."
	@echo "Key flags: A=Allocatable, W=Writable, X=eXecutable"
	@echo ""
	@echo "--- Section Headers (readelf -S) ---"
	@echo ""
	readelf -S $(ELF_EXPLORER)
	@echo ""
	@echo "--- Quick size summary (size command) ---"
	@echo ""
	@echo "  text = executable code (.text)"
	@echo "  data = initialized data (.data + .rodata)"
	@echo "  bss  = zero-initialized data (.bss) — 0 bytes on disk!"
	@echo ""
	size $(ELF_EXPLORER)

# --- symbols: Show symbol table with local vs global distinction ---
symbols: $(ELF_EXPLORER)
	@echo ""
	@echo "================================================================"
	@echo "  SYMBOL TABLE ANALYSIS"
	@echo "================================================================"
	@echo ""
	@echo "nm output key:"
	@echo "  T/t = .text (code)      — uppercase=GLOBAL, lowercase=local"
	@echo "  D/d = .data (init data) — uppercase=GLOBAL, lowercase=local"
	@echo "  B/b = .bss (zero data)  — uppercase=GLOBAL, lowercase=local"
	@echo "  R/r = .rodata (const)   — uppercase=GLOBAL, lowercase=local"
	@echo "  U   = undefined (needs to be resolved from shared library)"
	@echo ""
	@echo "--- All symbols sorted by address ---"
	@echo ""
	nm $(ELF_EXPLORER) | sort
	@echo ""
	@echo "--- Our functions and variables specifically ---"
	@echo ""
	@echo "Functions (.text section):"
	nm $(ELF_EXPLORER) | grep -E 'compute_sum|helper_function|counting_function|main' || true
	@echo ""
	@echo "Note: compute_sum is 'T' (GLOBAL) but helper_function is 't' (local/static)"
	@echo ""
	@echo "Variables:"
	nm $(ELF_EXPLORER) | grep -E 'global_counter|lookup_table|uninitialized|greeting|magic_number' || true
	@echo ""
	@echo "--- readelf symbol table (more detailed) ---"
	@echo ""
	readelf -s $(ELF_EXPLORER) | grep -E 'compute_sum|helper_function|counting_function|global_counter|uninitialized|greeting|magic_number' || true

# --- disasm: Disassemble the .text section ---
disasm: $(ELF_EXPLORER)
	@echo ""
	@echo "================================================================"
	@echo "  DISASSEMBLY of .text section"
	@echo "================================================================"
	@echo ""
	@echo "This shows the actual x86-64 machine code in human-readable form."
	@echo "Each line: address, hex bytes, assembly instruction"
	@echo ""
	objdump -d $(ELF_EXPLORER)

# --- dynamic: Explore dynamic linking (PLT, GOT) ---
dynamic: $(STATIC_VS_DYN)
	@echo ""
	@echo "================================================================"
	@echo "  DYNAMIC LINKING: PLT, GOT, and Shared Libraries"
	@echo "================================================================"
	@echo ""
	@echo "--- Shared library dependencies (ldd) ---"
	@echo ""
	ldd $(STATIC_VS_DYN)
	@echo ""
	@echo "--- Required shared libraries (NEEDED entries) ---"
	@echo ""
	readelf -d $(STATIC_VS_DYN) | grep NEEDED || true
	@echo ""
	@echo "--- Dynamic linker / interpreter ---"
	@echo ""
	readelf -l $(STATIC_VS_DYN) | grep -A1 interpreter || true
	@echo ""
	@echo "--- Undefined symbols (need runtime resolution) ---"
	@echo ""
	nm -D $(STATIC_VS_DYN) | grep ' U ' || true
	@echo ""
	@echo "--- PLT section (Procedure Linkage Table trampolines) ---"
	@echo ""
	objdump -d -j .plt $(STATIC_VS_DYN) 2>/dev/null || objdump -d -j .plt.sec $(STATIC_VS_DYN) 2>/dev/null || echo "(PLT section not found with expected name)"
	@echo ""
	@echo "--- Calls through PLT ---"
	@echo ""
	objdump -d $(STATIC_VS_DYN) | grep '@plt' || true
	@echo ""
	@echo "--- Dynamic relocations (GOT entries to be patched) ---"
	@echo ""
	objdump -R $(STATIC_VS_DYN) || true

# --- shared-lib: Build and demonstrate shared library usage ---
shared-lib: $(LIBMATH_SO) $(USE_LIBMATH)
	@echo ""
	@echo "================================================================"
	@echo "  SHARED LIBRARY: Building and Using libmath.so"
	@echo "================================================================"
	@echo ""
	@echo "--- Library file type ---"
	@echo ""
	file $(LIBMATH_SO)
	@echo ""
	@echo "--- Library ELF header (note Type: DYN) ---"
	@echo ""
	readelf -h $(LIBMATH_SO) | grep -E 'Type:|Machine:|Entry' || true
	@echo ""
	@echo "--- Exported symbols (dynamic symbol table) ---"
	@echo ""
	nm -D $(LIBMATH_SO)
	@echo ""
	@echo "--- All symbols (includes internal/static) ---"
	@echo ""
	nm $(LIBMATH_SO) || true
	@echo ""
	@echo "--- Binary dependencies ---"
	@echo ""
	ldd $(USE_LIBMATH)
	@echo ""
	@echo "--- Running the program (with rpath to find libmath.so) ---"
	@echo ""
	$(USE_LIBMATH)

# --- static-link: Compare statically vs dynamically linked binaries ---
static-link: $(DYNAMIC_BINARY) $(STATIC_BINARY)
	@echo ""
	@echo "================================================================"
	@echo "  STATIC vs DYNAMIC LINKING COMPARISON"
	@echo "================================================================"
	@echo ""
	@echo "--- File sizes ---"
	@echo ""
	ls -lh $(DYNAMIC_BINARY) $(STATIC_BINARY)
	@echo ""
	@echo "--- File types ---"
	@echo ""
	file $(DYNAMIC_BINARY)
	file $(STATIC_BINARY)
	@echo ""
	@echo "--- Dynamic binary dependencies ---"
	@echo ""
	ldd $(DYNAMIC_BINARY)
	@echo ""
	@echo "--- Static binary dependencies ---"
	@echo ""
	ldd $(STATIC_BINARY) 2>&1 || true
	@echo ""
	@echo "--- Symbol count comparison ---"
	@echo ""
	@printf "  Dynamic binary symbols: "
	@nm $(DYNAMIC_BINARY) 2>/dev/null | wc -l
	@printf "  Static binary symbols:  "
	@nm $(STATIC_BINARY) 2>/dev/null | wc -l
	@echo ""
	@echo "--- Section size comparison (size command) ---"
	@echo ""
	size $(DYNAMIC_BINARY) $(STATIC_BINARY)
	@echo ""
	@echo "The static binary is MUCH larger because it includes ALL of libc and libm"
	@echo "code directly in the binary, instead of loading them at runtime."

# --- rust-elf: Build Rust binary and compare ELF structure ---
rust-elf: $(RUST_ELF_SECTIONS) $(ELF_EXPLORER)
	@echo ""
	@echo "================================================================"
	@echo "  RUST ELF vs C ELF COMPARISON"
	@echo "================================================================"
	@echo ""
	@echo "--- File sizes ---"
	@echo ""
	ls -lh $(RUST_ELF_SECTIONS) $(ELF_EXPLORER)
	@echo ""
	@echo "--- File types ---"
	@echo ""
	file $(RUST_ELF_SECTIONS)
	file $(ELF_EXPLORER)
	@echo ""
	@echo "--- Section size comparison ---"
	@echo ""
	size $(RUST_ELF_SECTIONS) $(ELF_EXPLORER)
	@echo ""
	@echo "--- Symbol count comparison ---"
	@echo ""
	@printf "  Rust binary symbols:  "
	@nm $(RUST_ELF_SECTIONS) 2>/dev/null | wc -l
	@printf "  C binary symbols:     "
	@nm $(ELF_EXPLORER) 2>/dev/null | wc -l
	@echo ""
	@echo "--- Rust binary sections (same section names as C!) ---"
	@echo ""
	readelf -S $(RUST_ELF_SECTIONS) | head -40
	@echo ""
	@echo "--- Running the Rust program ---"
	@echo ""
	$(RUST_ELF_SECTIONS)

# --- java-compare: Show that .class is NOT ELF ---
java-compare: $(JAVA_CLASS)
	@echo ""
	@echo "================================================================"
	@echo "  JAVA .class vs ELF COMPARISON"
	@echo "================================================================"
	@echo ""
	@echo "--- .class file type ---"
	@echo ""
	file $(JAVA_CLASS)
	@echo ""
	@echo "--- JVM binary file type ---"
	@echo ""
	file $$(which java 2>/dev/null || echo "/usr/bin/java")
	@echo ""
	@echo "--- Magic bytes comparison ---"
	@echo ""
	@echo ".class magic (should be CA FE BA BE):"
	hexdump -C -n 16 $(JAVA_CLASS)
	@echo ""
	@echo "JVM binary magic (should be 7F 45 4C 46 = .ELF):"
	hexdump -C -n 16 $$(which java 2>/dev/null || echo "/usr/bin/java")
	@echo ""
	@echo "--- Bytecode disassembly (javap -c) ---"
	@echo ""
	cd $(JAVA_DIR) && javap -c ElfCompare
	@echo ""
	@echo "--- readelf on .class file (should FAIL — not ELF!) ---"
	@echo ""
	readelf -h $(JAVA_CLASS) 2>&1 || true
	@echo ""
	@echo "--- Running the Java program ---"
	@echo ""
	cd $(JAVA_DIR) && java ElfCompare

# --- hexdump: Show raw hex of ELF header ---
hexdump: $(ELF_EXPLORER)
	@echo ""
	@echo "================================================================"
	@echo "  RAW ELF HEADER (first 64 bytes)"
	@echo "================================================================"
	@echo ""
	@echo "The ELF header is always at the very beginning of the file."
	@echo "It's exactly 64 bytes for 64-bit ELF (52 bytes for 32-bit)."
	@echo ""
	@echo "Key fields in the first 16 bytes (ELF identification):"
	@echo "  Bytes 0-3:   7F 45 4C 46 = magic number (.ELF)"
	@echo "  Byte 4:      02 = 64-bit (01 = 32-bit)"
	@echo "  Byte 5:      01 = little-endian (02 = big-endian)"
	@echo "  Byte 6:      01 = ELF version 1 (current)"
	@echo "  Byte 7:      00 = OS/ABI (00 = System V / Linux)"
	@echo "  Bytes 8-15:  padding (zeros)"
	@echo ""
	@echo "--- Raw hex dump ---"
	@echo ""
	hexdump -C -n 64 $(ELF_EXPLORER)
	@echo ""
	@echo "--- Parsed ELF header (readelf -h) ---"
	@echo ""
	readelf -h $(ELF_EXPLORER)

# ==============================================================================
# Clean
# ==============================================================================

clean:
	@echo "Cleaning Module 3 build artifacts..."
	rm -f $(C_DIR)/elf_explorer
	rm -f $(C_DIR)/static_vs_dynamic
	rm -f $(C_DIR)/dynamic_binary
	rm -f $(C_DIR)/static_binary
	rm -f $(C_DIR)/libmath.o
	rm -f $(C_DIR)/libmath.so
	rm -f $(C_DIR)/use_libmath
	rm -f $(C_DIR)/*.s
	rm -f $(RUST_DIR)/elf_sections
	rm -f $(JAVA_DIR)/*.class
	@echo "Clean complete."
