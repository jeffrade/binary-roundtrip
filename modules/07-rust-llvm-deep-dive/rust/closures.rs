// =============================================================================
// Module 7: Rust / LLVM Deep Dive — How Closures Compile
// =============================================================================
//
// CONCEPT: In Rust, closures are NOT heap-allocated function pointers.
// Each closure is compiled into an anonymous struct that captures the
// referenced variables, plus a trait implementation (Fn, FnMut, or FnOnce).
//
// THE THREE CLOSURE TRAITS:
//   - Fn:     Captures by shared reference (&T). Can be called multiple times.
//             The closure struct holds &T fields.
//   - FnMut:  Captures by mutable reference (&mut T). Can be called multiple
//             times but needs exclusive access.
//             The closure struct holds &mut T fields.
//   - FnOnce: Captures by value (T). Can only be called once because it
//             consumes the captured values.
//             The closure struct holds T fields directly.
//
// KEY INSIGHT: Because the compiler knows the exact type of each closure
// (it's a unique anonymous struct), it can:
//   1. Inline the closure body at every call site
//   2. Avoid any heap allocation
//   3. Avoid any dynamic dispatch (no vtable lookup)
//
// CONTRAST WITH OTHER LANGUAGES:
//   Java lambda:    Compiles to an anonymous class, boxed on the heap.
//                   Captured values are copied into the heap object.
//                   Virtual dispatch through the functional interface.
//
//   JavaScript:     Closure captures the entire scope chain.
//                   Variables live on the heap (in a "closure object").
//                   Function calls go through a pointer.
//
//   Python:         Similar to JavaScript. Closures reference the enclosing
//                   scope's namespace dictionary.
//
//   C++ lambda:     Very similar to Rust. Also generates an anonymous struct.
//                   But C++ doesn't have ownership/borrow checking.
//
// TO EXPLORE:
//   1. Compile and run:
//        rustc closures.rs -o closures && ./closures
//
//   2. View the assembly at -O2:
//        rustc --emit=asm -C opt-level=2 closures.rs -o closures.s
//      The closure calls should be fully inlined — no function pointer
//      indirection in the hot paths.
//
//   3. View the LLVM IR:
//        rustc --emit=llvm-ir closures.rs -o closures.ll
//      Search for the closure types — they appear as anonymous structs
//      like `%"closures::{{closure}}"`.
//
//   4. View the MIR to see how Rust represents closures internally:
//        rustc --emit=mir closures.rs -o closures.mir
//      You'll see the closure constructors and the trait method calls.
//
// =============================================================================

/// Demonstrates Fn closures — capture by shared reference.
///
/// HOW THIS COMPILES:
///   The compiler generates something equivalent to:
///
///   struct ClosureFn<'a> {
///       multiplier: &'a i32,   // shared reference to `multiplier`
///   }
///
///   impl<'a> Fn<(i32,)> for ClosureFn<'a> {
///       fn call(&self, (x,): (i32,)) -> i32 {
///           x * (*self.multiplier)
///       }
///   }
///
///   The closure only borrows `multiplier` — it doesn't own it.
///   Multiple Fn closures can coexist because shared references are Copy.
fn demonstrate_fn_closure() {
    println!("--- Fn closure (capture by shared reference) ---");

    let multiplier = 3;

    // This closure captures `multiplier` by shared reference (&i32).
    // It implements Fn because it only reads the captured variable.
    let multiply = |x: i32| -> i32 {
        x * multiplier  // Reads `multiplier` through &i32
    };

    // We can call it multiple times — Fn allows repeated calls.
    println!("  multiply(4) = {}", multiply(4));   // 12
    println!("  multiply(7) = {}", multiply(7));   // 21

    // We can still read `multiplier` because the closure only borrowed it.
    println!("  multiplier is still accessible: {}", multiplier);

    // We can have multiple Fn closures sharing the same data.
    let add_multiplier = |x: i32| -> i32 { x + multiplier };
    println!("  add_multiplier(10) = {}", add_multiplier(10));  // 13

    // SIZE OF THE CLOSURE:
    // At -O2, the compiler inlines the closure body, so there's no actual
    // struct allocation. But conceptually, the closure "object" is:
    //   size_of::<&i32>() = 8 bytes (one pointer)
    println!("  Size of multiply closure: {} bytes", std::mem::size_of_val(&multiply));
    println!("  (This is the size of one &i32 reference = {} bytes)", std::mem::size_of::<&i32>());
    println!();
}

