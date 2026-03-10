// =============================================================================
// Module 7: Rust / LLVM Deep Dive — Understanding LLVM IR
// =============================================================================
//
// CONCEPT: LLVM IR (Intermediate Representation) is the language that sits
// between Rust (or C/C++/Swift) and machine code. Both rustc and clang
// compile to LLVM IR, then LLVM's backend optimizes it and generates
// native assembly for the target architecture.
//
// This file is designed to produce INTERESTING, READABLE LLVM IR. Each
// function demonstrates specific LLVM IR concepts.
//
// LLVM IR KEY CONCEPTS:
//
//   1. SSA FORM (Static Single Assignment)
//      Every variable is assigned exactly ONCE. Instead of:
//        x = 1; x = x + 1; x = x * 2;
//      SSA uses:
//        %x.0 = 1
//        %x.1 = add %x.0, 1
//        %x.2 = mul %x.1, 2
//      This makes optimization easier because each "variable" has exactly
//      one definition point.
//
//   2. BASIC BLOCKS
//      Code is organized into basic blocks — sequences of instructions with
//      no branches except at the end. Each block has a label:
//        entry:
//          %x = add i32 1, 2
//          br label %next        ; unconditional branch
//        next:
//          ret i32 %x            ; return
//
//   3. TYPE ANNOTATIONS
//      Every value has an explicit type:
//        i1    = 1-bit integer (boolean)
//        i8    = 8-bit integer (byte/char)
//        i32   = 32-bit integer
//        i64   = 64-bit integer
//        float = 32-bit float
//        double = 64-bit float
//        ptr   = pointer (opaque in modern LLVM)
//
//   4. INSTRUCTIONS
//      Arithmetic:  add, sub, mul, sdiv (signed), udiv (unsigned)
//      Float:       fadd, fsub, fmul, fdiv
//      Comparison:  icmp eq/ne/slt/sgt/..., fcmp oeq/one/...
//      Branching:   br (conditional and unconditional)
//      Memory:      alloca (stack), load, store
//      Calls:       call @function_name(args...)
//      Returns:     ret type value
//      Phi:         phi [val1, block1], [val2, block2]  (SSA merging)
//
//   5. PHI NODES
//      When two basic blocks merge (e.g., after an if/else), SSA needs a way
//      to select which value to use. That's a phi node:
//        %result = phi i32 [%val_true, %then_block], [%val_false, %else_block]
//      "If we came from then_block, use val_true; if from else_block, use val_false."
//
// TO EXPLORE:
//   1. Generate LLVM IR:
//        rustc --emit=llvm-ir llvm_ir_demo.rs -o llvm_ir_demo.ll
//
//   2. Generate OPTIMIZED LLVM IR (-O2):
//        rustc --emit=llvm-ir -C opt-level=2 llvm_ir_demo.rs -o llvm_ir_demo_opt.ll
//
//   3. Compare the unoptimized and optimized versions:
//        diff llvm_ir_demo.ll llvm_ir_demo_opt.ll
//
//      The -O2 version will be dramatically smaller — the optimizer has:
//        - Inlined function calls
//        - Eliminated dead code
//        - Simplified branches
//        - Replaced loops with closed-form expressions
//        - Removed unnecessary alloca/load/store
//
// =============================================================================

/// Demonstrates basic arithmetic and SSA form.
///
/// In LLVM IR (unoptimized), this becomes:
///   define i32 @arithmetic_ssa(i32 %a, i32 %b) {
///   entry:
///     %a.addr = alloca i32        ; stack slot for parameter a
///     %b.addr = alloca i32        ; stack slot for parameter b
///     store i32 %a, ptr %a.addr   ; store a to stack
///     store i32 %b, ptr %b.addr   ; store b to stack
///     %0 = load i32, ptr %a.addr  ; reload a
///     %1 = load i32, ptr %b.addr  ; reload b
///     %2 = add i32 %0, %1         ; sum = a + b
///     %3 = load i32, ptr %a.addr  ; reload a
///     %4 = load i32, ptr %b.addr  ; reload b
///     %5 = sub i32 %3, %4         ; diff = a - b
///     %6 = mul i32 %2, %5         ; result = sum * diff
///     ret i32 %6                   ; return result
///   }
///
/// At -O2, the optimizer removes the alloca/load/store and simplifies to:
///   define i32 @arithmetic_ssa(i32 %a, i32 %b) {
///     %sum = add i32 %a, %b
///     %diff = sub i32 %a, %b
///     %result = mul i32 %sum, %diff
///     ret i32 %result
///   }
///
/// Notice SSA: each %variable is assigned exactly once.
#[no_mangle]
#[inline(never)]
pub fn arithmetic_ssa(a: i32, b: i32) -> i32 {
    let sum = a + b;        // One SSA variable: %sum
    let diff = a - b;       // Another SSA variable: %diff
    let result = sum * diff; // Another SSA variable: %result
    result                  // ret i32 %result
}

