// =============================================================================
// Module 7: Rust / LLVM Deep Dive — Zero-Cost Abstractions
// =============================================================================
//
// CONCEPT: "Zero-cost abstraction" means that using high-level constructs
// (iterators, closures, pattern matching) produces the SAME machine code as
// writing the equivalent low-level loop by hand.
//
// This is Rust's foundational promise, inherited from C++:
//   "What you don't use, you don't pay for. And further: what you do use,
//    you couldn't hand-code any better." — Bjarne Stroustrup
//
// In this file, we implement the SAME algorithm two ways:
//   1. Imperative: a hand-written for loop with if/else
//   2. Functional: an iterator chain with filter(), map(), sum()
//
// At -O2, BOTH produce identical (or nearly identical) assembly.
//
// WHY THIS MATTERS:
//   In languages like Python or JavaScript, using .map().filter().reduce()
//   creates intermediate arrays, allocates closures on the heap, and adds
//   function call overhead. The "elegant" version is measurably slower.
//
//   In Rust, the optimizer sees through the abstraction layers and produces
//   the same tight loop. You get readability AND performance.
//
// TO VERIFY:
//   1. Compile both at -O2 and emit assembly:
//        rustc --emit=asm -C opt-level=2 zero_cost.rs -o zero_cost.s
//
//   2. Search for the two functions in the assembly:
//        grep -A 30 'sum_of_squares_imperative' zero_cost.s
//        grep -A 30 'sum_of_squares_iterator' zero_cost.s
//
//   3. Compare them — they should be nearly identical. The optimizer has:
//        - Inlined the closure bodies
//        - Eliminated the Iterator trait method calls
//        - Fused the filter and map into a single loop
//        - Possibly auto-vectorized with SIMD instructions
//
//   4. Now try at -O0 (no optimization):
//        rustc --emit=asm -C opt-level=0 zero_cost.rs -o zero_cost_O0.s
//
//      At -O0, the iterator version is MUCH larger — you'll see actual
//      function calls to Iterator::filter, Iterator::map, etc.
//      The abstraction is only "zero-cost" because the optimizer erases it.
//
//   5. Compare with the C equivalent (see c/equivalent.c):
//        gcc -O2 -S c/equivalent.c -o equivalent.s
//        diff -u zero_cost.s equivalent.s
//
//      The Rust and C versions should produce very similar assembly.
//
// =============================================================================

/// Sum of squares of even numbers — imperative version.
///
/// This is the "hand-written low-level" approach:
///   - Explicit loop
///   - Explicit conditional
///   - Manual accumulator
///
/// At -O2, the compiler produces a tight loop with:
///   - A conditional branch (test + je/jne) for the even check
///   - An imul for the squaring
///   - An add for the accumulation
///
/// We mark this #[no_mangle] so the function name appears clearly in assembly.
/// We also mark it #[inline(never)] to prevent the optimizer from inlining it
/// into main (which would make it harder to find in the assembly output).
#[no_mangle]
#[inline(never)]
pub fn sum_of_squares_imperative(data: &[i64]) -> i64 {
    let mut sum: i64 = 0;
    for &x in data {
        if x % 2 == 0 {       // Check if even
            sum += x * x;      // Square and accumulate
        }
    }
    sum
}

/// Sum of squares of even numbers — iterator chain version.
///
/// This is the "high-level functional" approach:
///   - .iter() creates an iterator over the slice
///   - .filter() keeps only even numbers (lazy — no intermediate collection)
///   - .map() squares each number (lazy — no intermediate collection)
///   - .sum() drives the iteration and accumulates
///
/// CRITICAL INSIGHT: None of these create intermediate arrays or allocate
/// memory. Rust iterators are "lazy" — they compose into a single pass.
/// The compiler inlines everything into a single loop identical to the
/// imperative version.
///
/// At -O2, this produces the SAME assembly as sum_of_squares_imperative.
/// That's the zero-cost abstraction in action.
#[no_mangle]
#[inline(never)]
pub fn sum_of_squares_iterator(data: &[i64]) -> i64 {
    data.iter()
        .filter(|&&x| x % 2 == 0)   // Keep only even numbers
        .map(|&x| x * x)             // Square each one
        .sum()                        // Accumulate the sum
}

