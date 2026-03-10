/*
 * =============================================================================
 * ANTI-DEBUGGING TECHNIQUES — Module 10: Reverse Engineering a C Binary
 * =============================================================================
 *
 * PURPOSE:
 *   This program demonstrates common anti-debugging techniques that real-world
 *   software (and malware) uses to detect and evade reverse engineering.
 *
 *   EDUCATIONAL USE ONLY. Understanding these techniques is essential for:
 *   - Security researchers analyzing malware
 *   - CTF (Capture The Flag) competition players
 *   - Software protection engineers
 *   - Anyone doing binary analysis
 *
 * WHAT TO OBSERVE:
 *   1. ptrace(PTRACE_TRACEME) — a classic anti-debug trick
 *   2. /proc/self/status TracerPid check — detects attached debugger
 *   3. Timing-based detection — measures execution time anomalies
 *   4. INT3 detection — catches software breakpoints
 *
 * IMPORTANT: Every technique shown here is TRIVIALLY BYPASSABLE.
 *   - ptrace: LD_PRELOAD a fake ptrace, or patch the call
 *   - TracerPid: LD_PRELOAD open/read, or modify /proc
 *   - Timing: set breakpoint AFTER the timing check
 *   - All of them: binary patch (NOP out the check)
 *
 *   The point is education, not security. No anti-debug technique is
 *   unbreakable — it just raises the bar for casual analysis.
 *
 * RUN:
 *   gcc -Wall -Wextra -o anti_debug anti_debug.c
 *   ./anti_debug              # runs normally
 *   gdb ./anti_debug          # gets detected!
 *
 * BYPASS:
 *   # Method 1: LD_PRELOAD a ptrace stub
 *   echo 'long ptrace(int r, ...) { return 0; }' > /tmp/fakeptrace.c
 *   gcc -shared -o /tmp/fakeptrace.so /tmp/fakeptrace.c
 *   LD_PRELOAD=/tmp/fakeptrace.so gdb ./anti_debug
 *
 *   # Method 2: Patch the binary (NOP out the ptrace call)
 *   # Find the ptrace callsite in objdump and replace with NOPs
 *
 * Compile cleanly with: gcc -Wall -Wextra -o anti_debug anti_debug.c
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ptrace.h>

/* =============================================================================
 * TECHNIQUE 1: ptrace(PTRACE_TRACEME)
 * =============================================================================
 * HOW IT WORKS:
 *   A process can only have ONE tracer (debugger) attached at a time.
 *   If we call ptrace(PTRACE_TRACEME), we "attach to ourselves".
 *   If a debugger (gdb, strace, ltrace) is already attached, this call
 *   FAILS because the tracer slot is already taken.
 *
 * DETECTION: If ptrace returns -1, a debugger is present.
 *
 * BYPASS METHODS:
 *   1. LD_PRELOAD a library that overrides ptrace() to always return 0
 *   2. Binary patch: replace the ptrace call with NOPs
 *   3. In GDB: use 'catch syscall ptrace' and modify the return value
 *   4. Use a kernel module to intercept the syscall
 *
 * NOTE: This is the most common anti-debug technique in Linux malware.
 *       It's also the easiest to bypass.
 * =============================================================================
 */
static int check_ptrace(void)
{
    printf("[Check 1] ptrace(PTRACE_TRACEME) anti-debug check...\n");

    /* Try to trace ourselves.
     * If we're already being traced (debugger attached), this returns -1.
     * PTRACE_TRACEME = 0 on most architectures. */
    long result = ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    if (result == -1) {
        printf("  DETECTED: ptrace failed — a debugger is attached!\n");
        printf("  (ptrace returned -1: tracer slot already occupied)\n");
        printf("\n");
        printf("  HOW TO BYPASS:\n");
        printf("    1. LD_PRELOAD a fake ptrace that returns 0\n");
        printf("    2. Patch the binary: NOP out the ptrace call\n");
        printf("    3. In GDB: catch syscall ptrace, set return to 0\n");
        return 1;  /* Debugger detected */
    }

    printf("  PASSED: no debugger detected via ptrace\n");

    /* Detach from ourselves so the program can be debugged later if needed.
     * Note: after PTRACE_TRACEME, we ARE being traced by the parent (ourselves).
     * This is a simplification — in real malware, they just exit here. */

    return 0;  /* No debugger */
}

/* =============================================================================
 * TECHNIQUE 2: /proc/self/status TracerPid Check
 * =============================================================================
 * HOW IT WORKS:
 *   Linux exposes process information via the /proc filesystem.
 *   /proc/self/status contains a "TracerPid" field:
 *     - TracerPid: 0       → not being traced
 *     - TracerPid: 12345   → being traced by PID 12345
 *
 *   When GDB attaches, the kernel updates TracerPid automatically.
 *
 * DETECTION: Read /proc/self/status, parse TracerPid, check if non-zero.
 *
 * BYPASS METHODS:
 *   1. LD_PRELOAD: intercept open()/read() for /proc/self/status
 *   2. Use a modified kernel that always reports TracerPid: 0
 *   3. Binary patch: NOP out the file open and comparison
 *   4. In GDB: set a breakpoint after the check and jump past it
 *
 * NOTE: This check is slightly harder to bypass than ptrace because
 *       it uses standard file I/O, not a special syscall.
 * =============================================================================
 */
