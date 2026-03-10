/*
 * fork_exec.c — A mini shell: this is literally how bash works
 * =============================================================
 *
 * Module 5: How Linux Runs a Binary
 *
 * WHAT THIS DEMONSTRATES:
 *   This is a minimal implementation of a shell. Every Unix shell (bash,
 *   zsh, fish) works on the same fundamental principle:
 *
 *     1. PROMPT: Print a prompt and read user input
 *     2. PARSE:  Break the input into command + arguments
 *     3. FORK:   Create a child process (copy of the shell)
 *     4. EXEC:   In the child, replace the copy with the requested program
 *     5. WAIT:   In the parent, wait for the child to finish
 *     6. REPEAT: Go back to step 1
 *
 *   That's it. bash is "just" a really sophisticated version of this loop
 *   with job control, piping, redirection, scripting, history, etc.
 *
 * HOW fork() + exec() WORK TOGETHER:
 *
 *   Before fork():
 *     [mini-shell process PID=100]
 *
 *   After fork():
 *     [Parent PID=100]  ──── identical copy ────  [Child PID=101]
 *           │                                          │
 *           │                                     execve("/bin/ls")
 *           │                                          │
 *           │                                    [/bin/ls PID=101]
 *        wait()                                        │
 *           │                                      ls runs...
 *           │                                      ls exits
 *           │◄─── child exit status ──────────────────┘
 *        print next prompt
 *
 * WHY fork() BEFORE exec()?
 *   Because exec() REPLACES the current process. If the shell called
 *   exec() directly, the shell itself would be replaced by 'ls' and
 *   when 'ls' exits, there's no shell to return to!
 *
 *   fork() creates a SACRIFICIAL COPY that gets replaced by exec(),
 *   while the original shell survives to run more commands.
 *
 * TO BUILD AND RUN:
 *   gcc -Wall -Wextra -o mini_shell fork_exec.c
 *   ./mini_shell
 *   mini$ ls -la
 *   mini$ echo hello world
 *   mini$ /bin/uname -a
 *   mini$ exit
 *
 * LIMITATIONS (on purpose — this is educational, not production):
 *   - No piping (|), redirection (>, <), or background (&)
 *   - No environment variable expansion ($HOME)
 *   - No globbing (*.c)
 *   - No quoting ("hello world")
 *   - Searches PATH manually (or you can use full paths)
 *   - Max 64 arguments per command
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LINE    1024    /* Maximum input line length */
#define MAX_ARGS    64      /* Maximum number of arguments */

/*
 * parse_line() — Split a command line into argv-style tokens
 *
 * Takes a string like "ls -la /tmp" and produces:
 *   args[0] = "ls"
 *   args[1] = "-la"
 *   args[2] = "/tmp"
 *   args[3] = NULL    (required by execvp)
 *
 * Uses strtok() which modifies the input string in place by replacing
 * delimiters with '\0'. This is fine since we don't need the original.
 *
 * Returns: number of arguments parsed
 */
static int parse_line(char *line, char *args[], int max_args)
{
    int argc = 0;
    char *token;

    /* Remove trailing newline */
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }

    /*
     * strtok splits by whitespace. Each call returns the next token.
     * " \t" means split on spaces and tabs.
     */
    token = strtok(line, " \t");
    while (token != NULL && argc < max_args - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t");
    }

    /* argv must be NULL-terminated (required by execvp) */
    args[argc] = NULL;

    return argc;
}

/*
 * run_command() — Fork a child process and execute a command
 *
 * This is the core of any Unix shell:
 *   1. fork() to create a child
 *   2. In child: execvp() to replace the child with the command
 *   3. In parent: waitpid() to wait for the child to finish
 */