/// Demonstrates branches and basic blocks.
///
/// This function has a conditional (if/else), which creates multiple
/// basic blocks in LLVM IR:
///
///   define i32 @branching(i32 %x) {
///   entry:
///     %cmp = icmp sgt i32 %x, 0          ; x > 0 ? (signed greater than)
///     br i1 %cmp, label %then, label %else  ; conditional branch
///
///   then:                                  ; basic block for the true branch
///     %pos = mul i32 %x, %x               ; x * x
///     br label %merge                      ; jump to merge point
///
///   else:                                  ; basic block for the false branch
///     %neg = sub i32 0, %x                 ; -x (negate)
///     br label %merge                      ; jump to merge point
///
///   merge:                                 ; PHI node merges the two paths
///     %result = phi i32 [%pos, %then], [%neg, %else]
///     ret i32 %result
///   }
///
/// The phi node at `merge` is the key SSA concept: it selects the right
/// value depending on which basic block we came from.
#[no_mangle]
#[inline(never)]
pub fn branching(x: i32) -> i32 {
    if x > 0 {
        x * x       // Square positive numbers
    } else {
        -x           // Negate non-positive numbers (absolute value)
    }
}

/// Demonstrates a loop in LLVM IR.
///
/// Loops create a "back edge" in the control flow graph — a branch that
/// goes backwards to a previously-seen basic block.
///
///   define i64 @loop_sum(i64 %n) {
///   entry:
///     br label %loop_header
///
///   loop_header:                           ; loop entry point
///     %i = phi i64 [0, %entry], [%i_next, %loop_body]
///     %sum = phi i64 [0, %entry], [%sum_next, %loop_body]
///     %cmp = icmp slt i64 %i, %n           ; i < n ?
///     br i1 %cmp, label %loop_body, label %exit
///
///   loop_body:
///     %i_next = add i64 %i, 1              ; i++
///     %sum_next = add i64 %sum, %i         ; sum += i
///     br label %loop_header                ; back edge (LOOP!)
///
///   exit:
///     ret i64 %sum
///   }
///
/// Notice the phi nodes at loop_header: they merge values from the initial
/// entry (i=0, sum=0) with the values from the loop body (i+1, sum+i).
///
/// At -O2, LLVM may replace this entire loop with a closed-form expression:
///   n * (n - 1) / 2
/// This is called "scalar evolution" — the optimizer recognizes the induction
/// variable pattern and computes the result directly.
#[no_mangle]
#[inline(never)]
pub fn loop_sum(n: i64) -> i64 {
    let mut sum: i64 = 0;
    let mut i: i64 = 0;
    while i < n {
        sum += i;
        i += 1;
    }
    sum
}

/// Demonstrates alloca (stack allocation) in LLVM IR.
///
/// In unoptimized LLVM IR, every local variable gets an alloca:
///   %arr = alloca [5 x i32]      ; allocate 5 i32s on the stack
///
/// The optimizer (mem2reg pass) promotes most allocas to SSA registers,
/// but arrays and structs that have their address taken keep their allocas.
#[no_mangle]
#[inline(never)]
pub fn stack_allocation() -> i32 {
    // In LLVM IR, this becomes: %arr = alloca [5 x i32]
    let arr = [10, 20, 30, 40, 50];

    // Array access becomes getelementptr + load:
    //   %ptr = getelementptr [5 x i32], ptr %arr, i64 0, i64 2
    //   %val = load i32, ptr %ptr
    let middle = arr[2];

    // Compute a result using array values
    // More loads and arithmetic instructions
    arr[0] + middle + arr[4]  // 10 + 30 + 50 = 90
}

/// Demonstrates function calls in LLVM IR.
///
/// In LLVM IR, a function call is:
///   %result = call i32 @some_function(i32 %arg1, i32 %arg2)
///
/// The calling convention determines how arguments are passed:
///   - First few integer args: in registers (rdi, rsi, rdx, rcx, r8, r9)
///   - First few float args: in xmm0-xmm7
///   - Additional args: on the stack
///   - Return value: in rax (integer) or xmm0 (float)
#[no_mangle]
#[inline(never)]
pub fn callee(x: i32, y: i32) -> i32 {
    x + y
}