static int check_tracer_pid(void)
{
    printf("[Check 2] /proc/self/status TracerPid check...\n");

    FILE *fp = fopen("/proc/self/status", "r");
    if (fp == NULL) {
        printf("  SKIPPED: could not open /proc/self/status\n");
        printf("  (This check only works on Linux with /proc mounted)\n");
        return 0;
    }

    char line[256];
    int tracer_pid = 0;
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "TracerPid:", 10) == 0) {
            tracer_pid = atoi(line + 10);
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (!found) {
        printf("  SKIPPED: TracerPid field not found in status\n");
        return 0;
    }

    if (tracer_pid != 0) {
        printf("  DETECTED: TracerPid = %d (a debugger is attached!)\n", tracer_pid);
        printf("  The process with PID %d is tracing us.\n", tracer_pid);
        printf("\n");
        printf("  HOW TO BYPASS:\n");
        printf("    1. LD_PRELOAD: intercept fopen/fgets for /proc/self/status\n");
        printf("    2. In GDB: break after fclose, set tracer_pid = 0\n");
        printf("    3. Patch the binary: NOP out the comparison\n");
        return 1;
    }

    printf("  PASSED: TracerPid = 0 (no debugger detected)\n");
    return 0;
}

/* =============================================================================
 * TECHNIQUE 3: Timing-Based Detection
 * =============================================================================
 * HOW IT WORKS:
 *   When a program runs under a debugger, execution is slower because:
 *     - Single-stepping pauses at every instruction
 *     - Breakpoints cause SIGTRAP signals and context switches
 *     - The debugger intercepts and processes every signal
 *
 *   By measuring how long a block of code takes, we can detect debugging.
 *   Normal execution: ~1-10 microseconds for a simple computation
 *   Under debugger:   could be milliseconds or seconds (if breakpoints set)
 *
 * DETECTION: Measure time before/after a computation. If too slow, debugger.
 *
 * BYPASS METHODS:
 *   1. In GDB: set breakpoint AFTER the timing check, not inside it
 *   2. LD_PRELOAD: intercept clock_gettime to return fake times
 *   3. Patch: NOP out the timing comparison
 *   4. Just run the timed section without breakpoints in it
 *
 * NOTE: This is unreliable — system load, scheduling, and CPU frequency
 *       scaling can all cause false positives.
 * =============================================================================
 */
static int check_timing(void)
{
    printf("[Check 3] Timing-based anti-debug check...\n");

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    /* Do a simple computation that should be very fast */
    volatile int sum = 0;
    for (int i = 0; i < 100000; i++) {
        sum += i;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    /* Calculate elapsed time in microseconds */
    long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000L +
                      (end.tv_nsec - start.tv_nsec) / 1000L;

    printf("  Computation took: %ld microseconds\n", elapsed_us);

    /* Threshold: normal execution should take <50ms.
     * Under a debugger with breakpoints, it could take much longer.
     * We use a generous threshold to avoid false positives. */
    long threshold_us = 50000;  /* 50 milliseconds */

    if (elapsed_us > threshold_us) {
        printf("  DETECTED: execution took too long (%ld us > %ld us threshold)\n",
               elapsed_us, threshold_us);
        printf("  This suggests single-stepping or breakpoints in the timed section.\n");
        printf("\n");
        printf("  HOW TO BYPASS:\n");
        printf("    1. Don't set breakpoints inside the timed section\n");
        printf("    2. LD_PRELOAD: fake clock_gettime to return controlled values\n");
        printf("    3. Patch: change the threshold constant in the binary\n");
        return 1;
    }

    printf("  PASSED: execution time within normal range\n");
    return 0;
}

/* =============================================================================
 * TECHNIQUE 4: Parent Process Check
 * =============================================================================
 * HOW IT WORKS:
 *   When you run a program from a shell, its parent is bash/zsh.
 *   When you run it from GDB, its parent is gdb.
 *   By checking the parent process name, we can detect debuggers.
 *
 * DETECTION: Read /proc/self/status for PPid, then read /proc/<ppid>/comm
 *
 * BYPASS METHODS:
 *   1. Start the program normally, then attach GDB (gdb -p <pid>)
 *   2. Use a wrapper script that re-parents the process
 *   3. Patch the binary to skip the check
 *   4. Rename gdb to something else (crude but effective)
 * =============================================================================
 */
static int check_parent_process(void)
{
    printf("[Check 4] Parent process name check...\n");

    /* Get parent PID */
    pid_t ppid = getppid();
    printf("  Parent PID: %d\n", ppid);

    /* Read parent process name from /proc */
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/comm", ppid);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("  SKIPPED: could not read parent process name\n");
        return 0;
    }

    char parent_name[256];
    if (fgets(parent_name, sizeof(parent_name), fp) == NULL) {
        fclose(fp);
        printf("  SKIPPED: could not read parent comm\n");
        return 0;
    }
    fclose(fp);

    /* Remove trailing newline */
    size_t len = strlen(parent_name);
    if (len > 0 && parent_name[len - 1] == '\n') {
        parent_name[len - 1] = '\0';
    }

    printf("  Parent process: '%s'\n", parent_name);

    /* Check for known debugger names */
    const char *debuggers[] = {"gdb", "lldb", "strace", "ltrace", "radare2", "r2", "ida", NULL};

    for (int i = 0; debuggers[i] != NULL; i++) {
        if (strcmp(parent_name, debuggers[i]) == 0) {
            printf("  DETECTED: parent is '%s' — a known debugger!\n", parent_name);
            printf("\n");
            printf("  HOW TO BYPASS:\n");
            printf("    1. Run normally, then attach debugger: gdb -p <pid>\n");
            printf("    2. Rename the debugger binary\n");
            printf("    3. Use a wrapper script to change the parent\n");
            printf("    4. Patch the string comparisons in the binary\n");
            return 1;
        }
    }

    printf("  PASSED: parent '%s' is not a known debugger\n", parent_name);
    return 0;
}

