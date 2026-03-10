// =============================================================================
// Module 7: Rust / LLVM Deep Dive — #![no_std] Bare-Metal Example
// =============================================================================
//
// CONCEPT: This is Rust WITHOUT the standard library. No heap allocator,
// no filesystem, no networking, no threads — just raw system calls.
//
// This is how Rust is used in:
//   - Operating system kernels (Linux kernel Rust modules)
//   - Embedded systems (microcontrollers with no OS)
//   - Bootloaders
//   - WebAssembly modules
//   - Any environment where libc isn't available
//
// WHAT #![no_std] MEANS:
//   - The `std` crate is NOT linked. This removes:
//       - String, Vec, HashMap (they need a heap allocator)
//       - println! (it needs stdout, which is an OS concept)
//       - File I/O, networking, threads
//       - The default panic handler (which prints to stderr)
//
//   - The `core` crate IS still available. This provides:
//       - Primitive types (i32, bool, etc.)
//       - Traits (Copy, Clone, Iterator, etc.)
//       - Option, Result
//       - Slices, arrays
//       - Inline assembly (core::arch::asm!)
//       - Intrinsics
//
// WHAT #![no_main] MEANS:
//   - Rust's normal entry point is main(), but that requires the Rust
//     runtime (which sets up a panic handler, initializes args, etc.)
//   - With #![no_main], we define our own entry point: _start
//   - _start is the same entry point that C programs use (it's what the
//     kernel jumps to after loading the ELF binary)
//
// HOW THIS COMPILES:
//   rustc --edition 2021 -C link-arg=-nostartfiles -C panic=abort \
//         no_std_demo.rs -o no_std_demo
//
//   Flags explained:
//     -C link-arg=-nostartfiles  Don't link crt0/crt1 (C runtime startup)
//     -C panic=abort             Don't include unwinding machinery
//
//   The resulting binary is TINY compared to a normal Rust binary:
//     Normal Rust hello:  ~4 MB (includes std, panic handling, etc.)
//     This no_std binary: ~16 KB (just our code + ELF headers)
//
// THIS IS THE SAME LEVEL AS:
//   - A C program compiled with -nostdlib -nostartfiles
//   - An assembly language program
//   - The first instructions a CPU runs after power-on
//
// =============================================================================

// Disable the standard library — we're going bare metal.
#![no_std]

// Disable the normal Rust main() entry point.
// We'll define _start ourselves, just like a C program without crt0.
#![no_main]

// =============================================================================
// PANIC HANDLER
// =============================================================================
// Without std, there's no default panic handler. We MUST provide one.
// In a real embedded system, this might blink an LED or reset the device.
// Here, we just make a sys_exit syscall with error code 1.

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    // In a real embedded system, you might:
    //   - Write to a UART debug port
    //   - Blink an error LED pattern
    //   - Log to flash memory for post-mortem
    //   - Trigger a watchdog reset
    //
    // We just exit with an error code.
    // The `!` return type means this function never returns.
    unsafe {
        sys_exit(1);
    }
}

// =============================================================================
// RAW SYSTEM CALLS (x86-64 Linux)
// =============================================================================
// Without libc, we talk directly to the kernel using the syscall instruction.
//
// x86-64 Linux syscall convention:
//   - Syscall number in rax
//   - Arguments in: rdi, rsi, rdx, r10, r8, r9
//   - Return value in: rax
//   - The `syscall` instruction traps into the kernel
//
// This is the LOWEST level of user-space programming:
//   Your code → syscall instruction → kernel handles it → returns to your code
//
// No libc wrapper. No runtime. Just you and the kernel.

