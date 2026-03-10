/*
 * memory_layout.c — Explore a process's virtual address space
 * ============================================================
 *
 * Module 5: How Linux Runs a Binary
 *
 * WHAT THIS DEMONSTRATES:
 *   Every process has its own virtual address space. The kernel and the
 *   dynamic linker map different regions at different addresses:
 *
 *   HIGH ADDRESSES (e.g., 0x7fff...)
 *     ┌─────────────────────┐
 *     │       Stack         │  ← Local variables, grows DOWNWARD
 *     │         ↓           │
 *     │                     │
 *     │   (unmapped gap)    │
 *     │                     │
 *     │         ↑           │
 *     │       Heap          │  ← malloc(), grows UPWARD
 *     ├─────────────────────┤
 *     │       .bss          │  ← Uninitialized globals (zeroed)
 *     ├─────────────────────┤
 *     │       .data         │  ← Initialized globals
 *     ├─────────────────────┤
 *     │       .rodata       │  ← String literals, constants
 *     ├─────────────────────┤
 *     │       .text         │  ← Executable code (your functions)
 *     └─────────────────────┘
 *   LOW ADDRESSES (e.g., 0x0040...)
 *
 *   NOTE: With ASLR (Address Space Layout Randomization), the exact
 *   addresses change on each run, but the RELATIVE layout stays the same.
 *
 * WHAT TO OBSERVE:
 *   - .text (code) is at the lowest addresses
 *   - .rodata is near .text
 *   - .data and .bss are above .text
 *   - Heap (malloc) is above .bss
 *   - Stack is at the HIGHEST addresses, far above everything else
 *   - The /proc/self/maps output shows you EXACTLY what's mapped where
 *
 * TO BUILD AND RUN:
 *   gcc -Wall -Wextra -o memory_layout memory_layout.c
 *   ./memory_layout
 *
 * TRY ALSO:
 *   # Disable ASLR to see consistent addresses:
 *   setarch $(uname -m) -R ./memory_layout
 *
 *   # Compare PIE vs non-PIE:
 *   gcc -no-pie -o memory_layout_nopie memory_layout.c
 *   ./memory_layout_nopie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* -----------------------------------------------------------------------
 * .data segment: Initialized global variables
 *
 * These have a defined initial value and live in the .data section of
 * the ELF binary. The linker allocates space and the loader copies the
 * initial values from the file into memory.
 * ----------------------------------------------------------------------- */
int global_initialized = 42;

/* -----------------------------------------------------------------------
 * .bss segment: Uninitialized global variables
 *
 * These are zeroed by the kernel at load time. They DON'T take up space
 * in the ELF file (just a size record), but DO take space in memory.
 * "BSS" historically stands for "Block Started by Symbol."
 * ----------------------------------------------------------------------- */
int global_uninitialized;

/* -----------------------------------------------------------------------
 * .rodata: This string literal will go in the read-only data section
 * ----------------------------------------------------------------------- */
const char *readonly_string = "I am a read-only string literal in .rodata";

/*
 * A helper function — its address will be in the .text section
 */
void helper_function(void)
{
    /* This function exists just so we can print its address */
    printf("  (helper_function was called)\n");
}

/*
 * Struct to hold our memory region information for sorted display
 */
struct mem_region {
    const char *name;
    const char *section;
    void       *address;
};

/*
 * Comparison function for qsort — sort by address ascending
 */
static int compare_regions(const void *a, const void *b)
{
    const struct mem_region *ra = (const struct mem_region *)a;
    const struct mem_region *rb = (const struct mem_region *)b;

    if (ra->address < rb->address) return -1;
    if (ra->address > rb->address) return  1;
    return 0;
}