/* =============================================================================
 * THE SECRET — only revealed if all checks pass
 * =============================================================================
 * This simulates protected functionality that the program tries to hide
 * from debuggers. In real malware, this would be the malicious payload.
 * In legitimate software, this might be DRM or license checking.
 * =============================================================================
 */
static void reveal_secret(void)
{
    printf("\n========================================\n");
    printf("  ALL ANTI-DEBUG CHECKS PASSED\n");
    printf("========================================\n\n");

    /* Decode a message (simple XOR, same technique as crackme.c) */
    unsigned char encoded[] = {
        0x32, 0x2b, 0x2a, 0x27, 0x3e, 0x26, 0x3f, 0x39,
        0x34, 0x2b, 0x2a, 0x68, 0x29, 0x35, 0x3e, 0x23,
        0x3e, 0x22, 0x22, 0x35, 0x68, 0x36, 0x3d, 0x2b,
        0x20, 0x3a, 0x68, 0x3e, 0x22, 0x68, 0x3b, 0x2b,
        0x3c, 0x2a, 0x68, 0x3e, 0x22, 0x68, 0x22, 0x2a,
        0x2a, 0x28, 0x22, 0x0e, 0x00
    };
    /* XOR key for decoding */
    unsigned char key = 0x47;

    printf("  Secret message: ");
    for (int i = 0; encoded[i] != 0x00; i++) {
        putchar(encoded[i] ^ key);
    }
    printf("\n\n");

    printf("  Now try to reach this code WITH a debugger attached!\n");
    printf("  Bypass each anti-debug check to get here under GDB.\n");
}

/* =============================================================================
 * MAIN
 * =============================================================================
 */
int main(void)
{
    printf("==========================================================\n");
    printf("  ANTI-DEBUGGING DEMONSTRATION\n");
    printf("  Educational: shows common techniques and how to bypass them\n");
    printf("==========================================================\n\n");

    int detected = 0;

    /* Run all checks */
    detected += check_ptrace();
    printf("\n");
    detected += check_tracer_pid();
    printf("\n");
    detected += check_timing();
    printf("\n");
    detected += check_parent_process();

    if (detected > 0) {
        printf("\n========================================\n");
        printf("  DEBUGGER DETECTED (%d check%s failed)\n",
               detected, detected > 1 ? "s" : "");
        printf("========================================\n\n");
        printf("  In real malware, the program would:\n");
        printf("    - Exit silently\n");
        printf("    - Behave differently (fake benign behavior)\n");
        printf("    - Corrupt its own memory to frustrate analysis\n");
        printf("    - Delete itself from disk\n");
        printf("\n");
        printf("  YOUR CHALLENGE: bypass all %d checks and reach the secret!\n", detected);
        printf("  Techniques: LD_PRELOAD, binary patching, GDB scripting\n");
    } else {
        reveal_secret();
    }

    printf("\n==========================================================\n");
    printf("  SUMMARY OF ANTI-DEBUG TECHNIQUES\n");
    printf("==========================================================\n");
    printf("\n");
    printf("  %-25s %-15s %s\n", "Technique", "Difficulty", "Bypass");
    printf("  %-25s %-15s %s\n", "---------", "----------", "------");
    printf("  %-25s %-15s %s\n", "ptrace(TRACEME)", "Trivial", "LD_PRELOAD / NOP");
    printf("  %-25s %-15s %s\n", "TracerPid check", "Easy", "LD_PRELOAD / GDB script");
    printf("  %-25s %-15s %s\n", "Timing check", "Easy", "avoid breakpoints in zone");
    printf("  %-25s %-15s %s\n", "Parent name check", "Easy", "attach after launch");
    printf("\n");
    printf("  TRUTH: All anti-debug techniques are breakable.\n");
    printf("  They only slow down analysis — they never prevent it.\n");
    printf("  Defense in depth (multiple checks) raises the bar.\n");
    printf("  But a determined reverse engineer WILL get through.\n");
    printf("==========================================================\n");

    return 0;
}
