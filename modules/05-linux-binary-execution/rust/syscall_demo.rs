// syscall_demo.rs — Making Linux system calls from Rust
// =====================================================
//
// Module 5: How Linux Runs a Binary
//
// WHAT THIS DEMONSTRATES:
//   Rust's standard library (std) sits on top of libc, which sits on top
//   of raw syscalls — exactly like C. The layers are:
//
//     Rust code:    std::io::Write / println!()
//                        |
//     Rust std:     Calls libc::write() (via FFI)
//                        |
//     libc:         Puts args in registers, executes 'syscall'
//                        |
//     Kernel:       sys_write() handles the I/O
//
//   This program demonstrates both:
//     1. Direct syscall via inline asm (unsafe, platform-specific)
//     2. Safe std::io::Write (the normal Rust way)
//
// IMPORTANT: inline asm with syscalls is inherently UNSAFE in Rust.
// This is intentional — Rust makes you explicitly opt into dangerous
// operations. The safe abstractions in std are what you should use
// in production code.
//
// TO BUILD AND RUN:
//   rustc -o syscall_demo syscall_demo.rs
//   ./syscall_demo
//
// TRY ALSO:
//   strace ./syscall_demo
//   # You'll see the same write() syscalls whether from asm or from std
//
// NOTE: Requires nightly Rust for inline asm, or Rust 1.59+ for stable asm!

use std::io::Write;

/// Raw write syscall using inline assembly.
///
/// This is the Rust equivalent of the C version — we directly invoke
/// the 'syscall' instruction with SYS_write (number 1).
///
/// SAFETY: This is unsafe because:
///   1. We're executing arbitrary assembly instructions
///   2. We're making promises to the compiler about register usage
///   3. A wrong syscall number or bad pointer would cause undefined behavior
///
/// In production Rust, you would NEVER do this. Use std::io instead.
#[allow(unused)]
unsafe fn raw_write(fd: u64, buf: *const u8, count: u64) -> i64 {
    let ret: i64;

    // Rust's asm! macro syntax:
    //   "syscall"     — the instruction
    //   in("rax")     — input: set %rax to SYS_write (1)
    //   in("rdi")     — input: set %rdi to fd
    //   in("rsi")     — input: set %rsi to buf pointer
    //   in("rdx")     — input: set %rdx to count
    //   lateout("rax") — output: return value comes back in %rax
    //   out("rcx")    — clobbered by syscall
    //   out("r11")    — clobbered by syscall
    //
    // The syscall instruction always clobbers rcx and r11.
    // rcx gets the return address, r11 gets the saved flags.
    std::arch::asm!(
        "syscall",
        in("rax") 1_u64,       // SYS_write = 1
        in("rdi") fd,          // file descriptor
        in("rsi") buf,         // buffer pointer
        in("rdx") count,       // byte count
        lateout("rax") ret,    // return value
        out("rcx") _,          // clobbered by syscall
        out("r11") _,          // clobbered by syscall
        options(nostack)
    );

    ret
}

/// Raw exit syscall using inline assembly.
///
/// SYS_exit = 60 on x86_64.
/// This immediately terminates the process without running destructors,
/// flushing buffers, or any other cleanup.
#[allow(unused)]
unsafe fn raw_exit(status: u64) -> ! {
    std::arch::asm!(
        "syscall",
        in("rax") 60_u64,      // SYS_exit = 60
        in("rdi") status,       // exit status
        // NOTE: 'noreturn' asm blocks cannot have out() clobbers.
        // Since this never returns, clobbered registers don't matter.
        options(noreturn, nostack)
    );
}

fn main() {
    println!("=== Rust Syscall Demonstration ===\n");

    // ---------------------------------------------------------------
    // APPROACH 1: Direct syscall via inline assembly (unsafe)
    //
    // This bypasses Rust's entire safety system. We're talking directly
    // to the kernel, just like the C version.
    // ---------------------------------------------------------------
    println!("--- Approach 1: Direct syscall via inline asm ---");

    let msg = b"[raw syscall]  Hello from a direct write() syscall in Rust!\n";

    // SAFETY: We're passing a valid pointer and length to SYS_write.
    // fd=1 is stdout. This is safe as long as the buffer is valid.
    let bytes_written = unsafe {
        raw_write(1, msg.as_ptr(), msg.len() as u64)
    };
    println!("  -> raw_write returned: {} bytes\n", bytes_written);

    // ---------------------------------------------------------------
    // APPROACH 2: Safe Rust std::io (the correct way)
    //
    // std::io::stdout() returns a handle that implements Write.
    // Under the hood, this goes through:
    //   Write::write() -> sys::io::write() -> libc::write() -> syscall
    //
    // But from Rust's perspective, it's completely safe — no undefined
    // behavior possible, buffer lifetimes are checked at compile time.
    // ---------------------------------------------------------------
    println!("--- Approach 2: Safe Rust std::io::Write ---");

    let mut stdout = std::io::stdout().lock();
    let msg2 = b"[std::io]      Hello from Rust's safe Write trait!\n";
    match stdout.write_all(msg2) {
        Ok(()) => println!("  -> write_all succeeded\n"),
        Err(e) => eprintln!("  -> write_all failed: {}\n", e),
    }
    drop(stdout); // Release the lock

    // ---------------------------------------------------------------
    // APPROACH 3: println! macro (the idiomatic way)
    //
    // println! is a macro that:
    //   1. Formats the string (like format!())
    //   2. Locks stdout
    //   3. Writes the formatted string + newline
    //   4. Unlocks stdout
    //
    // It panics on write failure (unlike write_all which returns Result).
    // ---------------------------------------------------------------
    println!("--- Approach 3: println! macro (idiomatic Rust) ---");
    println!("[println!]     Hello from Rust's println! macro!");
    println!("  -> println! is the most common way to write to stdout\n");

    // ---------------------------------------------------------------
    // APPROACH 4: Write to stderr via raw syscall
    // ---------------------------------------------------------------
    println!("--- Approach 4: Raw syscall to stderr (fd=2) ---");
    let stderr_msg = b"[raw syscall]  This goes to stderr (fd=2)!\n";
    unsafe {
        raw_write(2, stderr_msg.as_ptr(), stderr_msg.len() as u64);
    }
    println!();

    // ---------------------------------------------------------------
    // Explanation
    // ---------------------------------------------------------------
    println!("=== KEY TAKEAWAYS ===");
    println!("1. Rust's std::io wraps libc, which wraps raw syscalls.");
    println!("   The layers are: println! -> std::io -> libc::write -> syscall");
    println!();
    println!("2. Direct syscalls in Rust require 'unsafe' — Rust makes you");
    println!("   explicitly acknowledge when you're doing something dangerous.");
    println!();
    println!("3. The safe abstractions (std::io) are what you should use.");
    println!("   They handle errors properly and work cross-platform.");
    println!();
    println!("4. At the kernel level, all three approaches produce the same");
    println!("   write() syscall. Run 'strace' to verify.");
    println!();

    // ---------------------------------------------------------------
    // Demonstrate raw_exit (optional — uncomment to see it)
    //
    // NOTE: If you uncomment this, Rust's Drop handlers and println
    // flush will NOT happen. This goes directly to the kernel.
    // ---------------------------------------------------------------
    println!("Uncomment the raw_exit() call in the source to see direct process exit.");
    println!("It bypasses Rust's cleanup, just like C's _exit().\n");

    // unsafe { raw_exit(0); }
    // println!("You'd never see this if raw_exit were uncommented!");
}
