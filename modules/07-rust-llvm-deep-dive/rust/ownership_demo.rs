// =============================================================================
// Module 7: Rust / LLVM Deep Dive — Ownership at the Compiler Level
// =============================================================================
//
// CONCEPT: Rust's ownership system (move semantics, borrowing, lifetimes)
// is entirely a COMPILE-TIME mechanism. The borrow checker runs during
// compilation — specifically during MIR (Mid-level Intermediate Representation)
// analysis — and produces ZERO runtime code.
//
// This means:
//   - No reference counting (unlike Swift's ARC or Python's refcount)
//   - No garbage collector (unlike Java, Go, JavaScript)
//   - No runtime null checks (unlike Java's NullPointerException)
//   - The compiled binary has NO ownership-related overhead
//
// THE COMPILATION PIPELINE FOR BORROW CHECKING:
//   1. Source code (.rs)
//   2. Parse into AST (Abstract Syntax Tree)
//   3. Lower to HIR (High-level IR) — desugaring, name resolution
//   4. Type checking on HIR
//   5. Lower to MIR (Mid-level IR) — THIS IS WHERE BORROW CHECKING HAPPENS
//   6. BORROW CHECKER runs on MIR:
//      - Tracks which variables own which values
//      - Tracks which borrows are active
//      - Ensures no use-after-move
//      - Ensures no aliasing + mutation
//      - Computes lifetimes (Non-Lexical Lifetimes / NLL)
//   7. Lower to LLVM IR — ownership annotations are GONE at this point
//   8. LLVM optimizes and generates machine code
//
// KEY INSIGHT: Step 7 is crucial. By the time the code reaches LLVM IR,
// ALL ownership/borrowing information has been erased. The LLVM IR looks
// exactly like C code would — just loads, stores, and calls. The safety
// guarantees were enforced at compile time and left no trace in the binary.
//
// TO EXPLORE:
//   1. Compile and view MIR (where borrow checking happens):
//        rustc --emit=mir ownership_demo.rs -o ownership_demo.mir
//
//      In the MIR, you'll see:
//        - `_1 = move _2`        (ownership transfer)
//        - `_3 = &_1`            (shared borrow)
//        - `_4 = &mut _1`        (mutable borrow)
//        - `StorageLive(_1)`     (variable comes into scope)
//        - `StorageDead(_1)`     (variable goes out of scope — drop happens)
//
//   2. Compare with LLVM IR (where ownership is erased):
//        rustc --emit=llvm-ir ownership_demo.rs -o ownership_demo.ll
//
//      In the LLVM IR, there are NO borrow annotations. Just:
//        - `%ptr = alloca i32`   (stack allocation)
//        - `store i32 42, ptr %ptr`
//        - `%val = load i32, ptr %ptr`
//        - `call void @drop(%String* %s)`  (destructor call)
//
//      The ownership system compiles away completely.
//
// =============================================================================

/// Demonstrates move semantics.
///
/// When a value is "moved", its memory is NOT copied or freed. The compiler
/// simply changes which variable name refers to that memory, and invalidates
/// the old name. In the MIR, this appears as `_dest = move _src`.
///
/// In the LLVM IR, a move of a small value is just a register copy.
/// For heap-allocated values (like String), the pointer/length/capacity
/// are copied, but the heap data is NOT copied or reallocated.
fn demonstrate_moves() {
    println!("--- Move Semantics ---");

    // -------------------------------------------------------------------------
    // Move of a heap-allocated String
    // -------------------------------------------------------------------------
    let s1 = String::from("hello");
    println!("  s1 = {:?} (owns the heap buffer)", s1);

    // This MOVES s1's data into s2.
    // What happens at the machine level:
    //   - s1's three fields (pointer, length, capacity) are copied to s2
    //   - s1 is marked as "invalid" by the compiler (compile-time only)
    //   - NO heap allocation, NO data copy, NO reference count change
    //
    // In MIR: _2 = move _1
    // In LLVM IR: just three register copies (ptr, len, cap)
    let s2 = s1;
    println!("  s2 = {:?} (s1's data moved here)", s2);

    // s1 is now invalid. The compiler prevents us from using it.
    // Uncommenting the next line causes a compile-time error:
    // println!("  s1 = {:?}", s1);  // ERROR: value used after move
    //
    // NOTE: This check happens at compile time. In the binary, there is
    // NO runtime check. The compiler simply refuses to compile the code
    // if it detects a use-after-move.

    // When s2 goes out of scope, its destructor (Drop) is called.
    // In MIR: drop(_2)
    // In LLVM IR: call void @alloc::string::String::drop(&s2)
    // This frees the heap buffer. No double-free because s1 was invalidated.

    println!("  After move: s1 is INVALID (compile-time enforcement)");
    println!("  Cost of move: 3 register copies (ptr, len, cap)");
    println!("  Cost of safety check: ZERO (it's a compile-time error)");
    println!();

    // -------------------------------------------------------------------------
    // Move vs Copy for stack types
    // -------------------------------------------------------------------------
    // Types that implement Copy (like i32, f64, bool) are always copied,
    // never moved. This is because copying them is as cheap as moving them.
    let x: i32 = 42;
    let y = x;     // This is a COPY, not a move. x is still valid.
    println!("  i32 copy: x = {}, y = {} (both valid — i32 implements Copy)", x, y);
    println!();
}

