/*
 * stripped_vs_unstripped.c — Compare debugging with and without symbols
 * =====================================================================
 *
 * Module 6: Debuggers at the Binary Level
 *
 * WHAT THIS DEMONSTRATES:
 *   Debug symbols (DWARF format) are what make GDB actually useful.
 *   Without them, GDB can't show you:
 *     - Variable names        → you only see registers and memory addresses
 *     - Function names        → you only see hex addresses
 *     - Source code lines     → you only see disassembly
 *     - Struct definitions    → you can't ptype or print fields
 *
 *   This file is compiled TWICE:
 *     1. With -g:     Full debug symbols (DWARF info embedded in the binary)
 *     2. Stripped:     All symbol information removed (strip command)
 *
 * HOW TO BUILD:
 *   # Version with debug symbols
 *   gcc -g -O0 -o with_symbols stripped_vs_unstripped.c
 *
 *   # Version without symbols (stripped)
 *   gcc -g -O0 -o stripped stripped_vs_unstripped.c
 *   strip stripped
 *
 * HOW TO COMPARE:
 *   # Check file sizes — stripped is much smaller
 *   ls -la with_symbols stripped
 *
 *   # Check what debug info is present
 *   readelf --debug-dump=info with_symbols | head -30
 *   readelf --debug-dump=info stripped | head -30    # empty!
 *
 *   # Check the symbol table
 *   nm with_symbols       # shows all function and variable names
 *   nm stripped            # "no symbols"
 *
 * GDB COMPARISON:
 *
 *   === With symbols (with_symbols) ===
 *   (gdb) break main                        ← works by name
 *   (gdb) break compute_value               ← works by name
 *   (gdb) run
 *   (gdb) list                              ← shows source code!
 *   (gdb) print point.x                     ← shows named fields!
 *   (gdb) info locals                       ← shows all local vars!
 *   (gdb) backtrace                         ← shows function names!
 *   (gdb) ptype struct Point                ← shows struct definition!
 *
 *   === Without symbols (stripped) ===
 *   (gdb) break main                        ← FAILS! "Function not found"
 *   (gdb) info functions                    ← no function names
 *   (gdb) break *0x401150                   ← must use addresses
 *   (gdb) run
 *   (gdb) list                              ← "No symbol table"
 *   (gdb) disassemble                       ← only see assembly
 *   (gdb) info registers                    ← only see register values
 *   (gdb) x/10gx $rsp                      ← must examine raw memory
 *
 * WHAT IS DWARF?
 *   DWARF is the debug information format used on Linux (and macOS).
 *   It's stored in special ELF sections:
 *     .debug_info      — variable types, scopes, locations
 *     .debug_abbrev    — abbreviation tables
 *     .debug_line      — source line → address mapping
 *     .debug_str       — string table for debug info
 *     .debug_loc       — variable location descriptions
 *     .debug_ranges    — address ranges for scopes
 *
 *   When you compile with -g, the compiler embeds all this information.
 *   When you strip, these sections are removed.
 *
 *   DWARF is what allows GDB to map between:
 *     - Source line numbers ↔ instruction addresses
 *     - Variable names ↔ register/memory locations
 *     - Type names ↔ memory layouts
 */

#include <stdio.h>
#include <math.h>

/* A struct — interesting to inspect with and without debug symbols */
struct Point {
    double x;
    double y;
    char   label[32];
};

/* An enum — GDB can show enum names with symbols, just numbers without */
enum Color {
    RED   = 0,
    GREEN = 1,
    BLUE  = 2
};

/*
 * compute_value() — A function with local variables to inspect
 *
 * WITH symbols:
 *   (gdb) break compute_value
 *   (gdb) print p->x        ← "10.5"
 *   (gdb) print distance    ← "11.8033..."
 *   (gdb) info locals        ← shows all variables by name
 *
 * WITHOUT symbols:
 *   (gdb) disassemble        ← raw instructions
 *   (gdb) info registers     ← rdi probably has the pointer
 *   (gdb) x/gf $rdi          ← manually examine the double at *rdi
 *   (gdb) x/s $rdi+16        ← skip 16 bytes to get to the label field
 */
static double compute_value(struct Point *p, enum Color c)
{
    double distance = sqrt(p->x * p->x + p->y * p->y);
    double color_bonus = (double)c * 1.5;

    printf("  Point '%s': distance=%.4f, color_bonus=%.1f\n",
           p->label, distance, color_bonus);

    /* Some computation with local variables to inspect */
    double result = distance + color_bonus;
    double squared = result * result;
    double final_value = squared / 10.0;

    return final_value;
}

/*
 * process_array() — A function with a loop and array access
 *
 * WITH symbols:
 *   (gdb) break process_array
 *   (gdb) print points[0]           ← full struct with field names
 *   (gdb) print points[0].label     ← "Origin"
 *   (gdb) display i                 ← auto-print loop counter
 *
 * WITHOUT symbols:
 *   You'd need to:
 *   1. Find where the array is in memory (probably on the stack)
 *   2. Calculate offsets manually (sizeof(struct Point) = ??)
 *   3. Use x/ commands to dump raw memory
 */
static double process_array(struct Point points[], int count)
{
    double total = 0.0;
    enum Color colors[] = { RED, GREEN, BLUE };

    for (int i = 0; i < count; i++) {
        total += compute_value(&points[i], colors[i % 3]);
    }

    return total;
}

int main(void)
{
    printf("=== Stripped vs Unstripped Debug Demo ===\n\n");

    /* Create some data to inspect */
    struct Point points[] = {
        { .x =  0.0, .y =  0.0, .label = "Origin"    },
        { .x = 10.5, .y =  5.0, .label = "Northeast" },
        { .x = -3.0, .y =  7.7, .label = "Northwest" },
        { .x =  8.0, .y = -2.5, .label = "Southeast" },
    };
    int count = sizeof(points) / sizeof(points[0]);

    printf("Processing %d points:\n", count);

    double result = process_array(points, count);

    printf("\nTotal result: %.4f\n", result);

    /* Some more variables for GDB practice */
    int loop_counter = 0;
    double accumulator = 0.0;

    for (loop_counter = 0; loop_counter < 5; loop_counter++) {
        accumulator += (double)loop_counter * result / 10.0;
    }

    printf("Final accumulator: %.4f\n", accumulator);

    printf("\n=== KEY TAKEAWAY ===\n");
    printf("With debug symbols (-g), GDB shows you the high-level picture:\n");
    printf("  variable names, types, source lines, function names.\n\n");
    printf("Without symbols (stripped), you're back to the raw binary level:\n");
    printf("  registers, memory addresses, and assembly instructions.\n\n");
    printf("This is what reverse engineers deal with — no source, no symbols.\n");
    printf("Module 10 (Reverse Engineering) will teach you how to work\n");
    printf("at the binary level when you don't have debug information.\n");

    return 0;
}
