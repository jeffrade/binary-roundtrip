/*
 * Module 2: C Compilation Pipeline — All Stages Demo
 * ===================================================
 *
 * This single file is specifically designed to produce interesting output
 * at EVERY stage of the compilation pipeline. Each feature was chosen
 * because it reveals something important about that particular stage.
 *
 * ======================================================================
 * STAGE 1 — PREPROCESSING (gcc -E stages_demo.c > stages_demo.i)
 * ======================================================================
 * What to look for:
 *   - #include <stdio.h> expands to ~700 lines of declarations
 *   - #include <stdlib.h> adds more declarations
 *   - #define macros are replaced with their values (MAX_SIZE → 100)
 *   - SQUARE(x) calls are expanded inline
 *   - #ifdef DEBUG blocks are included or excluded
 *   - __FILE__, __LINE__ are replaced with actual values
 *   - The output is pure C — no # directives remain (except #line markers)
 *
 * ======================================================================
 * STAGE 2 — COMPILATION (gcc -S stages_demo.c → stages_demo.s)
 * ======================================================================
 * What to look for in the assembly:
 *   - .rodata section: string literals ("Hello...", format strings)
 *   - .data section: initialized global variable (global_counter = 42)
 *   - .bss section: uninitialized global variable (global_buffer)
 *   - Function labels: main:, compute_sum:, print_info:
 *   - Stack frame setup: push rbp; mov rbp, rsp; sub rsp, N
 *   - Function calls: call printf, call compute_sum
 *   - The struct access patterns (loading struct members via offsets)
 *
 * ======================================================================
 * STAGE 3 — ASSEMBLY (gcc -c stages_demo.c → stages_demo.o)
 * ======================================================================
 * What to look for:
 *   - file stages_demo.o → "ELF 64-bit LSB relocatable"
 *     ("relocatable" = not yet linked, addresses not final)
 *   - nm stages_demo.o:
 *     T main, T compute_sum, T print_info (defined functions)
 *     U printf, U puts (undefined — need libc at link time)
 *     D global_counter (Data section — initialized)
 *     B global_buffer (BSS section — uninitialized)
 *   - objdump -h stages_demo.o:
 *     Shows .text, .data, .bss, .rodata sections and their sizes
 *   - readelf -S stages_demo.o:
 *     Shows all ELF sections with addresses (all 0x0 — not yet linked)
 *
 * ======================================================================
 * STAGE 4 — LINKING (gcc -o stages_demo stages_demo.o)
 * ======================================================================
 * What to look for:
 *   - file stages_demo → "ELF 64-bit LSB executable" (or "pie executable")
 *     ("executable" not "relocatable" — it's fully linked now)
 *   - nm stages_demo: all symbols now have real addresses
 *   - ldd stages_demo: shows libc.so (where printf lives)
 *   - readelf -l stages_demo: shows LOAD segments with real addresses
 *   - strings stages_demo: shows all string literals embedded in the binary
 */

/* ======================================================================
 * INCLUDES — Interesting in preprocessing (each expands to hundreds of lines)
 * ====================================================================== */
#include <stdio.h>    /* printf, puts — expands to ~700 lines */
#include <stdlib.h>   /* atoi, EXIT_SUCCESS — expands to ~400 more lines */
#include <string.h>   /* strlen, strcpy — even more declarations */

/* ======================================================================
 * MACROS — Interesting in preprocessing (text substitution)
 * ====================================================================== */
#define MAX_SIZE      100
#define SQUARE(x)     ((x) * (x))
#define ARRAY_LEN(a)  (sizeof(a) / sizeof((a)[0]))

/* Conditional compilation: compile with -DDEBUG to enable */
#ifdef DEBUG
    #define LOG(msg) printf("[DEBUG %s:%d] %s\n", __FILE__, __LINE__, msg)
#else
    #define LOG(msg) ((void)0)
#endif

/* ======================================================================
 * GLOBAL VARIABLES — Interesting in assembly (different ELF sections)
 * ======================================================================
 *
 * global_counter: Initialized to 42.
 *   → Goes into .data section (read-write, initialized data).
 *   → In the object file, you'll see: nm shows it as "D" (Data).
 *   → The value 42 is stored in the binary itself.
 *
 * global_buffer: Declared but not initialized.
 *   → Goes into .bss section (Block Started by Symbol).
 *   → BSS takes NO space in the binary — the OS fills it with zeros.
 *   → nm shows it as "B" (BSS).
 *   → This is why large uninitialized arrays don't make your binary huge.
 */
int global_counter = 42;                  /* .data — initialized global */
char global_buffer[MAX_SIZE];             /* .bss  — uninitialized global */

/* ======================================================================
 * STRUCT — Interesting in assembly (member access via memory offsets)
 * ====================================================================== */
