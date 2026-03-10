/*
 * segfault.c — A program that segfaults, designed for GDB diagnosis practice
 * ===========================================================================
 *
 * Module 6: Debuggers at the Binary Level
 *
 * PURPOSE:
 *   This program deliberately crashes with a segmentation fault (SIGSEGV).
 *   The crash is hidden inside a nested call chain, simulating a real-world
 *   scenario where the bug isn't immediately obvious.
 *
 *   Your task: use GDB to find WHERE and WHY it crashes.
 *
 * HOW TO DIAGNOSE WITH GDB:
 *   gcc -g -O0 -o segfault segfault.c
 *   gdb ./segfault
 *   (gdb) run
 *   ... Program received signal SIGSEGV, Segmentation fault ...
 *   (gdb) backtrace          ← Shows the call chain that led to the crash
 *   (gdb) frame N            ← Jump to a specific frame in the backtrace
 *   (gdb) info locals        ← What are the local variables?
 *   (gdb) print ptr          ← Is the pointer NULL?
 *   (gdb) list               ← Show the source code around the crash
 *
 * WHAT IS A SEGFAULT?
 *   A segmentation fault occurs when a program tries to access memory
 *   that it's not allowed to access. Common causes:
 *
 *     1. Dereferencing a NULL pointer          — ptr = NULL; *ptr = 42;
 *     2. Accessing freed memory (use-after-free)
 *     3. Writing to read-only memory (.rodata)
 *     4. Stack overflow (infinite recursion)
 *     5. Accessing past the end of an array (sometimes)
 *
 *   The CPU's MMU (Memory Management Unit) detects the invalid access,
 *   generates a page fault, the kernel sees it's invalid, and sends
 *   SIGSEGV to the process.
 *
 * WHAT THE BACKTRACE TELLS YOU:
 *   The backtrace (bt) shows the call stack at the moment of the crash:
 *
 *     #0  inner_function (ptr=0x0) at segfault.c:XX    ← where it crashed
 *     #1  middle_function (data=...) at segfault.c:XX   ← who called that
 *     #2  outer_function () at segfault.c:XX            ← who called that
 *     #3  main () at segfault.c:XX                      ← entry point
 *
 *   Frame #0 is the crash site. Work your way UP the frames to understand
 *   how a NULL pointer got there.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -----------------------------------------------------------------------
 * Data structures used in the call chain
 * ----------------------------------------------------------------------- */

struct Config {
    char name[64];
    int  level;
};

struct Node {
    int           value;
    struct Config *config;  /* This might be NULL... */
    struct Node   *next;
};

struct Context {
    struct Node *head;
    int          node_count;
    char         description[128];
};

/* -----------------------------------------------------------------------
 * deep_process() — The function that actually crashes
 *
 * GDB: When the crash happens, the backtrace will lead you here.
 *   (gdb) backtrace
 *   (gdb) print config           ← Is config NULL?
 *   (gdb) print config->name     ← This is what crashes!
 *
 * The crash is a NULL pointer dereference. 'config' is NULL, and
 * accessing config->name tries to read from address 0x0 (or near it).
 * ----------------------------------------------------------------------- */
static void deep_process(struct Config *config, int multiplier)
{
    printf("    deep_process: multiplier=%d\n", multiplier);

    /*
     * THIS LINE CRASHES when config is NULL.
     *
     * config->name is equivalent to: ((char *)config + offsetof(Config, name))
     * If config == NULL (0x0), this tries to read from address 0x0,
     * which is unmapped memory → SIGSEGV.
     */
    printf("    deep_process: config name = '%s'\n", config->name);
    printf("    deep_process: config level = %d\n", config->level);
    printf("    deep_process: adjusted level = %d\n", config->level * multiplier);
}

/* -----------------------------------------------------------------------
 * process_node() — Processes a single node, calls deep_process()
 *
 * GDB: Check the node's config pointer here:
 *   (gdb) frame N          (where N is this function's frame number)
 *   (gdb) print node->config   ← Is it NULL or valid?
 *   (gdb) print *node          ← See all fields
 * ----------------------------------------------------------------------- */
static void process_node(struct Node *node, int index)
{
    printf("  process_node: processing node %d (value=%d)\n",
           index, node->value);

    /*
     * We pass node->config to deep_process without checking if it's NULL.
     * For nodes 0 and 1, config is valid. For node 2, config is NULL.
     * This is the root cause — but the crash happens one level deeper.
     */
    deep_process(node->config, index + 1);
}

/* -----------------------------------------------------------------------
 * traverse_list() — Walks the linked list, calls process_node()
 *
 * GDB: Inspect the linked list structure:
 *   (gdb) print *ctx->head                    ← first node
 *   (gdb) print *ctx->head->next              ← second node
 *   (gdb) print *ctx->head->next->next        ← third node
 *   (gdb) print ctx->head->next->next->config ← NULL!
 * ----------------------------------------------------------------------- */
