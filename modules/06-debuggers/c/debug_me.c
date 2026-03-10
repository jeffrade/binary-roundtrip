/*
 * debug_me.c — A program designed for GDB practice
 * ==================================================
 *
 * Module 6: Debuggers at the Binary Level
 *
 * PURPOSE:
 *   This program has NO bugs. It's designed to give you a rich environment
 *   for practicing GDB commands. There are multiple functions, structs,
 *   arrays, pointers, and loops — all the things you need to learn
 *   how to navigate a program in a debugger.
 *
 * HOW TO USE WITH GDB:
 *   gcc -g -O0 -o debug_me debug_me.c
 *   gdb ./debug_me
 *
 *   Then follow the exercises in gdb_exercises.txt (Exercise 1).
 *
 * CALL GRAPH:
 *   main()
 *     ├─→ init_dataset()           — fills an array of structs
 *     ├─→ process_data()           — iterates over the dataset
 *     │     ├─→ compute()          — does arithmetic on each element
 *     │     │     └─→ helper()     — a utility function
 *     │     └─→ classify()         — categorizes results
 *     └─→ print_summary()          — prints final results
 *
 * GDB PRACTICE POINTS (marked with "GDB:" comments throughout):
 *   - Setting breakpoints at different functions
 *   - Stepping through loops
 *   - Inspecting struct fields
 *   - Examining arrays
 *   - Watching variables change
 *   - Navigating the call stack
 *   - Printing expressions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* -----------------------------------------------------------------------
 * Data structures
 *
 * GDB: Try these commands to inspect structs:
 *   (gdb) print dataset[0]          — shows all fields
 *   (gdb) print dataset[0].name     — shows one field
 *   (gdb) print *dataset@5          — shows 5 elements
 *   (gdb) ptype struct DataPoint    — shows the struct definition
 * ----------------------------------------------------------------------- */

#define MAX_NAME_LEN  32
#define DATASET_SIZE  8

/* Categories for classifying data points */
enum Category {
    CAT_LOW,
    CAT_MEDIUM,
    CAT_HIGH,
    CAT_EXTREME
};

/* Our main data structure */
struct DataPoint {
    char   name[MAX_NAME_LEN];
    int    id;
    double value;
    double computed_result;
    enum Category category;
};

/* Summary statistics */
struct Summary {
    int    count;
    double total;
    double average;
    double min_value;
    double max_value;
    int    category_counts[4];  /* one for each Category */
};

/* -----------------------------------------------------------------------
 * helper() — A simple utility function at the bottom of the call chain
 *
 * GDB: When you're stopped in helper(), try:
 *   (gdb) backtrace         — see the full call chain
 *   (gdb) frame 1           — jump to compute()'s frame
 *   (gdb) info args          — see compute()'s arguments
 *   (gdb) frame 0            — back to helper()
 *   (gdb) info locals        — see helper()'s local variables
 * ----------------------------------------------------------------------- */
static double helper(double x, double factor)
{
    /* GDB: Set a breakpoint here and inspect x and factor */
    double adjusted = x * factor;

    /* GDB: Step over this line, then print 'scaled' to see the result */
    double scaled = adjusted / 100.0;

    /* GDB: Try: print adjusted, print scaled, print adjusted + scaled */
    return scaled + adjusted;
}

/* -----------------------------------------------------------------------
 * compute() — Performs a calculation on a data point
 *
 * GDB: Try these when stopped here:
 *   (gdb) print *dp            — dereference the pointer to see the struct
 *   (gdb) print dp->name       — access a field through the pointer
 *   (gdb) print dp->value      — see the current value
 *   (gdb) step                 — step INTO helper()
 *   (gdb) finish               — run until helper() returns, show result
 * ----------------------------------------------------------------------- */
static double compute(struct DataPoint *dp, int iteration)
{
    /* GDB: Set a watchpoint: watch dp->computed_result
     * This will stop execution whenever computed_result changes */

    double base = dp->value * (1.0 + (double)iteration * 0.1);

    /* GDB: Step into helper() to see the deepest function call */
    double result = helper(base, 2.5);

    /* Apply a non-linear transformation so values are interesting to inspect */
    result = sqrt(fabs(result)) + (double)(dp->id * 3);

    /* GDB: After this line, print dp->computed_result to verify it was set */
    dp->computed_result = result;

    return result;
}

/* -----------------------------------------------------------------------
 * classify() — Categorizes a computed result
 *
 * GDB: Good place to practice conditional breakpoints:
 *   (gdb) break classify if result > 50.0
 *   This will only stop when result is above 50.
 * ----------------------------------------------------------------------- */
static enum Category classify(double result)
{
    /* GDB: Try: print result, then step through to see which branch is taken */
    if (result < 10.0) {
        return CAT_LOW;
    } else if (result < 30.0) {
        return CAT_MEDIUM;
    } else if (result < 50.0) {
        return CAT_HIGH;
    } else {
        return CAT_EXTREME;
    }
}

/* -----------------------------------------------------------------------
 * init_dataset() — Fills the dataset with sample data
 *
 * GDB: After this function returns, inspect the dataset:
 *   (gdb) print dataset[0]
 *   (gdb) print dataset[3].name
 *   (gdb) print dataset          — shows the entire array
 * ----------------------------------------------------------------------- */