/// Demonstrates FnMut closures — capture by mutable reference.
///
/// HOW THIS COMPILES:
///   struct ClosureFnMut<'a> {
///       count: &'a mut i32,   // mutable reference to `count`
///   }
///
///   impl<'a> FnMut<(i32,)> for ClosureFnMut<'a> {
///       fn call_mut(&mut self, (x,): (i32,)) -> i32 {
///           *self.count += 1;
///           x * (*self.count)
///       }
///   }
///
///   The closure holds &mut count. Because Rust enforces that only ONE
///   mutable reference exists at a time, you can't use `count` while the
///   closure exists. This is checked at compile time (zero runtime cost).
fn demonstrate_fnmut_closure() {
    println!("--- FnMut closure (capture by mutable reference) ---");

    let mut count = 0;

    // This closure captures `count` by mutable reference (&mut i32).
    // It implements FnMut because it mutates the captured variable.
    // Note: the closure itself must be declared `mut` because calling it
    // requires &mut self.
    let mut counter = |x: i32| -> i32 {
        count += 1;    // Mutates `count` through &mut i32
        x * count
    };

    // Each call mutates `count` through the mutable reference.
    println!("  counter(10) = {} (count is now 1)", counter(10));   // 10
    println!("  counter(10) = {} (count is now 2)", counter(10));   // 20
    println!("  counter(10) = {} (count is now 3)", counter(10));   // 30

    // After the closure is dropped (goes out of scope or we stop using it),
    // we can access `count` again.
    // We need to drop the closure explicitly or let it go out of scope.
    // Using the closure in a block to limit its lifetime:
    drop(counter);  // Explicitly drop the closure, releasing the &mut borrow

    println!("  count after 3 calls: {}", count);  // 3
    println!();
}

/// Demonstrates FnOnce closures — capture by value (move).
///
/// HOW THIS COMPILES:
///   struct ClosureFnOnce {
///       data: String,   // OWNS the String (not a reference)
///   }
///
///   impl FnOnce<()> for ClosureFnOnce {
///       fn call_once(self, (): ()) -> String {
///           // `self` is consumed — the struct is moved
///           self.data
///       }
///   }
///
///   Because the closure takes `self` (not &self or &mut self), calling it
///   CONSUMES the closure. You can only call it once.
fn demonstrate_fnonce_closure() {
    println!("--- FnOnce closure (capture by value / move) ---");

    let name = String::from("Rust");

    // The `move` keyword forces capture by value.
    // The String is MOVED into the closure struct — `name` is no longer valid.
    let greeting = move || {
        // `name` is owned by the closure now.
        // Returning it transfers ownership out.
        format!("Hello, {}! (This closure consumed the captured String)", name)
    };

    // name is no longer accessible here — it was moved into the closure.
    // Uncommenting the next line would cause a compile error:
    // println!("{}", name);  // ERROR: value used after move

    // Call the closure — this consumes it (FnOnce).
    let result = greeting();
    println!("  {}", result);

    // Calling again would be a compile error (the closure was consumed):
    // let result2 = greeting();  // ERROR: use of moved value

    println!();

    // -------------------------------------------------------------------------
    // Another FnOnce example: consuming a Vec
    // -------------------------------------------------------------------------
    let data = vec![1, 2, 3, 4, 5];

    let consume_data = move || -> i64 {
        // `data` is moved into the closure. When we call .into_iter(),
        // we're consuming the Vec — this is why it must be FnOnce.
        let sum: i64 = data.into_iter().map(|x: i32| x as i64 * x as i64).sum();
        sum
    };

    println!("  Sum of squares (consuming Vec): {}", consume_data());
    // consume_data() can't be called again — the Vec is gone.
    println!();
}