/// A more complex example: chained transformations.
/// Even this longer chain compiles to a single loop at -O2.
#[no_mangle]
#[inline(never)]
pub fn complex_chain_imperative(data: &[i64]) -> i64 {
    let mut sum: i64 = 0;
    for &x in data {
        if x > 0 && x % 3 == 0 {
            let val = x * x + 1;
            if val < 10000 {
                sum += val;
            }
        }
    }
    sum
}

#[no_mangle]
#[inline(never)]
pub fn complex_chain_iterator(data: &[i64]) -> i64 {
    data.iter()
        .filter(|&&x| x > 0 && x % 3 == 0)  // Positive multiples of 3
        .map(|&x| x * x + 1)                  // Square and add 1
        .filter(|&val| val < 10000)            // Only values under 10000
        .sum()                                 // Accumulate
}

fn main() {
    println!("=== Module 7: Zero-Cost Abstractions ===");
    println!();

    // Create test data
    let data: Vec<i64> = (1..=100).collect();

    // -------------------------------------------------------------------------
    // Simple sum of squares of evens
    // -------------------------------------------------------------------------
    let result_imperative = sum_of_squares_imperative(&data);
    let result_iterator = sum_of_squares_iterator(&data);

    println!("Sum of squares of even numbers (1..=100):");
    println!("  Imperative: {}", result_imperative);
    println!("  Iterator:   {}", result_iterator);
    println!("  Match: {}", if result_imperative == result_iterator { "YES" } else { "NO" });
    println!();

    // -------------------------------------------------------------------------
    // Complex chain
    // -------------------------------------------------------------------------
    let complex_imp = complex_chain_imperative(&data);
    let complex_iter = complex_chain_iterator(&data);

    println!("Complex chain (positive multiples of 3, squared+1, under 10000):");
    println!("  Imperative: {}", complex_imp);
    println!("  Iterator:   {}", complex_iter);
    println!("  Match: {}", if complex_imp == complex_iter { "YES" } else { "NO" });
    println!();

    // -------------------------------------------------------------------------
    // Performance comparison (both should be identical at -O2)
    // -------------------------------------------------------------------------
    // We use a larger dataset for timing to make differences visible.
    let large_data: Vec<i64> = (1..=1_000_000).collect();

    let start = std::time::Instant::now();
    let r1 = sum_of_squares_imperative(&large_data);
    let t1 = start.elapsed();

    let start = std::time::Instant::now();
    let r2 = sum_of_squares_iterator(&large_data);
    let t2 = start.elapsed();

    println!("Performance (1,000,000 elements):");
    println!("  Imperative: {} in {:?}", r1, t1);
    println!("  Iterator:   {} in {:?}", r2, t2);
    println!("  (At -O2, these should be nearly identical in speed.)");
    println!();

    // -------------------------------------------------------------------------
    // What to explore next
    // -------------------------------------------------------------------------
    println!("EXERCISES:");
    println!("  1. Compile at -O2 and compare assembly of both versions:");
    println!("       rustc --emit=asm -C opt-level=2 zero_cost.rs");
    println!("     Look for sum_of_squares_imperative and sum_of_squares_iterator.");
    println!();
    println!("  2. Compile at -O0 and see the difference:");
    println!("       rustc --emit=asm -C opt-level=0 zero_cost.rs");
    println!("     The iterator version will be MUCH larger (actual trait calls).");
    println!();
    println!("  3. Compare with the C equivalent:");
    println!("       gcc -O2 -S ../c/equivalent.c -o equivalent_c.s");
    println!("     The assembly should be nearly identical to Rust's -O2 output.");
    println!();
    println!("  4. Try adding .enumerate() or .zip() to the chain.");
    println!("     Even these complex combinators optimize away at -O2.");
}