struct Point {
    int x;     /* offset 0 from struct base */
    int y;     /* offset 4 from struct base (int is 4 bytes) */
    char label[32]; /* offset 8 from struct base */
};

/* ======================================================================
 * FUNCTION: compute_sum — Interesting in assembly (loop, stack variables)
 * ======================================================================
 *
 * In assembly, look for:
 *   - Stack allocation for local variables (n, i, sum)
 *   - The loop structure: compare, conditional jump, increment
 *   - The return value is placed in the %eax register
 */
long compute_sum(int n)
{
    long sum = 0;
    for (int i = 1; i <= n; i++) {
        sum += SQUARE(i);  /* After preprocessing: sum += ((i) * (i)); */
    }
    LOG("compute_sum completed");
    return sum;
}

/* ======================================================================
 * FUNCTION: print_info — Interesting in assembly (struct access, strings)
 * ======================================================================
 *
 * In assembly, look for:
 *   - Struct member access: loading p->x uses [base + 0], p->y uses [base + 4]
 *   - String literals in .rodata section
 *   - printf call with format string
 */
void print_info(struct Point *p)
{
    printf("Point '%s' at (%d, %d)\n", p->label, p->x, p->y);
}

/* ======================================================================
 * STRING LITERALS — Interesting in the final binary (stored in .rodata)
 * ======================================================================
 *
 * Every string literal below is stored in the .rodata (read-only data)
 * section of the binary. You can find them with:
 *   strings stages_demo | grep "Stage"
 *
 * The strings are NOT in the .text section (code) — they're data.
 * The code references them by address.
 */

/* ======================================================================
 * MAIN FUNCTION — Ties everything together
 * ====================================================================== */
int main(int argc, char *argv[])
{
    /* Local variables — stored on the stack (not in .data or .bss) */
    int numbers[] = {10, 20, 30, 40, 50};
    struct Point origin;

    printf("=== C Compilation Pipeline: Stages Demo ===\n\n");

    /* --- Global variables (show .data vs .bss behavior) --- */
    printf("[.data section] global_counter = %d (initialized global)\n",
           global_counter);
    printf("[.bss  section] global_buffer[0] = %d (uninitialized, zero-filled by OS)\n",
           (int)global_buffer[0]);
    printf("\n");

    /* --- Macros (show preprocessing) --- */
    printf("[Preprocessing] MAX_SIZE = %d (was a #define macro)\n", MAX_SIZE);
    printf("[Preprocessing] SQUARE(7) = %d (was a function-like macro)\n", SQUARE(7));
    printf("[Preprocessing] ARRAY_LEN(numbers) = %zu (sizeof trick)\n",
           ARRAY_LEN(numbers));
    printf("[Preprocessing] Compiled on %s at %s\n", __DATE__, __TIME__);
    printf("\n");

    /* --- Function call (show the call/return in assembly) --- */
    int n = 10;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    long result = compute_sum(n);
    printf("[Function call] compute_sum(%d) = %ld (sum of squares)\n", n, result);
    printf("\n");

    /* --- Struct access (show memory layout in assembly) --- */
    origin.x = 0;
    origin.y = 0;
    strcpy(origin.label, "origin");
    printf("[Struct access] ");
    print_info(&origin);
    printf("  Struct size: %zu bytes (int + int + char[32])\n",
           sizeof(struct Point));
    printf("  Offset of x: %zu, y: %zu, label: %zu\n",
           __builtin_offsetof(struct Point, x),
           __builtin_offsetof(struct Point, y),
           __builtin_offsetof(struct Point, label));
    printf("\n");

    /* --- String literal in .rodata --- */
    const char *message = "This string lives in the .rodata section of the binary.";
    printf("[.rodata section] %s\n", message);
    printf("  Try: strings stages_demo | grep 'rodata'\n");
    printf("\n");

    /* --- Stack vs heap (local vs dynamic allocation) --- */
    printf("[Stack]  Local array 'numbers' is on the stack: %zu bytes\n",
           sizeof(numbers));
    int *heap_data = malloc(MAX_SIZE * sizeof(int));
    if (heap_data != NULL) {
        printf("[Heap]   Allocated %zu bytes on the heap via malloc()\n",
               MAX_SIZE * sizeof(int));
        free(heap_data);
    }
    printf("\n");

    /* --- Debug mode (conditional compilation) --- */
    LOG("This line only appears if compiled with -DDEBUG");
#ifdef DEBUG
    printf("[Conditional] DEBUG mode is ON\n");
#else
    printf("[Conditional] DEBUG mode is OFF (compile with -DDEBUG)\n");
#endif

    printf("\n");
    printf("Run 'make pipeline' to see this file at every compilation stage.\n");

    return EXIT_SUCCESS;
}
