/*
 * syscall_direct.c — Making Linux system calls directly
 * =====================================================
 *
 * Module 5: How Linux Runs a Binary
 *
 * WHAT THIS DEMONSTRATES:
 *   When your C program calls write(), here's what ACTUALLY happens:
 *
 *     Your code:   write(fd, buf, len)
 *                       |
 *     libc:        Puts args in registers, executes 'syscall' instruction
 *                       |
 *     CPU:         Switches to kernel mode (ring 0)
 *                       |
 *     Kernel:      sys_write() handles the actual I/O
 *                       |
 *     CPU:         Returns to user mode (ring 3), result in %rax
 *
 *   This program shows you BOTH paths:
 *     1. Using libc's write() wrapper (the normal way)
 *     2. Using inline assembly to execute 'syscall' directly (what libc does internally)
 *
 *   They produce identical results because libc's write() is just a thin
 *   wrapper around the exact same syscall instruction.
 *
 * LINUX x86_64 SYSCALL CONVENTION:
 *   - Syscall number goes in %rax
 *   - Arguments go in: %rdi, %rsi, %rdx, %r10, %r8, %r9 (up to 6 args)
 *   - Return value comes back in %rax
 *   - The 'syscall' instruction triggers the kernel transition
 *
 * KEY SYSCALL NUMBERS (x86_64):
 *   0 = read      1 = write     2 = open      3 = close
 *   9 = mmap     11 = munmap   57 = fork     59 = execve
 *  60 = exit     62 = kill    231 = exit_group
 *
 * TO BUILD AND RUN:
 *   gcc -Wall -Wextra -o syscall_direct syscall_direct.c
 *   ./syscall_direct
 *
 * TRY ALSO:
 *   strace ./syscall_direct
 *   # You'll see the write() and exit_group() syscalls regardless of
 *   # whether we used libc or inline assembly — the kernel sees the same thing!
 *
 * COMPILE FOR x86_64 LINUX ONLY — syscall numbers are architecture-specific.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* -----------------------------------------------------------------------
 * APPROACH 1: Direct syscall via inline assembly
 *
 * This is what libc does internally (simplified). We put the syscall
 * number in %rax and the arguments in %rdi, %rsi, %rdx, then execute
 * the 'syscall' instruction.
 *
 * The 'syscall' instruction is the gateway from user space to kernel space.
 * When the CPU executes it:
 *   1. Saves the return address (RIP) into RCX
 *   2. Saves RFLAGS into R11
 *   3. Loads the kernel's code segment and instruction pointer
 *   4. The kernel's syscall handler dispatches based on %rax
 *   5. When done, 'sysret' returns to user space
 * ----------------------------------------------------------------------- */

/*
 * raw_write() — Write to a file descriptor using a direct syscall.
 *
 * Parameters are identical to POSIX write():
 *   fd    — file descriptor (1 = stdout, 2 = stderr)
 *   buf   — pointer to data to write
 *   count — number of bytes to write
 *
 * Returns: number of bytes written, or negative errno on error
 */
static long raw_write(int fd, const void *buf, unsigned long count)
{
    long ret;

    /*
     * Inline assembly breakdown:
     *   "syscall"              — the instruction that traps into the kernel
     *   "=a" (ret)             — output: %rax → ret (return value)
     *   "a" (1)                — input: %rax = 1 (SYS_write)
     *   "D" (fd)               — input: %rdi = fd (first argument)
     *   "S" (buf)              — input: %rsi = buf (second argument)
     *   "d" (count)            — input: %rdx = count (third argument)
     *   "rcx", "r11", "memory" — clobbered by syscall instruction
     *
     * NOTE: The 'syscall' instruction always clobbers %rcx and %r11.
     * %rcx gets the return address (RIP), %r11 gets RFLAGS.
     */
    __asm__ volatile (
        "syscall"
        : "=a" (ret)                           /* output: return value in %rax */
        : "a"  ((long)1),                      /* %rax = 1 (SYS_write)        */
          "D"  ((long)fd),                     /* %rdi = file descriptor       */
          "S"  (buf),                          /* %rsi = buffer pointer        */
          "d"  (count)                         /* %rdx = byte count            */
        : "rcx", "r11", "memory"               /* clobbered registers          */
    );

    return ret;
}

