/*
 * entry_point.c — The REAL startup chain: _start → main → cleanup
 * ================================================================
 *
 * Module 5: How Linux Runs a Binary
 *
 * WHAT THIS DEMONSTRATES:
 *   main() is NOT the first thing that runs in your program. The actual
 *   execution chain on Linux (with glibc) is:
 *
 *     1. Kernel loads the ELF, maps segments, sets up the stack
 *     2. Kernel jumps to the ELF entry point: _start (in crt1.o)
 *     3. _start calls __libc_start_main() (in libc)
 *     4. __libc_start_main does:
 *        a. Initializes libc (malloc, stdio, etc.)
 *        b. Calls __libc_csu_init → runs constructors
 *        c. Calls main(argc, argv, envp)
 *        d. When main returns, calls exit(return_value)
 *     5. exit() does:
 *        a. Runs atexit() handlers (in reverse registration order)
 *        b. Flushes stdio streams
 *        c. Runs destructors
 *        d. Calls _exit() → SYS_exit_group syscall
 *
 *   So the execution order for THIS program is:
 *
 *     _start                    (you won't see this — it's in crt1.o)
 *     __libc_start_main         (you won't see this — it's in libc)
 *     before_main_1()           (constructor, priority 101)
 *     before_main_2()           (constructor, priority 102)
 *     main()                    (your code)
 *     atexit_handler_2()        (registered second, runs first — LIFO!)
 *     atexit_handler_1()        (registered first, runs second)
 *     after_main_2()            (destructor, priority 102)
 *     after_main_1()            (destructor, priority 101)
 *
 * WHAT TO OBSERVE:
 *   - Constructors run BEFORE main, in priority order (lower = earlier)
 *   - Destructors run AFTER main, in REVERSE priority order
 *   - atexit handlers run in LIFO order (last registered = first called)
 *   - main() is sandwiched in the middle of a lot of setup/teardown
 *
 * TO BUILD AND RUN:
 *   gcc -Wall -Wextra -o entry_point entry_point.c
 *   ./entry_point
 *
 * TRY ALSO:
 *   # See the REAL entry point:
 *   readelf -h entry_point | grep Entry
 *   # It will say something like 0x1060, NOT the address of main()!
 *
 *   # Watch the entire startup with strace:
 *   strace ./entry_point
 *
 *   # See constructors/destructors in the ELF:
 *   readelf -d entry_point | grep INIT
 *   readelf -d entry_point | grep FINI
 *
 *   # Set a breakpoint at _start (the REAL entry point) in gdb:
 *   gdb ./entry_point
 *   (gdb) break _start
 *   (gdb) run
 *   (gdb) bt     # backtrace — you'll see the CRT startup
 */

#include <stdio.h>
#include <stdlib.h>

/* -----------------------------------------------------------------------
 * CONSTRUCTORS: Functions that run BEFORE main()
 *
 * __attribute__((constructor)) tells the compiler to add this function
 * to the .init_array section of the ELF binary. The C runtime startup
 * code iterates over .init_array and calls each function before main().
 *
 * The optional (priority) argument controls the ORDER:
 *   - Lower numbers run FIRST
 *   - Range 0-100 is RESERVED for the implementation (libc)
 *   - User constructors should use 101+
 *
 * Common uses of constructors:
 *   - Library initialization (e.g., OpenSSL, GPU drivers)
 *   - Registering plugins or signal handlers
 *   - Setting up logging before the program "officially" starts
 * ----------------------------------------------------------------------- */

__attribute__((constructor(101)))
void before_main_1(void)
{
    printf("[1] CONSTRUCTOR (priority 101): before_main_1() — I run BEFORE main!\n");
    printf("    main() hasn't been called yet. stdio is already initialized though,\n");
    printf("    because __libc_start_main set it up before running constructors.\n\n");
}

__attribute__((constructor(102)))
void before_main_2(void)
{
    printf("[2] CONSTRUCTOR (priority 102): before_main_2() — I also run before main!\n");
    printf("    I run AFTER before_main_1 because my priority (102) is higher.\n\n");
}

