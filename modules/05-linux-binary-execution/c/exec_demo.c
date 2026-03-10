/*
 * exec_demo.c — Demonstrates execve(): replacing a process image
 * ===============================================================
 *
 * Module 5: How Linux Runs a Binary
 *
 * WHAT THIS DEMONSTRATES:
 *   execve() is the ONE syscall that loads and runs a new program.
 *   When a process calls execve():
 *
 *     1. The kernel DESTROYS the calling process's entire memory image
 *        - All code (.text) — gone
 *        - All data (.data, .bss, heap, stack) — gone
 *        - All memory mappings — gone
 *     2. The kernel loads the NEW program (parses its ELF header, maps segments)
 *     3. Sets up a fresh stack with argc, argv, envp
 *     4. Jumps to the new program's entry point (_start)
 *
 *   After execve(), the old program is COMPLETELY GONE. There is no "return"
 *   to the old code — it doesn't exist anymore. The process keeps the same PID,
 *   but everything else is replaced.
 *
 *   This is why shells use fork() + execve():
 *     - fork() creates a COPY of the shell process
 *     - The child calls execve() to become the new program
 *     - The parent (shell) is unaffected and waits for the child
 *
 * WHAT TO OBSERVE:
 *   - The parent process prints before and after the child runs
 *   - The child's code AFTER execve() never executes (it's been replaced)
 *   - The child gets a new program but keeps the same PID
 *
 * TO BUILD AND RUN:
 *   gcc -Wall -Wextra -o exec_demo exec_demo.c
 *   ./exec_demo
 *
 * TRY ALSO:
 *   strace -f ./exec_demo
 *   # -f follows forks, so you'll see both parent and child's syscalls
 *   # Look for: clone/fork, execve, wait4
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main(void)
{
    printf("=== execve() Demonstration ===\n\n");

    printf("Parent process PID: %d\n\n", getpid());

    /* ---------------------------------------------------------------
     * DEMO 1: fork() + execve() to run /bin/echo
     *
     * fork() creates an almost-exact copy of this process.
     * The child then calls execve() to REPLACE itself with /bin/echo.
     * --------------------------------------------------------------- */
    printf("--- Demo 1: fork() + execve(\"/bin/echo\") ---\n");

    pid_t pid = fork();

    if (pid < 0) {
        /*
         * fork() failed. This is rare but can happen if:
         *   - System is out of memory
         *   - Process limit reached (ulimit -u)
         */
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        /* ============================================================
         * CHILD PROCESS
         *
         * Right now, the child is an exact copy of the parent:
         * same code, same data, same stack. But we're about to
         * replace ALL of that with /bin/echo.
         * ============================================================ */

        printf("  [Child PID %d] About to call execve(\"/bin/echo\")\n", getpid());
        printf("  [Child PID %d] After execve, this process's memory will be:\n", getpid());
        printf("    - Old code (.text of exec_demo): DESTROYED\n");
        printf("    - Old data (all variables):      DESTROYED\n");
        printf("    - New code (.text of /bin/echo):  LOADED\n");
        printf("    - New stack with argv:            CREATED\n\n");

        /*
         * execve() takes three arguments:
         *   1. Path to the executable
         *   2. argv[] — argument vector (must be NULL-terminated)
         *   3. envp[] — environment (NULL = inherit parent's environment)
         *
         * The argv[0] is conventionally the program name.
         */
        char *args[] = {
            "echo",                                 /* argv[0]: program name     */
            "  [/bin/echo] Hello from the NEW program! The old code is GONE.",
            NULL                                    /* argv must end with NULL   */
        };

        /*
         * execve() replaces this entire process image.
         * If it succeeds, the lines below NEVER execute.
         * If it fails, it returns -1.
         */
        execve("/bin/echo", args, NULL);

        /*
         * !! WE ONLY REACH HERE IF execve() FAILED !!
         *
         * Common reasons for failure:
         *   - ENOENT: file doesn't exist
         *   - EACCES: not executable
         *   - ENOEXEC: not a valid executable format
         */
        perror("execve failed");
        _exit(127);  /* Convention: 127 = command not found */

        /*
         * THIS CODE IS UNREACHABLE WHEN execve SUCCEEDS.
         * After execve("/bin/echo", ...), this process IS /bin/echo.
         * The entire memory image of exec_demo (in this child) no longer exists.
         */
        printf("YOU WILL NEVER SEE THIS LINE — the process image was replaced!\n");
    }

    /* ================================================================
     * PARENT PROCESS
     *
     * The parent continues executing here. fork() returned the child's
     * PID, so we can wait for it.
     * ================================================================ */

    printf("[Parent PID %d] Waiting for child PID %d...\n", getpid(), pid);

    int status;
    waitpid(pid, &status, 0);

    /*
     * Decode the child's exit status.
     * The status value from waitpid() is encoded — use the W* macros:
     *   WIFEXITED(s)    — true if child exited normally
     *   WEXITSTATUS(s)  — the exit code (0-255)
     *   WIFSIGNALED(s)  — true if child was killed by a signal
     *   WTERMSIG(s)     — the signal number
     */
    if (WIFEXITED(status)) {
        printf("[Parent] Child exited with status %d\n\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("[Parent] Child killed by signal %d\n\n", WTERMSIG(status));
    }

    /* ---------------------------------------------------------------
     * DEMO 2: execve() with a non-existent binary (to show failure)
     * --------------------------------------------------------------- */
    printf("--- Demo 2: execve() with a non-existent binary ---\n");

    pid_t pid2 = fork();

    if (pid2 == 0) {
        /* Child: try to exec a non-existent program */
        char *args[] = { "nonexistent", NULL };
        printf("  [Child PID %d] Trying execve(\"/usr/bin/nonexistent\")...\n", getpid());

        execve("/usr/bin/nonexistent", args, NULL);

        /* execve failed — we're still the old program */
        printf("  [Child PID %d] execve FAILED: %s\n", getpid(), strerror(errno));
        printf("  [Child PID %d] Since execve failed, I'm still the OLD program!\n", getpid());
        printf("  [Child PID %d] My code and data are intact.\n\n", getpid());
        _exit(1);
    }

    waitpid(pid2, &status, 0);
    if (WIFEXITED(status)) {
        printf("[Parent] Child exited with status %d\n\n", WEXITSTATUS(status));
    }

    /* ---------------------------------------------------------------
     * DEMO 3: execve() passing environment variables
     * --------------------------------------------------------------- */
    printf("--- Demo 3: execve() with custom environment ---\n");

    pid_t pid3 = fork();

    if (pid3 == 0) {
        /*
         * We can pass a custom environment to the new program.
         * This completely REPLACES the inherited environment.
         */
        char *args[] = { "env", NULL };
        char *envp[] = {
            "MY_CUSTOM_VAR=hello_from_parent",
            "COURSE=source_to_binary",
            "MODULE=05_linux_execution",
            NULL  /* environment must also be NULL-terminated */
        };

        printf("  [Child PID %d] Calling execve(\"/usr/bin/env\") with custom env...\n\n",
               getpid());

        execve("/usr/bin/env", args, envp);

        perror("execve failed");
        _exit(127);
    }

    waitpid(pid3, &status, 0);

    printf("\n[Parent] Child exited with status %d\n\n", WEXITSTATUS(status));

    /* ---------------------------------------------------------------
     * SUMMARY
     * --------------------------------------------------------------- */
    printf("=== KEY TAKEAWAYS ===\n");
    printf("1. execve() REPLACES the process image entirely.\n");
    printf("   Code, data, heap, stack — all destroyed and replaced.\n\n");
    printf("2. The PID stays the same. Open file descriptors (unless\n");
    printf("   FD_CLOEXEC) are inherited. Signal dispositions are reset.\n\n");
    printf("3. If execve() succeeds, it NEVER returns. The old code\n");
    printf("   simply doesn't exist anymore.\n\n");
    printf("4. If execve() FAILS, the process continues normally.\n");
    printf("   The old code and data are untouched.\n\n");
    printf("5. fork() + execve() is how EVERY program is launched on Linux.\n");
    printf("   Your shell does this for every command you type.\n");

    return 0;
}