static void init_dataset(struct DataPoint dataset[], int size)
{
    const char *names[] = {
        "Alpha", "Beta", "Gamma", "Delta",
        "Epsilon", "Zeta", "Eta", "Theta"
    };

    /* GDB: Set a breakpoint inside this loop, then:
     *   (gdb) print i         — see the loop counter
     *   (gdb) continue        — run to next iteration
     *   (gdb) print dataset[i] — see the element being initialized */
    for (int i = 0; i < size; i++) {
        strncpy(dataset[i].name, names[i], MAX_NAME_LEN - 1);
        dataset[i].name[MAX_NAME_LEN - 1] = '\0';
        dataset[i].id = i + 1;
        dataset[i].value = (double)(i * 7 + 3) * 1.5;
        dataset[i].computed_result = 0.0;
        dataset[i].category = CAT_LOW;
    }
}

/* -----------------------------------------------------------------------
 * process_data() — Main processing loop
 *
 * GDB: This is the best function for practicing loop debugging:
 *   (gdb) break process_data
 *   (gdb) run
 *   (gdb) next              — step over (stays in process_data)
 *   (gdb) step              — step into (enters compute or classify)
 *   (gdb) continue          — run to next breakpoint
 *   (gdb) display i         — auto-print i at every stop
 *   (gdb) display dataset[i].computed_result
 * ----------------------------------------------------------------------- */
static void process_data(struct DataPoint dataset[], int size)
{
    printf("Processing %d data points...\n\n", size);

    /* GDB: Try setting a watchpoint on i:
     *   (gdb) watch i
     *   Now 'continue' will stop every time i changes value */
    for (int i = 0; i < size; i++) {
        printf("  [%d] Processing '%s' (value=%.2f)...\n",
               i, dataset[i].name, dataset[i].value);

        /* GDB: Step INTO compute() to trace the full call chain:
         *   compute → helper → back to compute → back here */
        double result = compute(&dataset[i], i);

        /* GDB: Step INTO classify() to see the categorization logic */
        dataset[i].category = classify(result);

        const char *cat_names[] = { "LOW", "MEDIUM", "HIGH", "EXTREME" };
        printf("       → result=%.2f, category=%s\n",
               result, cat_names[dataset[i].category]);
    }

    printf("\n");
}

/* -----------------------------------------------------------------------
 * print_summary() — Computes and prints summary statistics
 *
 * GDB: After this function completes, inspect the summary struct:
 *   (gdb) print summary
 *   (gdb) print summary.category_counts
 *   (gdb) print summary.average
 * ----------------------------------------------------------------------- */
static void print_summary(const struct DataPoint dataset[], int size)
{
    struct Summary summary;
    memset(&summary, 0, sizeof(summary));

    summary.count = size;
    summary.min_value = dataset[0].computed_result;
    summary.max_value = dataset[0].computed_result;

    /* GDB: Step through this loop and watch summary.total grow */
    for (int i = 0; i < size; i++) {
        double val = dataset[i].computed_result;

        summary.total += val;

        if (val < summary.min_value) summary.min_value = val;
        if (val > summary.max_value) summary.max_value = val;

        summary.category_counts[dataset[i].category]++;
    }

    summary.average = summary.total / (double)summary.count;

    /* GDB: Set a breakpoint here and inspect the final summary:
     *   (gdb) print summary
     *   (gdb) print summary.category_counts[0]   — count of CAT_LOW
     *   (gdb) print summary.category_counts[3]    — count of CAT_EXTREME */
    printf("Summary:\n");
    printf("  Count:   %d\n", summary.count);
    printf("  Total:   %.2f\n", summary.total);
    printf("  Average: %.2f\n", summary.average);
    printf("  Min:     %.2f\n", summary.min_value);
    printf("  Max:     %.2f\n", summary.max_value);
    printf("  Categories: LOW=%d  MEDIUM=%d  HIGH=%d  EXTREME=%d\n",
           summary.category_counts[CAT_LOW],
           summary.category_counts[CAT_MEDIUM],
           summary.category_counts[CAT_HIGH],
           summary.category_counts[CAT_EXTREME]);
}

/* -----------------------------------------------------------------------
 * main() — Entry point
 *
 * GDB: Start here:
 *   (gdb) break main
 *   (gdb) run
 *   (gdb) next     — step through main one line at a time
 *   (gdb) step     — dive into a function call
 * ----------------------------------------------------------------------- */
int main(void)
{
    printf("=== GDB Practice Program ===\n");
    printf("Compile: gcc -g -O0 -o debug_me debug_me.c -lm\n");
    printf("Debug:   gdb ./debug_me\n\n");

    /* GDB: After this call, inspect the dataset array:
     *   (gdb) print dataset
     *   (gdb) print dataset[0]
     *   (gdb) print dataset[7].name */
    struct DataPoint dataset[DATASET_SIZE];
    init_dataset(dataset, DATASET_SIZE);

    /* GDB: Step into this to trace the full processing chain */
    process_data(dataset, DATASET_SIZE);

    /* GDB: After this, the program is done.
     * Try: (gdb) print dataset[0].computed_result
     *      (gdb) print dataset[7].computed_result
     * Compare the before/after values. */
    print_summary(dataset, DATASET_SIZE);

    printf("\nDone. See gdb_exercises.txt for guided practice.\n");

    return 0;
}