static void run_command(char *args[])
{
    /*
     * STEP 1: fork()
     *
     * fork() creates a nearly identical copy of the current process.
     * After fork() returns:
     *   - Parent: fork() returns the child's PID (> 0)
     *   - Child:  fork() returns 0
     *   - Error:  fork() returns -1
     *
     * The child gets:
     *   - Same code (same text segment, actually shared via copy-on-write)
     *   - Same data (copy of all variables — but copy-on-write in practice)
     *   - Same open file descriptors (stdin/stdout/stderr)
     *   - A NEW PID
     */
    pid_t pid = fork();

    if (pid < 0) {
        /* fork failed */
        perror("mini-shell: fork");
        return;
    }

    if (pid == 0) {
        /* ============================================================
         * CHILD PROCESS
         *
         * STEP 2: exec()
         *
         * execvp() searches PATH for the command and replaces this
         * process with it. The "vp" means:
         *   v = vector (pass args as array)
         *   p = path (search PATH environment variable)
         *
         * If execvp succeeds, the lines below NEVER execute.
         * The child process IS NOW the requested command.
         * ============================================================ */

        printf("  [child PID %d] Executing: %s\n", getpid(), args[0]);

        execvp(args[0], args);

        /*
         * If we reach here, execvp() failed.
         * Common reasons:
         *   ENOENT = command not found in PATH
         *   EACCES = not executable (check permissions)
         */
        fprintf(stderr, "mini-shell: %s: %s\n", args[0], strerror(errno));
        _exit(127);  /* 127 = conventional "command not found" exit code */
    }

    /* ================================================================
     * PARENT PROCESS
     *
     * STEP 3: wait()
     *
     * waitpid() blocks until the child process terminates.
     * The shell needs to wait so it can:
     *   1. Collect the child's exit status
     *   2. Know when to show the next prompt
     *   3. Prevent zombie processes (children that have exited but
     *      whose status hasn't been collected)
     * ================================================================ */

    int status;
    waitpid(pid, &status, 0);

    /*
     * Decode exit status using the W* macros.
     * These macros are necessary because waitpid() encodes multiple
     * pieces of information into a single int.
     */
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0) {
            printf("  [parent] Command exited with status %d\n", exit_code);
        }
    } else if (WIFSIGNALED(status)) {
        printf("  [parent] Command killed by signal %d", WTERMSIG(status));
#ifdef WCOREDUMP
        if (WCOREDUMP(status)) {
            printf(" (core dumped)");
        }
#endif
        printf("\n");
    }
}

/*
 * builtin_cd() — Handle the 'cd' command
 *
 * cd CANNOT be an external program because execve would change the
 * CHILD's working directory, not the shell's. This is why cd must be
 * a "builtin" — handled directly by the shell process.
 */
static void builtin_cd(char *args[], int argc)
{
    const char *dir;

    if (argc < 2) {
        /* cd with no args → go home */
        dir = getenv("HOME");
        if (!dir) {
            fprintf(stderr, "mini-shell: cd: HOME not set\n");
            return;
        }
    } else {
        dir = args[1];
    }

    if (chdir(dir) != 0) {
        perror("mini-shell: cd");
    }
}

int main(void)
{
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║  Mini Shell — A minimal fork+exec shell                  ║\n");
    printf("║                                                          ║\n");
    printf("║  This is how ALL Unix shells work at their core:         ║\n");
    printf("║    1. Read a command line                                 ║\n");
    printf("║    2. fork() to create a child process                   ║\n");
    printf("║    3. exec() in the child to run the command             ║\n");
    printf("║    4. wait() in the parent for the child to finish       ║\n");
    printf("║                                                          ║\n");
    printf("║  Try: ls, echo hello, /bin/uname -a, cd /tmp, exit      ║\n");
    printf("║                                                          ║\n");
    printf("║  Run with strace -f to see the fork/exec/wait syscalls: ║\n");
    printf("║    strace -f ./mini_shell                                ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");

    /* ---------------------------------------------------------------
     * THE MAIN LOOP — Read-Eval-Print Loop (REPL)
     *
     * This is the same loop that runs in every shell:
     *   1. Print prompt
     *   2. Read input
     *   3. Parse into argv
     *   4. Check for builtins (cd, exit)
     *   5. For external commands: fork + exec + wait
     *   6. Repeat
     * --------------------------------------------------------------- */

    for (;;) {
        /* Print prompt with current working directory */
        char cwd[512];
        if (getcwd(cwd, sizeof(cwd))) {
            printf("mini[%s]$ ", cwd);
        } else {
            printf("mini$ ");
        }
        fflush(stdout);  /* Ensure prompt appears before blocking on input */

        /*
         * Read a line of input. fgets() blocks until the user presses Enter.
         * Returns NULL on EOF (Ctrl-D) or error.
         */
        if (fgets(line, sizeof(line), stdin) == NULL) {
            /* EOF (Ctrl-D) — user wants to exit */
            printf("\n[EOF — goodbye!]\n");
            break;
        }

        /* Parse the line into args */
        int argc = parse_line(line, args, MAX_ARGS);

        /* Skip empty lines */
        if (argc == 0) {
            continue;
        }

        /* ---------------------------------------------------------------
         * Check for BUILTIN commands
         *
         * Some commands MUST be handled by the shell itself, not by
         * forking a child:
         *   - 'exit' — terminates the shell itself
         *   - 'cd'   — changes the shell's working directory
         *
         * These are "builtins" because they need to affect the shell
         * process directly. A child process can't change the parent's
         * working directory or terminate the parent.
         * --------------------------------------------------------------- */

        if (strcmp(args[0], "exit") == 0) {
            printf("[Exiting mini-shell. Goodbye!]\n");
            break;
        }

        if (strcmp(args[0], "cd") == 0) {
            builtin_cd(args, argc);
            continue;
        }

        /* External command — fork + exec + wait */
        run_command(args);
    }

    return 0;
}