int main(void)
{
    printf("=== Process Memory Layout Explorer ===\n");
    printf("PID: %d\n\n", getpid());

    /* ---------------------------------------------------------------
     * Collect addresses from each memory region
     * --------------------------------------------------------------- */

    /* Stack variable — allocated on the stack, high addresses */
    int stack_variable = 99;

    /* Another stack variable — will be at a LOWER address than the first
     * because the stack grows DOWNWARD on x86_64 */
    int stack_variable2 = 100;

    /* Heap allocation — malloc returns memory from the heap */
    int *heap_ptr = malloc(sizeof(int));
    if (!heap_ptr) {
        perror("malloc");
        return 1;
    }
    *heap_ptr = 77;

    /* Second heap allocation — should be at a HIGHER address
     * because the heap grows UPWARD */
    int *heap_ptr2 = malloc(sizeof(int));
    if (!heap_ptr2) {
        perror("malloc");
        free(heap_ptr);
        return 1;
    }
    *heap_ptr2 = 88;

    /* ---------------------------------------------------------------
     * Print all addresses
     * --------------------------------------------------------------- */
    printf("Individual Addresses:\n");
    printf("─────────────────────────────────────────────────────────\n");
    printf("  .text   (main function):        %p\n", (void *)main);
    printf("  .text   (helper_function):      %p\n", (void *)helper_function);
    printf("  .rodata (string literal):       %p\n", (void *)readonly_string);
    printf("  .data   (global_initialized):   %p  (value=%d)\n",
           (void *)&global_initialized, global_initialized);
    printf("  .bss    (global_uninitialized): %p  (value=%d, zeroed by kernel)\n",
           (void *)&global_uninitialized, global_uninitialized);
    printf("  heap    (first malloc):         %p  (value=%d)\n",
           (void *)heap_ptr, *heap_ptr);
    printf("  heap    (second malloc):        %p  (value=%d)\n",
           (void *)heap_ptr2, *heap_ptr2);
    printf("  stack   (stack_variable):       %p  (value=%d)\n",
           (void *)&stack_variable, stack_variable);
    printf("  stack   (stack_variable2):      %p  (value=%d)\n",
           (void *)&stack_variable2, stack_variable2);
    printf("\n");

    /* ---------------------------------------------------------------
     * Sort by address to show the layout visually
     * --------------------------------------------------------------- */
    struct mem_region regions[] = {
        { "main()",              ".text",   (void *)main                  },
        { "helper_function()",   ".text",   (void *)helper_function       },
        { "string literal",      ".rodata", (void *)readonly_string       },
        { "global_initialized",  ".data",   (void *)&global_initialized   },
        { "global_uninitialized",".bss",    (void *)&global_uninitialized },
        { "heap alloc #1",       "heap",    (void *)heap_ptr              },
        { "heap alloc #2",       "heap",    (void *)heap_ptr2             },
        { "stack_variable",      "stack",   (void *)&stack_variable       },
        { "stack_variable2",     "stack",   (void *)&stack_variable2      },
    };
    size_t nregions = sizeof(regions) / sizeof(regions[0]);

    qsort(regions, nregions, sizeof(regions[0]), compare_regions);

    printf("Sorted by Address (low → high):\n");
    printf("─────────────────────────────────────────────────────────\n");
    for (size_t i = 0; i < nregions; i++) {
        printf("  %p  %-8s  %s\n",
               regions[i].address,
               regions[i].section,
               regions[i].name);

        /* Show gaps between regions to illustrate the address space layout */
        if (i + 1 < nregions) {
            unsigned long gap = (unsigned long)regions[i+1].address -
                                (unsigned long)regions[i].address;
            if (gap > 0x10000) {
                printf("     ... gap of 0x%lx bytes (%lu MB) ...\n",
                       gap, gap / (1024 * 1024));
            }
        }
    }
    printf("\n");

    /* ---------------------------------------------------------------
     * Show stack growth direction
     * --------------------------------------------------------------- */
    printf("Stack Growth Direction:\n");
    printf("─────────────────────────────────────────────────────────\n");
    if (&stack_variable > &stack_variable2) {
        printf("  stack_variable  is at HIGHER address (%p)\n", (void *)&stack_variable);
        printf("  stack_variable2 is at LOWER  address (%p)\n", (void *)&stack_variable2);
        printf("  → Stack grows DOWNWARD (toward lower addresses)\n");
        printf("  → This is standard for x86_64\n");
    } else {
        printf("  stack_variable  is at LOWER  address (%p)\n", (void *)&stack_variable);
        printf("  stack_variable2 is at HIGHER address (%p)\n", (void *)&stack_variable2);
        printf("  → Unexpected stack growth direction!\n");
    }
    printf("\n");

    /* ---------------------------------------------------------------
     * Show heap growth direction
     * --------------------------------------------------------------- */
    printf("Heap Growth Direction:\n");
    printf("─────────────────────────────────────────────────────────\n");
    if (heap_ptr2 > heap_ptr) {
        printf("  First  malloc at %p\n", (void *)heap_ptr);
        printf("  Second malloc at %p\n", (void *)heap_ptr2);
        printf("  → Heap grows UPWARD (toward higher addresses)\n");
    } else {
        printf("  First  malloc at %p\n", (void *)heap_ptr);
        printf("  Second malloc at %p\n", (void *)heap_ptr2);
        printf("  → Heap allocator chose a lower address (common with modern allocators)\n");
    }
    printf("\n");

    /* ---------------------------------------------------------------
     * Read and display /proc/self/maps
     *
     * /proc/self/maps is a kernel-provided pseudo-file that shows
     * every memory mapping in the current process. Each line contains:
     *   address_range  permissions  offset  device  inode  pathname
     *
     * Permissions:
     *   r = readable, w = writable, x = executable, p = private, s = shared
     *
     * Key mappings to look for:
     *   r-xp ... /path/to/binary    → .text (code, executable)
     *   r--p ... /path/to/binary    → .rodata (read-only data)
     *   rw-p ... /path/to/binary    → .data + .bss (read-write data)
     *   rw-p ... [heap]             → heap (malloc territory)
     *   rw-p ... [stack]            → stack
     *   r-xp ... /lib/x86_64-linux-gnu/libc.so.6  → libc's code
     *   r-xp ... /lib/x86_64-linux-gnu/ld-linux-x86-64.so.2  → dynamic linker
     * --------------------------------------------------------------- */
    printf("════════════════════════════════════════════════════════════\n");
    printf("/proc/self/maps — The kernel's view of our address space:\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    printf("Format: address_range perms offset dev inode pathname\n");
    printf("─────────────────────────────────────────────────────────\n");

    FILE *maps = fopen("/proc/self/maps", "r");
    if (maps) {
        char line[512];
        while (fgets(line, sizeof(line), maps)) {
            /* Annotate known regions */
            if (strstr(line, "[heap]")) {
                printf("  %s  ← HEAP: malloc() allocates from here\n", line);
            } else if (strstr(line, "[stack]")) {
                printf("  %s  ← STACK: local variables live here\n", line);
            } else if (strstr(line, "[vdso]")) {
                printf("  %s  ← VDSO: kernel-provided fast syscalls (gettimeofday, etc.)\n", line);
            } else if (strstr(line, "[vvar]")) {
                printf("  %s  ← VVAR: kernel variables for VDSO\n", line);
            } else if (strstr(line, "ld-linux") || strstr(line, "ld-musl")) {
                printf("  %s  ← DYNAMIC LINKER: resolves shared library symbols\n", line);
            } else if (strstr(line, "libc")) {
                /* Check for executable permission to distinguish .text from .data */
                if (strstr(line, "r-xp") || strstr(line, "r--p")) {
                    printf("  %s  ← LIBC: C library code/rodata\n", line);
                } else {
                    printf("  %s  ← LIBC: C library data\n", line);
                }
            } else if (strstr(line, "memory_layout")) {
                if (strstr(line, "r-xp")) {
                    printf("  %s  ← OUR .text: executable code\n", line);
                } else if (strstr(line, "r--p")) {
                    printf("  %s  ← OUR .rodata: read-only data\n", line);
                } else if (strstr(line, "rw-p")) {
                    printf("  %s  ← OUR .data/.bss: read-write data\n", line);
                } else {
                    printf("  %s", line);
                }
            } else {
                printf("  %s", line);
            }
        }
        fclose(maps);
    } else {
        perror("Could not open /proc/self/maps");
        printf("(This only works on Linux)\n");
    }

    printf("\n");
    printf("=== KEY TAKEAWAYS ===\n");
    printf("1. Each process has its own virtual address space (isolated from others).\n");
    printf("2. The .text section (code) is typically at LOW addresses and is r-x (read+execute).\n");
    printf("3. The .data/.bss sections are rw- (read+write, NOT executable — W^X protection).\n");
    printf("4. The heap starts after .bss and grows UPWARD.\n");
    printf("5. The stack starts at HIGH addresses and grows DOWNWARD.\n");
    printf("6. Shared libraries (libc, ld-linux) are mapped into your address space too.\n");
    printf("7. ASLR randomizes all these addresses on each run for security.\n");

    free(heap_ptr);
    free(heap_ptr2);

    return 0;
}
