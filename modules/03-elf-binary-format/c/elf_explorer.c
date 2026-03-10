/*
 * elf_explorer.c — A C program designed to produce interesting ELF sections
 * ============================================================================
 *
 * MODULE 3: ELF Binary Format
 * EXERCISE: Explore how C declarations map to ELF sections
 *
 * CONCEPT:
 *   When you compile a C program, different kinds of data and code end up in
 *   different "sections" of the resulting ELF binary. Understanding this
 *   mapping is key to reading ELF files.
 *
 * BUILD & EXPLORE:
 *   gcc -Wall -Wextra -o elf_explorer elf_explorer.c
 *   readelf -S elf_explorer          # Show all sections
 *   readelf -s elf_explorer          # Show symbol table
 *   nm elf_explorer                  # Show symbols (nm format)
 *   objdump -d elf_explorer          # Disassemble .text
 *   objdump -s -j .rodata elf_explorer  # Dump .rodata contents
 *   objdump -s -j .data elf_explorer    # Dump .data contents
 *
 * WHAT TO OBSERVE:
 *   After building, run the commands above and match each declaration below
 *   to the section it lands in. The ELF format organizes your program into:
 *
 *   .text   — Executable machine code (functions)
 *   .data   — Initialized global/static variables (read-write)
 *   .bss    — Uninitialized global/static variables (zero-initialized, no disk space)
 *   .rodata — Read-only data (string literals, const globals)
 *   .symtab — Symbol table (names of functions and variables)
 *   .strtab — String table (the actual name strings for .symtab)
 *
 * KEY INSIGHT:
 *   .bss takes NO space in the file on disk — it only records *how much*
 *   zero-initialized memory to allocate at runtime. Compare the file size
 *   of the binary to the virtual size of .bss with:
 *     readelf -S elf_explorer | grep bss
 *   The "Size" column shows runtime size, but it contributes 0 bytes to the file.
 */

#include <stdio.h>
#include <string.h>

/* ========================================================================
 * .data SECTION — Initialized global variables
 * ========================================================================
 * These have explicit initial values, so the compiler must store those
 * values somewhere in the binary. That's the .data section.
 *
 * OBSERVE: Run `nm elf_explorer | grep global_counter`
 *   You'll see something like: 0000000000404030 D global_counter
 *   The 'D' means it's in the initialized data section.
 *
 * OBSERVE: Run `objdump -s -j .data elf_explorer`
 *   You'll see the hex value 0x2a (decimal 42) stored in the binary.
 */
int global_counter = 42;

/*
 * This initialized array also goes to .data.
 * OBSERVE: `nm elf_explorer | grep lookup_table` shows 'D' (data section).
 * The entire array {10, 20, 30, 40, 50} is stored in the binary file.
 */
int lookup_table[5] = {10, 20, 30, 40, 50};

/* ========================================================================
 * .bss SECTION — Uninitialized (zero-initialized) global variables
 * ========================================================================
 * These have no explicit initializer, so C guarantees they start at zero.
 * The compiler doesn't need to store zeros — it just records "allocate
 * this many bytes of zero memory at runtime." This saves file space.
 *
 * OBSERVE: Run `nm elf_explorer | grep uninitialized_buffer`
 *   You'll see something like: 0000000000404060 B uninitialized_buffer
 *   The 'B' means it's in the BSS section.
 *
 * OBSERVE: Compare the .data and .bss sizes:
 *   readelf -S elf_explorer | grep -E '\.data|\.bss'
 *   .bss might show a large Size but contributes 0 bytes to the file.
 *
 * FUN FACT: BSS originally stood for "Block Started by Symbol" from old
 * assembler syntax. Some people remember it as "Better Save Space."
 */
int uninitialized_counter;
char uninitialized_buffer[4096];   /* 4KB of zeros — takes 0 bytes in the file! */

/* ========================================================================
 * .rodata SECTION — Read-only data (constants and string literals)
 * ========================================================================
 * The `const` qualifier tells the compiler this data never changes.
 * The compiler places it in .rodata, which the OS maps as read-only memory.
 * Attempting to write to .rodata at runtime causes a segfault (SIGSEGV).
 *
 * OBSERVE: Run `objdump -s -j .rodata elf_explorer`
 *   You'll see the string "Hello from .rodata!" in the hex dump.
 *   You'll also see all the printf format strings from this program.
 *
 * OBSERVE: Run `nm elf_explorer | grep greeting`
 *   You'll see: ... R greeting
 *   The 'R' means it's in a read-only section.
 */
const char greeting[] = "Hello from .rodata!";
const int magic_number = 0xDEADBEEF;

/*
 * String literals (the "..." in printf calls) also go to .rodata,
 * even though they don't have a named variable. The compiler assigns
 * them anonymous labels in .rodata. You can see them with:
 *   objdump -s -j .rodata elf_explorer
 */