/* -----------------------------------------------------------------------
 * DESTRUCTORS: Functions that run AFTER main() returns
 *
 * __attribute__((destructor)) adds the function to .fini_array.
 * These run after main() returns (or after exit() is called),
 * in REVERSE priority order.
 * ----------------------------------------------------------------------- */

__attribute__((destructor(101)))
void after_main_1(void)
{
    printf("[7] DESTRUCTOR (priority 101): after_main_1() — I run AFTER main!\n");
    printf("    I'm the last destructor because my priority (101) is the lowest.\n");
    printf("    After me, the process truly exits (SYS_exit_group).\n");
}

__attribute__((destructor(102)))
void after_main_2(void)
{
    printf("[6] DESTRUCTOR (priority 102): after_main_2() — I also run after main!\n");
    printf("    Destructors run in REVERSE priority order (higher priority = first).\n\n");
}

/* -----------------------------------------------------------------------
 * ATEXIT HANDLERS: Functions registered at runtime, called on exit
 *
 * atexit() registers functions that run when exit() is called (or when
 * main() returns, which implicitly calls exit()).
 *
 * Key behaviors:
 *   - Handlers run in LIFO order (last registered = first called)
 *   - You can register up to ATEXIT_MAX handlers (at least 32)
 *   - They run BEFORE destructors
 *   - They do NOT run if you call _exit() or _Exit() directly
 * ----------------------------------------------------------------------- */

void atexit_handler_1(void)
{
    printf("[4] ATEXIT handler 1: Registered FIRST, but runs SECOND (LIFO order).\n");
    printf("    atexit handlers run AFTER main but BEFORE destructors.\n\n");
}

void atexit_handler_2(void)
{
    printf("[5] ATEXIT handler 2: Registered SECOND, but runs FIRST (LIFO order).\n\n");
}

/* -----------------------------------------------------------------------
 * MAIN: The function you THOUGHT was the beginning
 * ----------------------------------------------------------------------- */

int main(void)
{
    printf("[3] MAIN: main() is finally running!\n");
    printf("    But look above — constructors already ran before me.\n\n");

    /* Register atexit handlers */
    atexit(atexit_handler_1);
    atexit(atexit_handler_2);

    printf("    Registered 2 atexit handlers.\n");
    printf("    When main returns, they'll run in REVERSE order (LIFO),\n");
    printf("    followed by destructors.\n\n");

    printf("    The FULL execution chain on Linux (glibc) is:\n");
    printf("    ┌───────────────────────────────────────────────┐\n");
    printf("    │ 1. Kernel: load ELF, map segments, push      │\n");
    printf("    │    argc/argv/envp/auxv onto stack             │\n");
    printf("    │                                               │\n");
    printf("    │ 2. _start (from crt1.o):                     │\n");
    printf("    │    Aligns stack, calls __libc_start_main      │\n");
    printf("    │                                               │\n");
    printf("    │ 3. __libc_start_main (from libc):             │\n");
    printf("    │    - Initialize libc internals                │\n");
    printf("    │    - Set up thread-local storage              │\n");
    printf("    │    - Call .init_array (constructors)           │\n");
    printf("    │    - Call main(argc, argv, envp)              │\n");
    printf("    │                                               │\n");
    printf("    │ 4. main() — YOUR CODE RUNS HERE              │\n");
    printf("    │                                               │\n");
    printf("    │ 5. exit(return_value):                        │\n");
    printf("    │    - Run atexit handlers (LIFO)               │\n");
    printf("    │    - Flush stdio streams                      │\n");
    printf("    │    - Call .fini_array (destructors)            │\n");
    printf("    │    - Call _exit() → SYS_exit_group            │\n");
    printf("    └───────────────────────────────────────────────┘\n\n");

    printf("[3] MAIN: About to return 0. Watch what happens next...\n\n");

    return 0;
    /*
     * Returning from main() is equivalent to calling exit(0).
     * __libc_start_main catches our return value and calls exit() with it.
     * This triggers: atexit handlers → fflush → destructors → _exit
     */
}
