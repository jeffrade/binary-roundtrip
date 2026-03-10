/*
 * find_the_bug.c — A program with 3 intentional bugs. Find them with GDB!
 * ========================================================================
 *
 * Module 6: Debuggers at the Binary Level
 *
 * CHALLENGE:
 *   This program has exactly 3 bugs. It compiles and runs, but produces
 *   incorrect results. Your task is to use GDB to find all three.
 *
 *   The bugs are:
 *     Bug 1: An off-by-one array access
 *     Bug 2: Using an uninitialized variable
 *     Bug 3: An integer overflow
 *
 * HOW TO DEBUG:
 *   gcc -g -O0 -Wno-uninitialized -o find_the_bug find_the_bug.c
 *   gdb ./find_the_bug
 *
 *   Hints:
 *     - Run the program first to see the wrong output
 *     - Set breakpoints in each function
 *     - Step through and inspect variable values
 *     - Ask yourself: "Is this value what I expected?"
 *
 * DO NOT SCROLL TO THE BOTTOM — the answers are there, separated by
 * a large gap. Try to find the bugs yourself first!
 */

#include <stdio.h>
#include <string.h>

#define NUM_SCORES  5

/* -----------------------------------------------------------------------
 * Bug 1 is in this function.
 *
 * This function should compute the average of an array of scores.
 *
 * GDB hints:
 *   (gdb) break compute_average
 *   (gdb) run
 *   (gdb) print scores[0]   through   print scores[4]
 *   (gdb) print scores[5]    — what's at this index?
 *   (gdb) watch sum          — watch it accumulate
 * ----------------------------------------------------------------------- */
double compute_average(int scores[], int count)
{
    double sum = 0.0;

    /*
     * This loop is supposed to average 'count' elements.
     * Look very carefully at the loop bound...
     */
    for (int i = 0; i <= count; i++) {
        printf("  Adding scores[%d] = %d\n", i, scores[i]);
        sum += scores[i];
    }

    return sum / (double)count;
}

/* -----------------------------------------------------------------------
 * Bug 2 is in this function.
 *
 * This function should find the maximum value in an array.
 *
 * GDB hints:
 *   (gdb) break find_maximum
 *   (gdb) run
 *   (gdb) print max_val    — what's its initial value?
 *   (gdb) info locals      — see ALL local variables
 * ----------------------------------------------------------------------- */
int find_maximum(int data[], int count)
{
    int max_val;  /* What's the initial value? */

    for (int i = 0; i < count; i++) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }

    return max_val;
}

/* -----------------------------------------------------------------------
 * Bug 3 is in this function.
 *
 * This function computes the product of large numbers.
 *
 * GDB hints:
 *   (gdb) break compute_product
 *   (gdb) run
 *   (gdb) next
 *   (gdb) print result     — is this what you expected?
 *   (gdb) print/x result   — look at it in hex
 *   Think about the range of 'int' (32-bit signed: max ≈ 2.1 billion)
 * ----------------------------------------------------------------------- */
int compute_product(int a, int b)
{
    /*
     * Multiply two large numbers.
     * What happens when the result exceeds INT_MAX?
     */
    int result = a * b;

    printf("  %d * %d = %d\n", a, b, result);

    return result;
}

/* -----------------------------------------------------------------------
 * main() — Calls the buggy functions
 * ----------------------------------------------------------------------- */
int main(void)
{
    printf("=== Find the Bug: 3 bugs to find with GDB ===\n\n");

    /* ----- Test compute_average ----- */
    printf("--- Test 1: compute_average ---\n");
    int scores[NUM_SCORES] = {85, 92, 78, 96, 88};

    printf("Scores: ");
    for (int i = 0; i < NUM_SCORES; i++) {
        printf("%d ", scores[i]);
    }
    printf("\n");

    double avg = compute_average(scores, NUM_SCORES);
    printf("Computed average: %.2f\n", avg);
    printf("Expected average: %.2f\n\n", (85.0+92+78+96+88) / 5.0);

    /* ----- Test find_maximum ----- */
    printf("--- Test 2: find_maximum ---\n");
    int data[] = {-5, -12, -3, -8, -1};

    printf("Data: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");

    int max = find_maximum(data, 5);
    printf("Computed maximum: %d\n", max);
    printf("Expected maximum: -1\n\n");

    /* ----- Test compute_product ----- */
    printf("--- Test 3: compute_product ---\n");
    int a = 100000;
    int b = 100000;

    int product = compute_product(a, b);
    printf("Expected: %lld\n", (long long)a * (long long)b);
    printf("Got:      %d\n", product);
    if (product != (int)((long long)a * b)) {
        printf("  (These don't match! Why?)\n");
    }
    printf("\n");

    /* ----- Summary ----- */
    printf("=== Results ===\n");
    printf("Did the computed values match the expected values?\n");
    printf("If not, use GDB to find out why!\n");
    printf("\n");
    printf("  gdb ./find_the_bug\n");
    printf("  (gdb) break compute_average\n");
    printf("  (gdb) break find_maximum\n");
    printf("  (gdb) break compute_product\n");
    printf("  (gdb) run\n");

    return 0;
}


/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * ═══════════════════════════════════════════════════════════════════
 *                     SPOILER ALERT — BUG ANSWERS BELOW
 * ═══════════════════════════════════════════════════════════════════
 *
 * BUG 1: Off-by-one in compute_average()
 *   Line: for (int i = 0; i <= count; i++)
 *   Fix:  for (int i = 0; i <  count; i++)
 *
 *   The loop iterates count+1 times instead of count times.
 *   When i == count (i.e., i == 5), scores[5] reads PAST the end
 *   of the array (valid indices are 0-4). This reads garbage memory,
 *   corrupting the sum and producing an incorrect average.
 *
 *   In GDB, you can see this by printing scores[5] — it will be
 *   some random value from whatever is next on the stack.
 *
 * BUG 2: Uninitialized variable in find_maximum()
 *   Line: int max_val;   (no initializer)
 *   Fix:  int max_val = data[0];
 *
 *   max_val starts with whatever garbage was on the stack. If all
 *   values in the array are negative (as in our test), and the
 *   garbage happens to be 0 or positive, none of the array values
 *   will be greater than max_val, so the function returns garbage.
 *
 *   In GDB: (gdb) print max_val   — immediately after the declaration,
 *   it will show some random value like 32767 or -1094795586.
 *
 * BUG 3: Integer overflow in compute_product()
 *   Line: int result = a * b;   (100000 * 100000 = 10,000,000,000)
 *   Fix:  long long result = (long long)a * b;
 *
 *   int is 32-bit signed on most platforms, with a maximum value of
 *   2,147,483,647 (about 2.1 billion). 100000 * 100000 = 10 billion,
 *   which overflows. The result wraps around (undefined behavior in C,
 *   but typically wraps on x86).
 *
 *   In GDB: (gdb) print a * b  — GDB will show the overflowed value.
 *           (gdb) print (long long)a * b  — GDB will show the correct value.
 *
 * ═══════════════════════════════════════════════════════════════════
 */