/// Demonstrates shared borrowing (&T).
///
/// A shared borrow creates a pointer that the compiler tracks.
/// The borrow checker ensures:
///   1. The borrowed data lives long enough (lifetime check)
///   2. No one mutates the data while shared borrows exist (aliasing rule)
///
/// In MIR: _ref = &_original
/// In LLVM IR: just a pointer — %ref = getelementptr or just the address
/// In assembly: just a lea (load effective address) instruction
///
/// Runtime cost of borrowing: ONE POINTER. Same as C.
fn demonstrate_shared_borrows() {
    println!("--- Shared Borrowing (&T) ---");

    let data = vec![1, 2, 3, 4, 5];

    // Create a shared borrow. Multiple shared borrows can coexist.
    let borrow1 = &data;
    let borrow2 = &data;

    // Both borrows are valid simultaneously — shared access is fine.
    println!("  data = {:?}", data);
    println!("  borrow1 = {:?} (shared reference)", borrow1);
    println!("  borrow2 = {:?} (shared reference)", borrow2);

    // While shared borrows exist, mutation is forbidden:
    // data.push(6);  // ERROR: cannot borrow `data` as mutable because
    //                //        it is also borrowed as immutable

    // This rule prevents data races at compile time:
    //   - Multiple readers (shared borrows) = safe
    //   - One writer (mutable borrow) = safe
    //   - Readers + writer simultaneously = UNSAFE (prevented by compiler)

    println!("  Both borrows coexist — compile-time verified, zero runtime cost");
    println!("  In LLVM IR: borrow1 and borrow2 are just pointers to data");
    println!();
}

/// Demonstrates mutable borrowing (&mut T).
///
/// A mutable borrow gives exclusive access. The compiler ensures
/// that NO other references (shared or mutable) exist while a
/// mutable borrow is active.
///
/// This is the core of Rust's aliasing discipline:
///   At any point in time, you can have EITHER:
///     - Any number of shared references (&T), OR
///     - Exactly ONE mutable reference (&mut T)
///   But NEVER both.
///
/// This rule is what prevents data races, iterator invalidation,
/// and use-after-free — ALL at compile time with ZERO runtime cost.
fn demonstrate_mutable_borrows() {
    println!("--- Mutable Borrowing (&mut T) ---");

    let mut data = vec![1, 2, 3];
    println!("  data before: {:?}", data);

    // Create a mutable borrow — exclusive access.
    {
        let borrow_mut = &mut data;

        // While this mutable borrow exists:
        //   - We CANNOT read `data` directly
        //   - We CANNOT create another &data or &mut data
        //   - Only `borrow_mut` can access the vector

        borrow_mut.push(4);
        borrow_mut.push(5);
        println!("  Modified through &mut: {:?}", borrow_mut);

        // This exclusivity is what prevents bugs like:
        //   - Iterator invalidation (modifying a collection while iterating)
        //   - Data races (two threads reading and writing the same data)
        //   - Dangling pointers (reference to freed memory)
    }
    // Mutable borrow is dropped here — `data` is accessible again.

    println!("  data after: {:?}", data);
    println!("  Exclusive access enforced at compile time — zero runtime cost");
    println!();
}