/// sys_write: Write bytes to a file descriptor.
///
/// Equivalent to the C function write(fd, buf, count), but we invoke
/// the syscall directly without going through libc.
///
/// Syscall number 1 = SYS_write
///   rdi = fd (file descriptor: 1 = stdout, 2 = stderr)
///   rsi = buf (pointer to the byte buffer)
///   rdx = count (number of bytes to write)
///   returns: number of bytes written (or negative errno)
#[inline(always)]
unsafe fn sys_write(fd: u64, buf: *const u8, count: u64) -> i64 {
    let ret: i64;
    core::arch::asm!(
        "syscall",
        // Syscall number goes in rax. SYS_write = 1.
        in("rax") 1_u64,
        // First argument: file descriptor
        in("rdi") fd,
        // Second argument: pointer to buffer
        in("rsi") buf,
        // Third argument: byte count
        in("rdx") count,
        // The syscall instruction clobbers rcx and r11
        // (the kernel uses them to save rip and rflags)
        out("rcx") _,
        out("r11") _,
        // Return value comes back in rax
        lateout("rax") ret,
    );
    ret
}

/// sys_exit: Terminate the process.
///
/// Syscall number 60 = SYS_exit
///   rdi = exit code (0 = success, non-zero = failure)
///   Does not return.
#[inline(always)]
unsafe fn sys_exit(code: u64) -> ! {
    core::arch::asm!(
        "syscall",
        in("rax") 60_u64,   // SYS_exit = 60
        in("rdi") code,      // Exit code
        // This syscall never returns. The `noreturn` option tells the
        // compiler that execution does not continue past this point.
        // With `noreturn`, we cannot specify output clobbers — the compiler
        // knows there are no outputs because control never returns.
        options(noreturn),
    );
}

// =============================================================================
// COMPILER INTRINSICS
// =============================================================================
// LLVM may emit calls to memset/memcpy/memcmp for array initialization and
// struct copies. Without libc, we must provide these ourselves.
// These are minimal implementations — just enough for the compiler to link.

#[no_mangle]
pub unsafe extern "C" fn memset(dest: *mut u8, c: i32, n: usize) -> *mut u8 {
    let mut i = 0;
    while i < n {
        *dest.add(i) = c as u8;
        i += 1;
    }
    dest
}

#[no_mangle]
pub unsafe extern "C" fn memcpy(dest: *mut u8, src: *const u8, n: usize) -> *mut u8 {
    let mut i = 0;
    while i < n {
        *dest.add(i) = *src.add(i);
        i += 1;
    }
    dest
}

