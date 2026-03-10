// debug_me.rs — A Rust program designed for GDB practice
// ========================================================
//
// Module 6: Debuggers at the Binary Level
//
// PURPOSE:
//   This is the Rust equivalent of debug_me.c — a program with no bugs,
//   designed to give you a rich environment for practicing GDB.
//
//   Rust produces DWARF debug info just like C, so GDB works well with
//   Rust binaries. However, there are some differences:
//
//     1. Rust names are mangled (e.g., debug_me::process_data)
//        GDB handles this — you can use the full path or set breakpoints
//        by line number.
//
//     2. Rust's Vec, String, etc. are more complex in memory than C arrays.
//        GDB can still show them, but the raw structure is more nested.
//
//     3. Rust's enums are tagged unions — GDB shows the variant and value.
//
//     4. Rust-specific: closures, iterators, and traits have complex
//        representations that are harder to inspect in GDB.
//
// HOW TO BUILD AND DEBUG:
//   rustc -g -o debug_me_rs debug_me.rs
//   gdb ./debug_me_rs
//
// GDB TIPS FOR RUST:
//   (gdb) break debug_me::main         — use module::function syntax
//   (gdb) break debug_me.rs:120        — or break by line number
//   (gdb) print data                   — works for simple types
//   (gdb) print point                  — shows struct fields
//   (gdb) info locals                  — shows all local variables
//   (gdb) ptype Point                  — shows the struct layout
//
// CALL GRAPH:
//   main()
//     ├─→ create_dataset()     — builds a Vec<DataPoint>
//     ├─→ process_data()       — iterates and computes
//     │     ├─→ compute()      — arithmetic on each point
//     │     │     └─→ helper() — utility function
//     │     └─→ classify()     — categorize result
//     └─→ print_summary()      — final output

/// Categories for classifying results
#[derive(Debug, Clone, Copy, PartialEq)]
enum Category {
    Low,
    Medium,
    High,
    Extreme,
}

/// Our main data structure
///
/// GDB: Try these commands:
///   (gdb) print point
///   (gdb) print point.name
///   (gdb) ptype DataPoint
#[derive(Debug, Clone)]
struct DataPoint {
    name: String,
    id: u32,
    value: f64,
    computed_result: f64,
    category: Category,
}

/// Summary statistics
#[derive(Debug)]
struct Summary {
    count: usize,
    total: f64,
    average: f64,
    min_value: f64,
    max_value: f64,
    category_counts: [usize; 4],
}

/// A simple utility function at the bottom of the call chain.
///
/// GDB: When stopped here, try:
///   (gdb) backtrace       — see the full call chain
///   (gdb) frame 1         — jump to compute()'s frame
///   (gdb) info args        — see compute()'s arguments
///   (gdb) info locals      — see helper()'s locals
fn helper(x: f64, factor: f64) -> f64 {
    // GDB: Set a breakpoint here and inspect x and factor
    let adjusted = x * factor;

    // GDB: Step over this, then print 'scaled'
    let scaled = adjusted / 100.0;

    scaled + adjusted
}

/// Performs a calculation on a data point.
///
/// GDB: Try these when stopped here:
///   (gdb) print *dp           — show the DataPoint
///   (gdb) print dp.name       — access a field
///   (gdb) step                — step INTO helper()
///   (gdb) finish              — run until helper returns
fn compute(dp: &mut DataPoint, iteration: usize) -> f64 {
    let base = dp.value * (1.0 + iteration as f64 * 0.1);

    // GDB: Step into helper()
    let result = helper(base, 2.5);

    // Apply transformation
    let result = result.abs().sqrt() + (dp.id as f64 * 3.0);

    dp.computed_result = result;
    result
}

/// Categorizes a result value.
///
/// GDB: Practice conditional breakpoints:
///   (gdb) break debug_me::classify if result > 50.0
fn classify(result: f64) -> Category {
    // GDB: Step through to see which branch is taken
    if result < 10.0 {
        Category::Low
    } else if result < 30.0 {
        Category::Medium
    } else if result < 50.0 {
        Category::High
    } else {
        Category::Extreme
    }
}

