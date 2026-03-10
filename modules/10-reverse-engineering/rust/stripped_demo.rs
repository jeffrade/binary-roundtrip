// =============================================================================
// Stripped Rust Binary Demo — Module 10: Reverse Engineering a C Binary
// =============================================================================
//
// PURPOSE:
//   Demonstrate that stripped Rust binaries are HARDER to reverse engineer
//   than stripped C binaries, for several reasons:
//
//   1. NAME MANGLING: Rust function names like `mymodule::MyStruct::my_method`
//      become something like `_ZN8mymodule8MyStruct9my_method17h1234567890abcdefE`
//      Even unstripped, they're hard to read. Stripped, they're gone entirely.
//
//   2. LARGER BINARY: Rust statically links its standard library by default.
//      A "Hello World" in C is ~16KB. In Rust it's ~300KB-4MB.
//      More code to analyze = harder reverse engineering.
//
//   3. COMPLEX RUNTIME: Rust includes panic handling, stack unwinding,
//      format machinery, and the allocator. These add many functions
//      that aren't YOUR code but appear in the binary.
//
//   4. MONOMORPHIZATION: Generics produce separate copies of functions
//      for each concrete type. Vec<i32> and Vec<String> generate
//      completely different machine code.
//
//   5. ITERATOR CHAINS: Rust's zero-cost abstractions (map, filter, fold)
//      compile to efficient but hard-to-read loops in assembly.
//
// YOUR MISSION:
//   1. Compile: rustc -o stripped_demo stripped_demo.rs
//   2. Check size: ls -la stripped_demo (notice it's much bigger than C)
//   3. Strip: strip stripped_demo
//   4. Compare: strings stripped_demo | wc -l (many more strings than C)
//   5. Disassemble: objdump -d stripped_demo | wc -l (much more code)
//   6. Try to figure out what the program does!
//
// COMPILE:
//   rustc stripped_demo.rs -o stripped_demo          # debug (unstripped)
//   rustc -C opt-level=2 -o stripped_demo_opt stripped_demo.rs  # optimized
//   strip stripped_demo_opt                           # then strip
//
// =============================================================================

use std::collections::HashMap;

// =============================================================================
// A simple cipher implementation
// The student should try to identify this as a substitution cipher
// by analyzing the stripped binary.
//
// CLUES IN THE BINARY:
//   - String literals survive stripping (look for them with `strings`)
//   - The alphabet constant "abcdefghijklmnopqrstuvwxyz" will be visible
//   - HashMap operations generate many supporting functions
//   - The structure is harder to follow than C due to Rust's abstractions
// =============================================================================

/// Build a substitution cipher mapping.
/// In the binary, this creates a HashMap — look for alloc and hash operations.
fn build_cipher_map(key: &str) -> HashMap<char, char> {
    let alphabet = "abcdefghijklmnopqrstuvwxyz";
    let mut map = HashMap::new();

    // Remove duplicates from key, preserving order
    let mut seen = Vec::new();
    for c in key.chars() {
        let lower = c.to_ascii_lowercase();
        if lower.is_ascii_alphabetic() && !seen.contains(&lower) {
            seen.push(lower);
        }
    }

    // Fill remaining with unused alphabet characters
    for c in alphabet.chars() {
        if !seen.contains(&c) {
            seen.push(c);
        }
    }

    // Build the mapping
    for (i, c) in alphabet.chars().enumerate() {
        map.insert(c, seen[i]);
        map.insert(
            c.to_ascii_uppercase(),
            seen[i].to_ascii_uppercase(),
        );
    }

    map
}

/// Encode a message using the substitution cipher.
/// In the binary, look for:
///   - Iterator chain (chars() → map() → collect())
///   - HashMap lookup calls
///   - String allocation
fn encode_message(message: &str, cipher: &HashMap<char, char>) -> String {
    message
        .chars()
        .map(|c| *cipher.get(&c).unwrap_or(&c))
        .collect()
}

/// Build the reverse mapping for decoding.
/// This is a second HashMap with key/value swapped.
fn build_reverse_map(cipher: &HashMap<char, char>) -> HashMap<char, char> {
    cipher.iter().map(|(&k, &v)| (v, k)).collect()
}

// =============================================================================
// A word frequency counter
// This exercises: String splitting, HashMap, sorting, iterators
// In the stripped binary, these all compile to complex but fast machine code.
// =============================================================================