/// Demonstrates that closures capturing nothing are zero-sized.
///
/// A closure that doesn't capture any variables has no fields in its
/// anonymous struct, so its size is 0 bytes. The compiler can convert
/// it directly to a function pointer if needed.
fn demonstrate_zero_sized_closures() {
    println!("--- Zero-sized closures (no captures) ---");

    let no_capture = |x: i32| x * 2;

    println!("  Size of closure with no captures: {} bytes",
             std::mem::size_of_val(&no_capture));
    // This is 0 bytes! The closure has no state to store.

    // A closure that captures one i32 reference:
    let val = 42;
    let one_capture = |x: i32| x + val;
    println!("  Size of closure capturing one i32 by ref: {} bytes",
             std::mem::size_of_val(&one_capture));
    // This is 8 bytes on 64-bit — one pointer.

    // A closure that captures two values by move:
    let a = 10_i32;
    let b = 20_i64;
    let two_captures = move || a as i64 + b;
    println!("  Size of closure capturing i32+i64 by move: {} bytes",
             std::mem::size_of_val(&two_captures));
    // This is 16 bytes (4 for i32 + padding + 8 for i64) or 12 with packing.

    // A zero-sized closure can be coerced to a function pointer:
    let fn_ptr: fn(i32) -> i32 = no_capture;
    println!("  Zero-sized closure as fn pointer: fn_ptr(5) = {}", fn_ptr(5));

    // But a capturing closure CANNOT be coerced to a fn pointer:
    // let fn_ptr2: fn(i32) -> i32 = one_capture;  // ERROR!
    // Because a fn pointer has no room to store the captured state.

    // Suppress unused variable warnings
    let _ = one_capture(0);
    let _ = two_captures;

    println!();
}

/// Demonstrates generic functions taking closures — static dispatch.
///
/// When a function takes `impl Fn(i32) -> i32`, the compiler monomorphizes
/// the function for each concrete closure type. This means:
///   - No vtable lookup
///   - The closure body gets inlined
///   - Zero overhead
///
/// Compare with `dyn Fn(i32) -> i32` which uses dynamic dispatch:
///   - Vtable lookup on every call
///   - Cannot inline
///   - Requires a Box<dyn Fn> (heap allocation)
fn apply_static(f: impl Fn(i32) -> i32, x: i32) -> i32 {
    // At compile time, the compiler knows the exact closure type.
    // This call is monomorphized and the closure body is inlined.
    f(x)
}

fn apply_dynamic(f: &dyn Fn(i32) -> i32, x: i32) -> i32 {
    // The compiler does NOT know the exact closure type.
    // This call goes through a vtable (function pointer table).
    // The closure body CANNOT be inlined.
    f(x)
}

fn demonstrate_static_vs_dynamic() {
    println!("--- Static dispatch (impl Fn) vs Dynamic dispatch (dyn Fn) ---");

    let factor = 5;
    let closure = |x: i32| x * factor;

    // Static dispatch — the compiler generates apply_static::<ClosureType>
    // and inlines the closure body.
    let result_static = apply_static(&closure, 10);
    println!("  Static dispatch:  apply(|x| x * 5, 10) = {}", result_static);

    // Dynamic dispatch — goes through a vtable.
    let result_dynamic = apply_dynamic(&closure, 10);
    println!("  Dynamic dispatch: apply(|x| x * 5, 10) = {}", result_dynamic);

    println!("  (Same result, but static dispatch is faster — no vtable lookup.)");
    println!();

    println!("  In the assembly (-O2):");
    println!("    Static: the multiply instruction is inline in apply_static");
    println!("    Dynamic: there's a `call` through a function pointer in apply_dynamic");
    println!();
}

fn main() {
    println!("=== Module 7: How Closures Compile ===");
    println!();

    demonstrate_fn_closure();
    demonstrate_fnmut_closure();
    demonstrate_fnonce_closure();
    demonstrate_zero_sized_closures();
    demonstrate_static_vs_dynamic();

    println!("SUMMARY:");
    println!("  Each Rust closure is an anonymous struct + trait impl.");
    println!("  Fn:     struct holds &T references     (shared borrow)");
    println!("  FnMut:  struct holds &mut T references  (exclusive borrow)");
    println!("  FnOnce: struct holds T values            (ownership)");
    println!();
    println!("  No heap allocation. No dynamic dispatch (with impl Fn).");
    println!("  At -O2, closure calls are inlined — zero overhead.");
    println!();
    println!("  Java/JavaScript closures box captured values on the heap.");
    println!("  Rust closures live on the stack (or are optimized away entirely).");
}