/* ========================================================================
 * .text SECTION — Executable code (functions)
 * ========================================================================
 * All compiled function code goes into the .text section. This section
 * has the executable permission bit set in its ELF flags.
 *
 * OBSERVE: Run `readelf -S elf_explorer | grep .text`
 *   The Flags column will show 'AX' — Allocatable and eXecutable.
 *   Compare to .data which shows 'WA' — Writable and Allocatable.
 *   Compare to .rodata which shows 'A' — Allocatable only (not writable).
 *
 * OBSERVE: Run `objdump -d elf_explorer`
 *   You'll see the disassembly of every function, all from .text.
 */

/*
 * A regular (global/external) function — visible to other translation units.
 *
 * OBSERVE: Run `nm elf_explorer | grep compute_sum`
 *   You'll see: ... T compute_sum
 *   The 'T' means it's a global symbol in the .text section.
 *   Uppercase 'T' = global (visible to the linker from other files).
 */
int compute_sum(int a, int b) {
    return a + b;
}

/*
 * A static function — LOCAL to this translation unit only.
 *
 * OBSERVE: Run `nm elf_explorer | grep helper_function`
 *   You'll see: ... t helper_function
 *   The lowercase 't' means it's a LOCAL symbol in .text.
 *   Lowercase = local (invisible to the linker from other files).
 *
 * KEY INSIGHT: Both compute_sum and helper_function are in .text,
 *   but they differ in VISIBILITY. This is a fundamental ELF concept:
 *   - STB_GLOBAL (uppercase in nm) — visible across object files
 *   - STB_LOCAL  (lowercase in nm) — private to this object file
 *
 * OBSERVE: Run `readelf -s elf_explorer | grep -E 'compute_sum|helper_function'`
 *   You'll see the Bind column shows GLOBAL vs LOCAL.
 */
static int helper_function(int x) {
    return x * x + 1;
}

/*
 * Static local variables inside functions go to .data (if initialized)
 * or .bss (if uninitialized), NOT on the stack. They persist across calls.
 *
 * OBSERVE: Run `nm elf_explorer | grep call_count`
 *   You'll see it in the data/bss section, despite being "inside" a function.
 *   The compiler may mangle the name (e.g., call_count.1234).
 */
int counting_function(void) {
    static int call_count = 0;  /* .data — initialized, persists across calls */
    call_count++;
    return call_count;
}

/* ========================================================================
 * main — Ties everything together and prints exploration instructions
 * ======================================================================== */
int main(void) {
    printf("=== ELF Section Explorer ===\n\n");

    /* Use all our variables so the compiler doesn't optimize them away */

    /* .rodata: const string */
    printf("[.rodata] greeting = \"%s\"\n", greeting);
    printf("[.rodata] magic_number = 0x%X\n", magic_number);

    /* .data: initialized globals */
    printf("[.data]   global_counter = %d\n", global_counter);
    printf("[.data]   lookup_table[2] = %d\n", lookup_table[2]);

    /* .bss: uninitialized globals (guaranteed zero) */
    printf("[.bss]    uninitialized_counter = %d (always 0)\n", uninitialized_counter);
    printf("[.bss]    uninitialized_buffer[0] = %d (always 0)\n", uninitialized_buffer[0]);
    printf("[.bss]    sizeof(uninitialized_buffer) = %zu bytes (0 bytes on disk!)\n",
           sizeof(uninitialized_buffer));

    /* .text: functions */
    printf("[.text]   compute_sum(3, 4) = %d\n", compute_sum(3, 4));
    printf("[.text]   helper_function(5) = %d (static — local symbol)\n", helper_function(5));
    printf("[.text]   counting_function() = %d\n", counting_function());
    printf("[.text]   counting_function() = %d (static local persists!)\n", counting_function());

    printf("\n=== Exploration Commands ===\n");
    printf("Try these commands on the compiled binary:\n\n");
    printf("  readelf -h elf_explorer       # ELF header (magic, arch, entry point)\n");
    printf("  readelf -S elf_explorer       # Section headers (.text, .data, .bss, etc.)\n");
    printf("  readelf -s elf_explorer       # Symbol table (functions + variables)\n");
    printf("  readelf -l elf_explorer       # Program headers (segments for loading)\n");
    printf("  nm elf_explorer               # Symbols: T=text, D=data, B=bss, R=rodata\n");
    printf("  nm elf_explorer | sort        # Sorted by address — see memory layout\n");
    printf("  objdump -d elf_explorer       # Disassemble .text section\n");
    printf("  objdump -s -j .rodata elf_explorer  # Raw .rodata contents\n");
    printf("  objdump -s -j .data elf_explorer    # Raw .data contents\n");
    printf("  hexdump -C elf_explorer | head -4   # ELF magic: 7f 45 4c 46\n");
    printf("  size elf_explorer             # Quick summary: text, data, bss sizes\n");

    printf("\n=== Key Things to Notice ===\n");
    printf("1. 'nm' output: uppercase = GLOBAL, lowercase = local\n");
    printf("2. .bss has a runtime size but takes 0 bytes in the file\n");
    printf("3. .rodata is mapped read-only — writing to it causes SIGSEGV\n");
    printf("4. String literals from printf are in .rodata too\n");
    printf("5. Static functions show 't' (local) vs 'T' (global) in nm\n");

    return 0;
}
