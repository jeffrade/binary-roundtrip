/* =============================================================================
 * Module 7: Rust / LLVM Deep Dive — C Equivalent of zero_cost.rs
 * =============================================================================
 *
 * This file implements the SAME algorithm as rust/zero_cost.rs:
 *   Sum of squares of even numbers in an array.
 *
 * PURPOSE:
 *   Compile both this C file and the Rust zero_cost.rs at -O2, then compare
 *   the generated assembly. They should be nearly identical, proving that
 *   Rust's iterator abstractions compile to the same machine code as
 *   hand-written C.
 *
 * TO COMPARE:
 *   1. Compile C at -O2:
 *        gcc -O2 -S c/equivalent.c -o build/equivalent_c.s
 *
 *   2. Compile Rust at -O2:
 *        rustc --emit=asm -C opt-level=2 rust/zero_cost.rs -o build/zero_cost_rust.s
 *
 *   3. Compare the assembly for the core function:
 *        grep -A 30 'sum_of_squares' build/equivalent_c.s
 *        grep -A 30 'sum_of_squares_imperative' build/zero_cost_rust.s
 *        grep -A 30 'sum_of_squares_iterator' build/zero_cost_rust.s
 *
 *   4. All three should be structurally identical:
 *        - A loop over the array
 *        - A conditional check (test + je) for evenness
 *        - A multiply (imul) for squaring
 *        - An add for accumulation
 *
 *   WHAT THIS PROVES:
 *     Rust's .iter().filter().map().sum() chain compiles to the SAME
 *     machine code as a hand-written C for loop. The abstraction is truly
 *     zero-cost. The LLVM optimizer sees through both languages' representations
 *     and produces the same optimal code.
 *
 * BONUS COMPARISON:
 *   Try compiling at -O0 (no optimization):
 *     gcc -O0 -S c/equivalent.c -o build/equivalent_c_O0.s
 *     rustc --emit=asm -C opt-level=0 rust/zero_cost.rs -o build/zero_cost_O0.s
 *
 *   At -O0, the Rust iterator version is MUCH larger than the C version,
 *   because the iterator trait methods haven't been inlined yet. The zero-cost
 *   guarantee only holds when optimization is enabled.
 *
 * ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Sum of squares of even numbers — C imperative version
 *
 * This is the "gold standard" low-level implementation. The Rust iterator
 * version should compile to identical (or better) assembly at -O2.
 *
 * We use __attribute__((noinline)) to prevent the optimizer from inlining
 * this function into main, which would make it harder to compare assembly.
 * This is equivalent to Rust's #[inline(never)].
 * -------------------------------------------------------------------------- */
__attribute__((noinline))
int64_t sum_of_squares_imperative(const int64_t *data, size_t len) {
    int64_t sum = 0;

    for (size_t i = 0; i < len; i++) {
        if (data[i] % 2 == 0) {       /* Check if even */
            sum += data[i] * data[i];   /* Square and accumulate */
        }
    }

    return sum;
}

/* ---------------------------------------------------------------------------
 * Complex chain equivalent — matches Rust's complex_chain_imperative
 *
 * Filters: positive multiples of 3, squared + 1, values under 10000.
 * -------------------------------------------------------------------------- */
__attribute__((noinline))
int64_t complex_chain(const int64_t *data, size_t len) {
    int64_t sum = 0;

    for (size_t i = 0; i < len; i++) {
        if (data[i] > 0 && data[i] % 3 == 0) {
            int64_t val = data[i] * data[i] + 1;
            if (val < 10000) {
                sum += val;
            }
        }
    }

    return sum;
}

/* ---------------------------------------------------------------------------
 * Helper: get current time in nanoseconds (for benchmarking)
 * -------------------------------------------------------------------------- */
static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/* ---------------------------------------------------------------------------
 * Main: Run the computations and print results
 * -------------------------------------------------------------------------- */
int main(void) {
    printf("=== Module 7: C Equivalent (for comparison with Rust) ===\n\n");

    /* Create test data: 1, 2, 3, ..., 100 */
    const int N_SMALL = 100;
    int64_t small_data[100];
    for (int i = 0; i < N_SMALL; i++) {
        small_data[i] = i + 1;
    }

    /* -------------------------------------------------------------------
     * Small dataset: verify correctness
     * ------------------------------------------------------------------ */
    int64_t result = sum_of_squares_imperative(small_data, N_SMALL);
    int64_t complex_result = complex_chain(small_data, N_SMALL);

    printf("Sum of squares of even numbers (1..=100):\n");
    printf("  C imperative: %ld\n", result);
    printf("  (Compare with Rust output — should be identical)\n\n");

    printf("Complex chain (positive multiples of 3, squared+1, under 10000):\n");
    printf("  C imperative: %ld\n", complex_result);
    printf("  (Compare with Rust output — should be identical)\n\n");

    /* -------------------------------------------------------------------
     * Large dataset: benchmark
     * ------------------------------------------------------------------ */
    const int N_LARGE = 1000000;
    int64_t *large_data = malloc(N_LARGE * sizeof(int64_t));
    if (!large_data) {
        fprintf(stderr, "Failed to allocate memory\n");
        return 1;
    }

    for (int i = 0; i < N_LARGE; i++) {
        large_data[i] = i + 1;
    }

    uint64_t start, end;

    start = now_ns();
    int64_t large_result = sum_of_squares_imperative(large_data, N_LARGE);
    end = now_ns();

    printf("Performance (1,000,000 elements):\n");
    printf("  C imperative: %ld in %lu ns\n", large_result, (unsigned long)(end - start));
    printf("  (Compare with Rust output — should be similar speed at -O2)\n\n");

    free(large_data);

    /* -------------------------------------------------------------------
     * Assembly comparison instructions
     * ------------------------------------------------------------------ */
    printf("TO COMPARE ASSEMBLY:\n");
    printf("  1. Compile C at -O2:\n");
    printf("       gcc -O2 -S equivalent.c -o equivalent_c.s\n\n");
    printf("  2. Compile Rust at -O2:\n");
    printf("       rustc --emit=asm -C opt-level=2 ../rust/zero_cost.rs -o zero_cost.s\n\n");
    printf("  3. Compare the core loop in both files.\n");
    printf("     They should be nearly identical — the same test/je/imul/add pattern.\n\n");
    printf("  4. This proves Rust's iterator abstractions are truly zero-cost:\n");
    printf("     .iter().filter().map().sum() compiles to the same code as a C for loop.\n");

    return 0;
}
