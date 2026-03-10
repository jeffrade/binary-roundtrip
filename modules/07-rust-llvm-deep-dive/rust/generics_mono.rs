// =============================================================================
// Module 7: Rust / LLVM Deep Dive — Generics & Monomorphization
// =============================================================================
//
// CONCEPT: When you write a generic function in Rust, the compiler generates
// a separate, specialized copy for EVERY concrete type you use it with. This
// is called "monomorphization" (mono = one, morph = shape — making one shape
// per type).
//
// KEY INSIGHT: There is NO runtime type checking, NO boxing, NO vtable lookup.
// Each call goes directly to a fully specialized function. This is why Rust
// generics have zero runtime overhead — but it comes at a cost: binary size.
//
// CONTRAST WITH JAVA:
//   Java uses "type erasure" for generics. List<Integer> and List<String> share
//   the same bytecode (List<Object>). This means:
//     - Smaller binary (one copy of the code)
//     - But: boxing required (int -> Integer), runtime type casts, no
//       specialization
//
//   Rust monomorphization gives you:
//     - Larger binary (one copy per type)
//     - But: zero overhead, full optimization per type, no boxing
//
// TO EXPLORE:
//   1. Compile and run:
//        rustc generics_mono.rs -o generics_mono && ./generics_mono
//
//   2. Generate LLVM IR and look for the monomorphized functions:
//        rustc --emit=llvm-ir generics_mono.rs -o generics_mono.ll
//        grep 'define.*double' generics_mono.ll
//
//      You should see THREE separate function definitions:
//        - One for i32 (double<i32>)
//        - One for f64 (double<f64>)
//        - One for i64 (double<i64>)
//
//      Each will have different LLVM IR instructions:
//        - i32 version uses: mul i32
//        - i64 version uses: mul i64
//        - f64 version uses: fmul double
//
//   3. Look at the generated assembly:
//        rustc --emit=asm -C opt-level=2 generics_mono.rs -o generics_mono.s
//        grep -A 5 'double' generics_mono.s
//
//      The i32 version uses `imull`, f64 uses `mulsd`, i64 uses `imulq`.
//      The compiler has fully specialized each function for its type.
//
//   4. Check the binary size impact:
//        rustc generics_mono.rs -o mono_binary
//        ls -la mono_binary
//
//      Now imagine a library with 50 generic functions called with 10 types
//      each — that's 500 function instantiations! This is why Rust binaries
//      tend to be larger than equivalent C binaries.
//
// =============================================================================

use std::ops::Mul;

/// A generic function that doubles a value by multiplying it by itself.
///
/// The trait bound `T: Mul<Output=T> + Copy` means:
///   - T must support multiplication (the Mul trait)
///   - The result of multiplication must also be T (Output=T)
///   - T must be Copy (so we can use `x` twice without moving it)
///
/// WHAT HAPPENS AT COMPILE TIME:
///   The compiler sees every call site, determines the concrete type,
///   and generates a specialized version. This function doesn't exist
///   as a single function in the binary — it becomes multiple functions.
fn double<T: Mul<Output = T> + Copy>(x: T) -> T {
    // In the LLVM IR, this single line becomes different instructions
    // depending on the type:
    //   i32: %result = mul i32 %x, %x
    //   i64: %result = mul i64 %x, %x
    //   f64: %result = fmul double %x, %x
    x * x
}

/// Another generic function to demonstrate monomorphization with multiple
/// type parameters. Each unique combination of (T, U) gets its own copy.
fn add_and_scale<T, U>(a: T, b: T, scale: U) -> T
where
    T: std::ops::Add<Output = T> + Mul<Output = T> + Copy,
    U: Into<T> + Copy,
    T: From<U>,
{
    // This function will be monomorphized for each (T, U) combination used.
    // If we call it with (i32, i32), (f64, f64), and (i32, i64), we get
    // three separate compiled functions.
    let sum = a + b;
    let scale_as_t: T = scale.into();
    sum * scale_as_t
}

/// A generic struct — each instantiation creates a separate type in the
/// compiled binary, with its own methods.
///
/// In the LLVM IR, you'll see separate struct definitions:
///   %Wrapper_i32 = type { i32 }
///   %Wrapper_f64 = type { double }
struct Wrapper<T: Mul<Output = T> + Copy + std::fmt::Display> {
    value: T,
}

impl<T: Mul<Output = T> + Copy + std::fmt::Display> Wrapper<T> {
    fn new(value: T) -> Self {
        Wrapper { value }
    }

    /// This method is also monomorphized — one copy per T.
    fn doubled(&self) -> T {
        self.value * self.value
    }

    fn show(&self) {
        // Even this println! call gets monomorphized, because the Display
        // trait implementation is different for each type.
        println!("  Wrapper{{ value: {} }} -> doubled: {}", self.value, self.doubled());
    }
}

fn main() {
    println!("=== Module 7: Generics & Monomorphization ===");
    println!();

    // -------------------------------------------------------------------------
    // Calling double<T> with three different types
    // -------------------------------------------------------------------------
    // Each of these calls triggers the compiler to generate a separate function.
    // After monomorphization, the binary contains:
    //   - double::<i32>(i32) -> i32      using integer multiplication
    //   - double::<f64>(f64) -> f64      using floating-point multiplication
    //   - double::<i64>(i64) -> i64      using 64-bit integer multiplication

    let int_result = double(5_i32);        // Calls double::<i32>
    let float_result = double(3.14_f64);   // Calls double::<f64>
    let long_result = double(1000_i64);    // Calls double::<i64>

    println!("double(5_i32)    = {} (uses imull on x86)", int_result);
    println!("double(3.14_f64) = {} (uses mulsd on x86)", float_result);
    println!("double(1000_i64) = {} (uses imulq on x86)", long_result);
    println!();

    // -------------------------------------------------------------------------
    // Generic struct instantiation
    // -------------------------------------------------------------------------
    // Each Wrapper<T> becomes a separate type with its own vtable-free methods.

    println!("Generic struct monomorphization:");
    let w1 = Wrapper::new(7_i32);
    let w2 = Wrapper::new(2.718_f64);
    let w3 = Wrapper::new(42_i64);

    w1.show();   // Calls Wrapper::<i32>::show()
    w2.show();   // Calls Wrapper::<f64>::show()
    w3.show();   // Calls Wrapper::<i64>::show()
    println!();

    // -------------------------------------------------------------------------
    // Binary size impact demonstration
    // -------------------------------------------------------------------------
    println!("KEY TAKEAWAY:");
    println!("  Each generic instantiation creates a NEW function in the binary.");
    println!("  3 calls to double<T> = 3 separate functions in machine code.");
    println!("  3 Wrapper<T> instances = 3 copies of every Wrapper method.");
    println!();
    println!("  This is the trade-off:");
    println!("    Rust (monomorphization): Zero runtime cost, larger binary");
    println!("    Java (type erasure):     Runtime boxing cost, smaller binary");
    println!("    C++ (templates):         Same as Rust — each instantiation = new code");
    println!();

    // -------------------------------------------------------------------------
    // Verify: look at the LLVM IR
    // -------------------------------------------------------------------------
    println!("TO VERIFY THIS YOURSELF:");
    println!("  rustc --emit=llvm-ir generics_mono.rs");
    println!("  grep 'define.*double' generics_mono.ll");
    println!();
    println!("  You will see three separate 'define' entries — one per type.");
    println!("  That's monomorphization in action.");
}