static void traverse_list(struct Context *ctx)
{
    printf("traverse_list: processing %d nodes in '%s'\n",
           ctx->node_count, ctx->description);

    struct Node *current = ctx->head;
    int index = 0;

    while (current != NULL) {
        process_node(current, index);
        current = current->next;
        index++;
    }

    printf("traverse_list: done.\n");
}

/* -----------------------------------------------------------------------
 * build_context() — Creates the data structures
 *
 * NOTE: Node 2 intentionally has config = NULL. This is the setup
 * for the bug. In real code, this might happen due to:
 *   - A failed allocation (malloc returns NULL)
 *   - A missing configuration entry
 *   - A database query returning no results
 *   - Forgetting to initialize a field
 * ----------------------------------------------------------------------- */
static struct Context *build_context(void)
{
    struct Context *ctx = malloc(sizeof(struct Context));
    if (!ctx) return NULL;

    strncpy(ctx->description, "Test processing run", sizeof(ctx->description) - 1);
    ctx->description[sizeof(ctx->description) - 1] = '\0';
    ctx->node_count = 3;

    /* Create node 0 — has a valid config */
    struct Node *node0 = malloc(sizeof(struct Node));
    struct Config *cfg0 = malloc(sizeof(struct Config));
    if (!node0 || !cfg0) { free(node0); free(cfg0); free(ctx); return NULL; }
    strncpy(cfg0->name, "Primary", sizeof(cfg0->name) - 1);
    cfg0->name[sizeof(cfg0->name) - 1] = '\0';
    cfg0->level = 10;
    node0->value = 100;
    node0->config = cfg0;

    /* Create node 1 — has a valid config */
    struct Node *node1 = malloc(sizeof(struct Node));
    struct Config *cfg1 = malloc(sizeof(struct Config));
    if (!node1 || !cfg1) { free(node1); free(cfg1); free(node0->config); free(node0); free(ctx); return NULL; }
    strncpy(cfg1->name, "Secondary", sizeof(cfg1->name) - 1);
    cfg1->name[sizeof(cfg1->name) - 1] = '\0';
    cfg1->level = 20;
    node1->value = 200;
    node1->config = cfg1;

    /* Create node 2 — config is intentionally NULL (THE BUG) */
    struct Node *node2 = malloc(sizeof(struct Node));
    if (!node2) { free(node1->config); free(node1); free(node0->config); free(node0); free(ctx); return NULL; }
    node2->value = 300;
    node2->config = NULL;  /* <== THIS IS THE ROOT CAUSE */
    node2->next = NULL;

    /* Link the list: node0 → node1 → node2 → NULL */
    node0->next = node1;
    node1->next = node2;
    ctx->head = node0;

    return ctx;
}

/* -----------------------------------------------------------------------
 * free_context() — Clean up allocated memory
 * ----------------------------------------------------------------------- */
static void free_context(struct Context *ctx)
{
    if (!ctx) return;

    struct Node *current = ctx->head;
    while (current) {
        struct Node *next = current->next;
        free(current->config);  /* free(NULL) is safe */
        free(current);
        current = next;
    }
    free(ctx);
}

/* -----------------------------------------------------------------------
 * main()
 *
 * GDB session example:
 *   $ gdb ./segfault
 *   (gdb) run
 *   Program received signal SIGSEGV, Segmentation fault.
 *   0x... in deep_process (config=0x0, multiplier=3) at segfault.c:XX
 *   (gdb) bt
 *   #0  deep_process (config=0x0, ...)    ← config is NULL!
 *   #1  process_node (node=0x..., index=2)
 *   #2  traverse_list (ctx=0x...)
 *   #3  main ()
 *   (gdb) frame 1
 *   (gdb) print node->config              ← NULL! That's why deep_process got NULL
 *   (gdb) frame 2
 *   (gdb) print *ctx                      ← see the context
 *   (gdb) print *ctx->head->next->next    ← node 2, the problematic one
 *   (gdb) print ctx->head->next->next->config  ← NULL!
 *
 *   Now you know: node 2's config is NULL. Go to build_context() to see why.
 * ----------------------------------------------------------------------- */
int main(void)
{
    printf("=== Segfault Diagnosis Practice ===\n");
    printf("This program WILL crash. That's intentional.\n");
    printf("Use GDB to find out WHERE and WHY.\n\n");
    printf("  gdb ./segfault\n");
    printf("  (gdb) run\n");
    printf("  (gdb) backtrace\n");
    printf("  (gdb) print config\n");
    printf("  (gdb) frame 1\n");
    printf("  (gdb) print node->config\n\n");

    struct Context *ctx = build_context();
    if (!ctx) {
        fprintf(stderr, "Failed to build context\n");
        return 1;
    }

    /* This will process nodes 0 and 1 fine, then crash on node 2 */
    traverse_list(ctx);

    /* We never reach here due to the crash */
    printf("Processing complete.\n");

    free_context(ctx);
    return 0;
}