#[no_mangle]
pub unsafe extern "C" fn memcmp(s1: *const u8, s2: *const u8, n: usize) -> i32 {
    let mut i = 0;
    while i < n {
        let a = *s1.add(i);
        let b = *s2.add(i);
        if a != b {
            return a as i32 - b as i32;
        }
        i += 1;
    }
    0
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================
// Without std, we don't have println! or any string formatting.
// We must write our own helpers that work with raw byte slices.

/// Write a string to stdout using our raw sys_write.
#[inline(always)]
unsafe fn write_stdout(msg: &[u8]) {
    sys_write(1, msg.as_ptr(), msg.len() as u64);
}

/// Convert a u64 to decimal ASCII digits and write to stdout.
/// This is a minimal itoa implementation — no heap allocation needed.
unsafe fn write_number(mut n: u64) {
    // Maximum digits in a u64: 20 (18446744073709551615)
    // We build the string backwards in a stack buffer.
    let mut buf = [0u8; 20];
    let mut pos = 20;

    if n == 0 {
        write_stdout(b"0");
        return;
    }

    while n > 0 {
        pos -= 1;
        buf[pos] = b'0' + (n % 10) as u8;
        n /= 10;
    }

    // Write only the digits we produced (from pos to 20).
    let digits = &buf[pos..20];
    sys_write(1, digits.as_ptr(), digits.len() as u64);
}

// =============================================================================
// ENTRY POINT
// =============================================================================
// This is where the kernel jumps after loading our ELF binary.
// In a normal C program, this would be in crt1.o, which calls main().
// In a normal Rust program, the runtime sets up panic hooks and calls main().
// Here, WE are the entry point. Nothing runs before us.

/// The program entry point — called directly by the Linux kernel.
///
/// The `#[no_mangle]` attribute ensures the symbol name is exactly `_start`
/// (not a mangled Rust name like `_ZN11no_std_demo5_start...`).
///
/// The `extern "C"` ensures C calling convention (which the kernel expects).
///
/// The `-> !` means this function never returns — it must call sys_exit.
/// If _start returns, the CPU would try to execute whatever garbage is
/// on the stack as code, causing a segfault.
#[no_mangle]
pub extern "C" fn _start() -> ! {
    unsafe {
        // =====================================================================
        // Write a greeting — directly to stdout via syscall
        // =====================================================================
        write_stdout(b"=== Module 7: no_std Bare-Metal Rust ===\n");
        write_stdout(b"\n");
        write_stdout(b"This program has:\n");
        write_stdout(b"  - No standard library (no std)\n");
        write_stdout(b"  - No C runtime (no crt0, no libc)\n");
        write_stdout(b"  - No heap allocator (no malloc/free)\n");
        write_stdout(b"  - No panic unwinding\n");
        write_stdout(b"  - No println! macro\n");
        write_stdout(b"\n");
        write_stdout(b"It communicates with the kernel using raw syscalls:\n");
        write_stdout(b"  - sys_write (syscall 1) to write to stdout\n");
        write_stdout(b"  - sys_exit (syscall 60) to terminate\n");
        write_stdout(b"\n");

        // =====================================================================
        // Demonstrate that we can still do computation
        // =====================================================================
        write_stdout(b"Computing sum of 1..=100: ");

        // We can use normal Rust code — loops, arithmetic, etc.
        // We just can't use anything that requires allocation or OS abstractions.
        let mut sum: u64 = 0;
        let mut i: u64 = 1;
        while i <= 100 {
            sum += i;
            i += 1;
        }
        write_number(sum);
        write_stdout(b"\n");
        write_stdout(b"Expected: 5050\n");
        write_stdout(b"\n");

        // =====================================================================
        // Show the stack — we're running on the kernel-provided stack
        // =====================================================================
        write_stdout(b"Stack pointer (approximate): 0x");

        // Read the stack pointer using inline assembly.
        let sp: u64;
        core::arch::asm!(
            "mov {}, rsp",
            out(reg) sp,
        );

        // Print as hex (minimal hex printer)
        let hex_chars = b"0123456789abcdef";
        let mut hex_buf = [0u8; 16];
        let mut val = sp;
        for byte in hex_buf.iter_mut().rev() {
            *byte = hex_chars[(val & 0xf) as usize];
            val >>= 4;
        }
        sys_write(1, hex_buf.as_ptr(), 16);
        write_stdout(b"\n\n");

        // =====================================================================
        // Summary
        // =====================================================================
        write_stdout(b"WHAT THIS DEMONSTRATES:\n");
        write_stdout(b"  The Rust compiler can produce a binary that is as\n");
        write_stdout(b"  minimal as hand-written assembly. No runtime overhead,\n");
        write_stdout(b"  no hidden allocations, no background threads.\n");
        write_stdout(b"\n");
        write_stdout(b"  This is the same level that OS kernels and bootloaders\n");
        write_stdout(b"  operate at. The Linux kernel itself includes Rust code\n");
        write_stdout(b"  compiled in this mode.\n");
        write_stdout(b"\n");
        write_stdout(b"  Compare the binary size:\n");
        write_stdout(b"    Normal Rust hello:  ~4 MB (includes std, unwinding)\n");
        write_stdout(b"    This no_std binary: ~16 KB (just code + ELF headers)\n");
        write_stdout(b"\n");

        // =====================================================================
        // Exit cleanly
        // =====================================================================
        // We MUST call sys_exit. If _start returns, the CPU would execute
        // garbage on the stack, causing a segfault.
        sys_exit(0);
    }
}