/// Count word frequencies in a text.
/// In the binary, look for:
///   - split_whitespace (calls into Rust's string parsing)
///   - HashMap insert/get patterns
///   - to_lowercase (Unicode-aware, pulls in lots of code)
fn count_words(text: &str) -> Vec<(String, usize)> {
    let mut counts: HashMap<String, usize> = HashMap::new();

    for word in text.split_whitespace() {
        // Clean punctuation and lowercase
        let cleaned: String = word
            .chars()
            .filter(|c| c.is_alphanumeric())
            .map(|c| c.to_ascii_lowercase())
            .collect();

        if !cleaned.is_empty() {
            *counts.entry(cleaned).or_insert(0) += 1;
        }
    }

    // Sort by frequency (descending), then alphabetically
    let mut result: Vec<(String, usize)> = counts.into_iter().collect();
    result.sort_by(|a, b| b.1.cmp(&a.1).then(a.0.cmp(&b.0)));
    result
}

// =============================================================================
// A simple stack-based calculator (RPN)
// This exercises: enum, match, Vec (as stack), parse
// Rust's enum/match compiles to jump tables — interesting in disassembly.
// =============================================================================

/// Evaluate a Reverse Polish Notation expression.
/// In the binary, look for:
///   - A match/switch (compiled as jump table or if-else chain)
///   - Vec push/pop operations (stack)
///   - f64 arithmetic (uses SSE/AVX registers, not integer registers)
///   - String parsing (str::parse)
fn rpn_calculate(expression: &str) -> Result<f64, String> {
    let mut stack: Vec<f64> = Vec::new();

    for token in expression.split_whitespace() {
        match token {
            "+" | "-" | "*" | "/" => {
                if stack.len() < 2 {
                    return Err(format!("Not enough operands for '{}'", token));
                }
                let b = stack.pop().unwrap();
                let a = stack.pop().unwrap();
                let result = match token {
                    "+" => a + b,
                    "-" => a - b,
                    "*" => a * b,
                    "/" => {
                        if b == 0.0 {
                            return Err("Division by zero".to_string());
                        }
                        a / b
                    }
                    _ => unreachable!(),
                };
                stack.push(result);
            }
            _ => {
                match token.parse::<f64>() {
                    Ok(num) => stack.push(num),
                    Err(_) => return Err(format!("Invalid token: '{}'", token)),
                }
            }
        }
    }

    if stack.len() == 1 {
        Ok(stack[0])
    } else {
        Err(format!("Invalid expression: {} values left on stack", stack.len()))
    }
}

// =============================================================================
// Main — orchestrates all the demonstrations
// =============================================================================
fn main() {
    println!("=== Rust Reverse Engineering Demo ===");
    println!("Try to figure out what this does from the stripped binary!\n");

    // --- Substitution Cipher ---
    println!("--- Section 1 ---");
    let key = "secretkey";
    let cipher = build_cipher_map(key);
    let message = "The quick brown fox jumps over the lazy dog";

    println!("Original:  {}", message);
    let encoded = encode_message(message, &cipher);
    println!("Processed: {}", encoded);

    let reverse = build_reverse_map(&cipher);
    let decoded = encode_message(&encoded, &reverse);
    println!("Reversed:  {}", decoded);
    println!();

    // --- Word Frequency ---
    println!("--- Section 2 ---");
    let text = "to be or not to be that is the question \
                whether tis nobler in the mind to suffer \
                the slings and arrows of outrageous fortune \
                or to take arms against a sea of troubles";

    let frequencies = count_words(text);
    println!("Word frequencies:");
    for (word, count) in frequencies.iter().take(10) {
        println!("  {:>12}: {}", word, count);
    }
    println!("  ({} unique words total)", frequencies.len());
    println!();

    // --- RPN Calculator ---
    println!("--- Section 3 ---");
    let expressions = vec![
        "3 4 +",
        "10 2 * 3 +",
        "15 7 1 1 + - / 3 * 2 1 1 + + -",
        "2 3 4 * +",
    ];

    for expr in expressions {
        match rpn_calculate(expr) {
            Ok(result) => println!("  {} = {}", expr, result),
            Err(e) => println!("  {} => ERROR: {}", expr, e),
        }
    }
    println!();

    // --- Binary size comparison hint ---
    println!("=== RE Difficulty Comparison ===");
    println!("When this binary is stripped, notice:");
    println!("  - Binary size: much larger than equivalent C (Rust std lib included)");
    println!("  - `strings` output: many Rust-specific strings (panic messages, etc.)");
    println!("  - `objdump -d`: thousands more functions (monomorphized generics)");
    println!("  - No function names: Rust's mangled names are gone after stripping");
    println!("  - HashMap operations: compiled to complex hash + probe sequences");
    println!("  - Iterator chains: optimized to tight loops (hard to map back to source)");
    println!();
    println!("Tip: Look for string literals with `strings` first.");
    println!("     They survive stripping and give the biggest clues.");
}