/// Creates the dataset.
///
/// GDB: After this returns, inspect the Vec:
///   (gdb) print dataset
///   (gdb) print dataset[0]
///   (gdb) print dataset.len    — might need: print dataset.len()
fn create_dataset() -> Vec<DataPoint> {
    let names = [
        "Alpha", "Beta", "Gamma", "Delta",
        "Epsilon", "Zeta", "Eta", "Theta",
    ];

    // GDB: Set a breakpoint in this loop
    //   (gdb) print i
    //   (gdb) print name
    names.iter().enumerate().map(|(i, &name)| {
        DataPoint {
            name: name.to_string(),
            id: (i + 1) as u32,
            value: (i * 7 + 3) as f64 * 1.5,
            computed_result: 0.0,
            category: Category::Low,
        }
    }).collect()
}

/// Main processing loop.
///
/// GDB: Best function for practicing loop debugging:
///   (gdb) break debug_me::process_data
///   (gdb) display i                    — auto-print at every stop
fn process_data(dataset: &mut Vec<DataPoint>) {
    println!("Processing {} data points...\n", dataset.len());

    // GDB: Using a traditional loop so 'i' is easy to inspect
    let len = dataset.len();
    for i in 0..len {
        println!(
            "  [{}] Processing '{}' (value={:.2})...",
            i, dataset[i].name, dataset[i].value
        );

        // GDB: Step into compute()
        let result = compute(&mut dataset[i], i);

        // GDB: Step into classify()
        dataset[i].category = classify(result);

        let cat_name = match dataset[i].category {
            Category::Low     => "LOW",
            Category::Medium  => "MEDIUM",
            Category::High    => "HIGH",
            Category::Extreme => "EXTREME",
        };
        println!("       -> result={:.2}, category={}", result, cat_name);
    }
    println!();
}

/// Computes and prints summary statistics.
///
/// GDB: After completion, inspect:
///   (gdb) print summary
///   (gdb) print summary.category_counts
fn print_summary(dataset: &[DataPoint]) {
    let mut summary = Summary {
        count: dataset.len(),
        total: 0.0,
        average: 0.0,
        min_value: dataset[0].computed_result,
        max_value: dataset[0].computed_result,
        category_counts: [0; 4],
    };

    // GDB: Watch summary.total grow through the loop
    for dp in dataset {
        summary.total += dp.computed_result;

        if dp.computed_result < summary.min_value {
            summary.min_value = dp.computed_result;
        }
        if dp.computed_result > summary.max_value {
            summary.max_value = dp.computed_result;
        }

        let cat_idx = match dp.category {
            Category::Low     => 0,
            Category::Medium  => 1,
            Category::High    => 2,
            Category::Extreme => 3,
        };
        summary.category_counts[cat_idx] += 1;
    }

    summary.average = summary.total / summary.count as f64;

    // GDB: Inspect the final summary
    //   (gdb) print summary
    println!("Summary:");
    println!("  Count:   {}", summary.count);
    println!("  Total:   {:.2}", summary.total);
    println!("  Average: {:.2}", summary.average);
    println!("  Min:     {:.2}", summary.min_value);
    println!("  Max:     {:.2}", summary.max_value);
    println!(
        "  Categories: LOW={}  MEDIUM={}  HIGH={}  EXTREME={}",
        summary.category_counts[0],
        summary.category_counts[1],
        summary.category_counts[2],
        summary.category_counts[3]
    );
}

/// Entry point — start GDB practice here.
///
/// GDB:
///   (gdb) break debug_me::main
///   (gdb) run
///   (gdb) next       — step through one line at a time
///   (gdb) step       — dive into function calls
fn main() {
    println!("=== Rust GDB Practice Program ===");
    println!("Build: rustc -g -o debug_me_rs debug_me.rs");
    println!("Debug: gdb ./debug_me_rs");
    println!();

    // GDB: After this, inspect the dataset
    //   (gdb) print dataset
    //   (gdb) print dataset[0]
    let mut dataset = create_dataset();

    // GDB: Step into this to trace the processing chain
    process_data(&mut dataset);

    // GDB: After this, compare before/after computed_result values
    print_summary(&dataset);

    println!("\nDone. See gdb_exercises.txt for guided practice.");
}