#[no_mangle]
#[inline(never)]
pub fn caller_demo() -> i32 {
    // In LLVM IR:
    //   %r1 = call i32 @callee(i32 10, i32 20)
    //   %r2 = call i32 @callee(i32 %r1, i32 5)
    //   ret i32 %r2
    let a = callee(10, 20);     // First call
    let b = callee(a, 5);       // Chain the result
    b
}

/// Demonstrates how structs look in LLVM IR.
///
/// A struct becomes a named type:
///   %Point = type { i32, i32 }
///
/// Field access uses getelementptr:
///   %x_ptr = getelementptr %Point, ptr %p, i32 0, i32 0  ; field 0 (x)
///   %y_ptr = getelementptr %Point, ptr %p, i32 0, i32 1  ; field 1 (y)
struct Point {
    x: i32,
    y: i32,
}

#[no_mangle]
#[inline(never)]
pub fn struct_demo() -> i32 {
    // In LLVM IR: %p = alloca %Point
    let p = Point { x: 3, y: 4 };

    // Field access through getelementptr:
    //   %x = getelementptr %Point, ptr %p, i32 0, i32 0
    //   %y = getelementptr %Point, ptr %p, i32 0, i32 1
    //   %xval = load i32, ptr %x
    //   %yval = load i32, ptr %y
    //   %x2 = mul i32 %xval, %xval
    //   %y2 = mul i32 %yval, %yval
    //   %sum = add i32 %x2, %y2
    p.x * p.x + p.y * p.y  // Distance squared: 9 + 16 = 25
}

/// Demonstrates how match (switch) compiles to LLVM IR.
///
/// A match with integer patterns becomes a switch instruction:
///   switch i32 %val, label %default [
///     i32 1, label %case_1
///     i32 2, label %case_2
///     i32 3, label %case_3
///   ]
///
/// This is more efficient than a chain of if/else comparisons because
/// LLVM can optimize it into a jump table.
#[no_mangle]
#[inline(never)]
pub fn match_demo(val: i32) -> &'static str {
    match val {
        1 => "one",
        2 => "two",
        3 => "three",
        4..=10 => "four to ten",
        _ => "other",
    }
}

fn main() {
    println!("=== Module 7: Understanding LLVM IR ===");
    println!();

    // Call each function so they appear in the LLVM IR output.
    // (Functions that are never called may be optimized away.)

    println!("arithmetic_ssa(7, 3)  = {} (expected: (7+3)*(7-3) = 40)", arithmetic_ssa(7, 3));
    println!("branching(5)          = {} (expected: 5*5 = 25)", branching(5));
    println!("branching(-3)         = {} (expected: 3)", branching(-3));
    println!("loop_sum(10)          = {} (expected: 0+1+...+9 = 45)", loop_sum(10));
    println!("stack_allocation()    = {} (expected: 10+30+50 = 90)", stack_allocation());
    println!("caller_demo()         = {} (expected: callee(callee(10,20), 5) = 35)", caller_demo());
    println!("struct_demo()         = {} (expected: 3*3+4*4 = 25)", struct_demo());
    println!("match_demo(2)         = {:?} (expected: \"two\")", match_demo(2));
    println!("match_demo(7)         = {:?} (expected: \"four to ten\")", match_demo(7));
    println!("match_demo(99)        = {:?} (expected: \"other\")", match_demo(99));
    println!();

    println!("LLVM IR EXPLORATION GUIDE:");
    println!("  1. Generate unoptimized LLVM IR:");
    println!("       rustc --emit=llvm-ir llvm_ir_demo.rs");
    println!();
    println!("  2. Things to look for in the .ll file:");
    println!("     a. SSA form: each %variable assigned exactly once");
    println!("     b. Basic blocks: labels like 'entry:', 'then:', 'else:'");
    println!("     c. alloca: stack allocation for local variables");
    println!("     d. load/store: reading/writing memory");
    println!("     e. br: branches (conditional and unconditional)");
    println!("     f. phi: merging values from different paths");
    println!("     g. call: function calls with typed arguments");
    println!("     h. getelementptr: struct field / array element access");
    println!("     i. switch: compiled match statements");
    println!();
    println!("  3. Generate optimized LLVM IR and compare:");
    println!("       rustc --emit=llvm-ir -C opt-level=2 llvm_ir_demo.rs");
    println!("     The optimizer will:");
    println!("       - Remove alloca/load/store (mem2reg pass)");
    println!("       - Inline small functions");
    println!("       - Simplify branches");
    println!("       - Replace loop_sum with closed-form math");
    println!("       - Constant-fold known values");
}