/// Demonstrates Non-Lexical Lifetimes (NLL).
///
/// Before Rust 2018, borrow lifetimes were based on lexical scope (curly
/// braces). Now, the borrow checker is smarter — it tracks the ACTUAL
/// last use of each borrow, not just the scope boundary.
///
/// This analysis happens in MIR. The borrow checker builds a "liveness"
/// map for each reference and checks that no conflicts exist.
fn demonstrate_nll() {
    println!("--- Non-Lexical Lifetimes (NLL) ---");

    let mut data = String::from("hello");

    // Create a shared borrow.
    let r = &data;
    println!("  Shared borrow: {}", r);
    // `r` is last used on the line above.
    // With NLL, the borrow ENDS here (not at the end of the block).

    // Now we can create a mutable borrow — the shared borrow is already done.
    data.push_str(", world!");
    println!("  After mutation: {}", data);

    println!("  NLL recognized that the shared borrow ended early.");
    println!("  In MIR, you can see the precise lifetime regions for each borrow.");
    println!();
}

/// Demonstrates how Drop (destructors) work with ownership.
///
/// When a value goes out of scope, Rust calls its destructor (the Drop trait).
/// In MIR, this appears as `drop(_variable)` at the end of its scope.
/// In LLVM IR, it's a call to the type's drop function.
///
/// The compiler determines drop order at compile time:
///   - Local variables are dropped in REVERSE declaration order
///   - This is deterministic (unlike GC languages where finalization order
///     is unpredictable)
struct Droppable {
    name: &'static str,
}

impl Drop for Droppable {
    fn drop(&mut self) {
        println!("    Dropping '{}'", self.name);
    }
}

fn demonstrate_drop_order() {
    println!("--- Drop Order (Deterministic Destructors) ---");

    let _a = Droppable { name: "first" };
    let _b = Droppable { name: "second" };
    let _c = Droppable { name: "third" };

    println!("  Created: first, second, third");
    println!("  Drop order will be: third, second, first (reverse order)");
    println!("  Drops happening now:");

    // _c, _b, _a will be dropped in reverse order when this function returns.
    // This is deterministic and visible in the MIR:
    //   drop(_c)
    //   drop(_b)
    //   drop(_a)
    //
    // Compare with Java: finalize() order is unpredictable and not guaranteed.
    // Compare with C: no destructors — you must call free() manually.
    // Compare with C++: same RAII pattern — destructors in reverse order.
}

/// Demonstrates that ownership checks are ERASED in the final binary.
///
/// This function does a series of moves and borrows. If you look at the
/// LLVM IR output, you'll see that none of the ownership annotations
/// survive — it's just loads, stores, and function calls.
#[no_mangle]
#[inline(never)]
pub fn ownership_erased_in_binary(input: &str) -> String {
    // MIR shows: _1 = &_input (borrow)
    // LLVM IR shows: just a pointer parameter

    let mut result = String::from(input);  // Allocation
    // MIR shows: _result = String::from(move _input)
    // LLVM IR shows: call @alloc::string::String::from(ptr, len)

    result.push_str("!!!");  // Mutation through &mut
    // MIR shows: &mut _result used
    // LLVM IR shows: call @alloc::string::String::push_str(ptr, "!!!")

    result  // Move out of function (return)
    // MIR shows: _0 = move _result
    // LLVM IR shows: return value via pointer (sret)
}

fn main() {
    println!("=== Module 7: Ownership at the Compiler Level ===");
    println!();

    demonstrate_moves();
    demonstrate_shared_borrows();
    demonstrate_mutable_borrows();
    demonstrate_nll();
    demonstrate_drop_order();
    println!();

    // Call the erased function to demonstrate it works
    let result = ownership_erased_in_binary("hello");
    println!("--- Ownership Erased in Binary ---");
    println!("  ownership_erased_in_binary(\"hello\") = {:?}", result);
    println!();

    println!("EXERCISES:");
    println!("  1. View the MIR to see ownership tracking:");
    println!("       rustc --emit=mir ownership_demo.rs");
    println!("     Look for: move, &, &mut, drop, StorageLive/Dead");
    println!();
    println!("  2. View the LLVM IR to see ownership ERASED:");
    println!("       rustc --emit=llvm-ir ownership_demo.rs");
    println!("     No borrow annotations — just loads, stores, calls.");
    println!();
    println!("  3. Compare the MIR and LLVM IR for demonstrate_moves().");
    println!("     In MIR: explicit move semantics and drop points.");
    println!("     In LLVM IR: just pointer copies and a single free call.");
    println!();
    println!("  KEY TAKEAWAY:");
    println!("    Rust's safety guarantees cost NOTHING at runtime.");
    println!("    The borrow checker works on MIR, before LLVM ever sees the code.");
    println!("    The final binary is identical to what you'd write in C.");
}