/*
 * raw_exit() — Terminate the process using a direct syscall.
 *
 * SYS_exit = 60 on x86_64.
 *
 * NOTE: This is _exit(), not exit(). It does NOT flush stdio buffers
 * or run atexit handlers. The kernel simply destroys the process.
 */
static void raw_exit(int status)
{
    __asm__ volatile (
        "syscall"
        :                                      /* no outputs (we never return) */
        : "a"  ((long)60),                     /* %rax = 60 (SYS_exit)        */
          "D"  ((long)status)                  /* %rdi = exit status           */
        : "rcx", "r11", "memory"
    );

    /*
     * We should never reach here. The __builtin_unreachable() tells the
     * compiler this code path is impossible, suppressing "function might
     * return" warnings.
     */
    __builtin_unreachable();
}

/* -----------------------------------------------------------------------
 * APPROACH 2: Using libc's write() wrapper
 *
 * libc's write() does essentially what raw_write() does above, plus:
 *   - Sets errno on failure (our raw version doesn't)
 *   - Handles thread cancellation points
 *   - May have additional safety checks
 *
 * Under the hood, glibc's write() looks something like:
 *
 *   ssize_t write(int fd, const void *buf, size_t count) {
 *       long ret;
 *       SYSCALL_CANCEL(ret, SYS_write, fd, buf, count);
 *       if (ret < 0) { errno = -ret; return -1; }
 *       return ret;
 *   }
 * ----------------------------------------------------------------------- */

int main(void)
{
    /* === PART 1: Direct syscall via inline assembly === */

    const char msg1[] = "[raw syscall]  Hello from a direct write() syscall!\n";
    long bytes_written = raw_write(1, msg1, strlen(msg1));

    /*
     * We use printf here (libc) to print the result. Notice the irony:
     * we're demonstrating raw syscalls but using libc to format output.
     * That's fine — the point is to understand what's beneath libc.
     */
    printf("  → raw_write returned: %ld bytes\n\n", bytes_written);

    /* === PART 2: libc's write() wrapper === */

    const char msg2[] = "[libc write()] Hello from libc's write() wrapper!\n";
    ssize_t bytes_written2 = write(STDOUT_FILENO, msg2, strlen(msg2));
    printf("  → libc write returned: %zd bytes\n\n", bytes_written2);

    /* === PART 3: Writing to stderr (fd=2) via raw syscall === */

    const char msg3[] = "[raw syscall]  This goes to stderr (fd=2)!\n";
    raw_write(2, msg3, strlen(msg3));

    /* === EXPLANATION === */

    printf("=== KEY TAKEAWAY ===\n");
    printf("Both approaches produce IDENTICAL results at the kernel level.\n");
    printf("Run 'strace ./syscall_direct' to verify — you'll see the same\n");
    printf("write(1, \"...\", N) syscall regardless of which path we used.\n");
    printf("\n");
    printf("The call chain:\n");
    printf("  Your code → libc write() → syscall instruction → kernel sys_write()\n");
    printf("  Your code → inline asm   → syscall instruction → kernel sys_write()\n");
    printf("\n");
    printf("libc is just a convenience layer. The kernel doesn't know or care\n");
    printf("whether you called its syscall through libc or directly.\n");
    printf("\n");

    /* === PART 4: Demonstrate raw _exit === */

    printf("Now calling raw_exit(0) — this bypasses libc's exit().\n");
    printf("atexit() handlers and stdio flushing will NOT happen.\n");
    printf("(But we already flushed stdout with each printf's \\n.)\n");

    /*
     * OBSERVE: raw_exit() calls SYS_exit (60) directly.
     * If we had registered atexit() handlers, they would NOT run.
     * If we had unflushed stdio buffers, they would be LOST.
     *
     * This is the difference between:
     *   exit(0)     — libc function: flushes buffers, runs atexit, then SYS_exit_group
     *   _exit(0)    — libc function: just calls SYS_exit_group
     *   raw_exit(0) — our function: just calls SYS_exit (single thread version)
     */
    raw_exit(0);

    /* This line is never reached */
    printf("YOU SHOULD NEVER SEE THIS\n");
    return 1;
}
