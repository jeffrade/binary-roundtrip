/*
 * =============================================================================
 * FUNCTION FINDER — Module 10: Reverse Engineering a C Binary
 * =============================================================================
 *
 * YOUR MISSION:
 *   1. Compile and strip:  gcc -O0 -o function_finder function_finder.c && strip function_finder
 *   2. Run it:             ./function_finder
 *   3. Identify what EACH function does by analyzing the stripped binary!
 *
 * THE PROGRAM HAS 6 DISTINCT FUNCTIONS (besides main):
 *   When stripped, they become anonymous: sub_XXXX or unnamed blocks.
 *   Your job: figure out what each one does.
 *
 * TECHNIQUES TO USE:
 *   - strings function_finder     — find embedded strings (format strings give huge clues)
 *   - ltrace ./function_finder    — watch library calls (printf, strcmp, qsort, etc.)
 *   - strace ./function_finder    — watch system calls (file access, etc.)
 *   - objdump -d function_finder  — read the disassembly
 *   - gdb ./function_finder       — set breakpoints, inspect state
 *
 * CLUES IN THE BINARY (even when stripped):
 *   - String literals survive stripping (format strings are your best friend)
 *   - Library function calls are visible (they go through PLT/GOT)
 *   - Constants like array sizes, loop bounds are embedded as immediates
 *   - The calling convention tells you argument count and types
 *
 * WHAT THE FUNCTIONS ARE (SPOILER — try RE first!):
 *   1. Bubble sort
 *   2. Binary search
 *   3. Caesar cipher encode
 *   4. Caesar cipher decode
 *   5. Email address validator
 *   6. Run-length encoding compression
 *
 * Compile cleanly with: gcc -Wall -Wextra -o function_finder function_finder.c
 * =============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* =============================================================================
 * FUNCTION 1: Bubble Sort
 * =============================================================================
 * HINTS FOR IDENTIFICATION:
 *   - Two nested loops visible in disassembly
 *   - Swap pattern: temp = a; a = b; b = temp (three moves)
 *   - Array element access with index arithmetic (base + i * sizeof(int))
 *   - Comparison followed by conditional swap
 *   - ltrace won't show much (no libc calls inside)
 *   - The outer loop runs n-1 times, inner loop shrinks each iteration
 */
static void func_alpha(int *arr, int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* =============================================================================
 * FUNCTION 2: Binary Search
 * =============================================================================
 * HINTS FOR IDENTIFICATION:
 *   - Three variables: low, high, mid
 *   - mid = (low + high) / 2 — shift right by 1 in disassembly (sar $1)
 *   - Comparison that branches THREE ways: equal, less, greater
 *   - Returns an index or -1 (0xFFFFFFFF in disassembly)
 *   - Loop that converges (low and high move toward each other)
 */
static int func_beta(const int *arr, int n, int target)
{
    int low = 0;
    int high = n - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return -1;  /* Not found */
}

/* =============================================================================
 * FUNCTION 3: Caesar Cipher Encode
 * =============================================================================
 * HINTS FOR IDENTIFICATION:
 *   - Loops through each character
 *   - Checks if character is a letter (comparisons with 'a','z','A','Z')
 *   - Adds a shift value (second parameter) to each letter
 *   - Modular arithmetic with 26 (0x1A) — alphabet wrapping
 *   - Very similar to ROT13 but with a configurable shift
 *   - ltrace: no libc calls inside (pure computation)
 */
static void func_gamma(char *text, int shift)
{
    shift = ((shift % 26) + 26) % 26;  /* Normalize shift to 0-25 */

    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            text[i] = 'a' + (text[i] - 'a' + shift) % 26;
        } else if (text[i] >= 'A' && text[i] <= 'Z') {
            text[i] = 'A' + (text[i] - 'A' + shift) % 26;
        }
        /* Non-letters pass through unchanged */
    }
}

/* =============================================================================
 * FUNCTION 4: Caesar Cipher Decode (inverse of func_gamma)
 * =============================================================================
 * HINTS FOR IDENTIFICATION:
 *   - Almost identical structure to func_gamma
 *   - SUBTRACTS the shift instead of adding it
 *   - The (+ 26) ensures the result stays positive before modulo
 *   - In disassembly: very similar to func_gamma but sub instead of add
 */
static void func_delta(char *text, int shift)
{
    shift = ((shift % 26) + 26) % 26;

    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            text[i] = 'a' + (text[i] - 'a' - shift + 26) % 26;
        } else if (text[i] >= 'A' && text[i] <= 'Z') {
            text[i] = 'A' + (text[i] - 'A' - shift + 26) % 26;
        }
    }
}

/* =============================================================================
 * FUNCTION 5: Email Address Validator
 * =============================================================================
 * HINTS FOR IDENTIFICATION:
 *   - Calls strlen() (visible in ltrace)
 *   - Searches for '@' character (0x40) — a very distinctive constant
 *   - Searches for '.' character (0x2E) after the '@'
 *   - Multiple validation checks with early returns (many branch instructions)
 *   - Checks isalnum() or similar character class tests
 *   - Returns 0 or 1 (boolean result)
 *   - The '@' and '.' searches are dead giveaways for email validation
 */
static int func_epsilon(const char *input)
{
    size_t len = strlen(input);

    /* Must be at least a@b.c (5 chars) */
    if (len < 5 || len > 254) {
        return 0;
    }

    /* Find '@' */
    const char *at = NULL;
    int at_count = 0;
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '@') {
            at = &input[i];
            at_count++;
        }
    }

    /* Must have exactly one '@' */
    if (at_count != 1 || at == NULL) {
        return 0;
    }

    /* '@' can't be first or last character */
    if (at == input || at == input + len - 1) {
        return 0;
    }

    /* Must have a '.' after '@' but not immediately after */
    const char *dot = NULL;
    for (const char *p = at + 2; *p; p++) {
        if (*p == '.') {
            dot = p;
        }
    }

    if (dot == NULL) {
        return 0;
    }

    /* Must have at least 2 chars after the last dot (e.g., .com, .io) */
    if (strlen(dot + 1) < 2) {
        return 0;
    }

    /* Check that local part has valid characters */
    for (const char *p = input; p < at; p++) {
        if (!isalnum((unsigned char)*p) && *p != '.' && *p != '_' && *p != '-') {
            return 0;
        }
    }

    return 1;  /* Valid */
}

/* =============================================================================
 * FUNCTION 6: Run-Length Encoding (RLE) Compression
 * =============================================================================
 * HINTS FOR IDENTIFICATION:
 *   - Reads through input string sequentially
 *   - Counts consecutive identical characters
 *   - Calls sprintf() or writes formatted output (visible in ltrace)
 *   - Output format: character followed by count (e.g., "aaa" → "a3")
 *   - Look for the counting loop and the write pattern
 *   - The sprintf format string "%c%d" might be visible via strings
 */
static void func_zeta(const char *input, char *output, size_t out_size)
{
    size_t out_pos = 0;
    size_t in_len = strlen(input);

    for (size_t i = 0; i < in_len && out_pos < out_size - 1;) {
        char current = input[i];
        int count = 1;

        /* Count consecutive identical characters */
        while (i + count < in_len && input[i + count] == current) {
            count++;
        }

        /* Write character and count to output */
        if (count > 1) {
            int written = snprintf(output + out_pos, out_size - out_pos,
                                   "%c%d", current, count);
            if (written > 0) {
                out_pos += (size_t)written;
            }
        } else {
            if (out_pos < out_size - 1) {
                output[out_pos++] = current;
            }
        }

        i += count;
    }

    output[out_pos] = '\0';
}

/* =============================================================================
 * MAIN — Demonstrates all functions
 * =============================================================================
 * When you run the binary, observe the output carefully.
 * Each section exercises one of the functions above.
 * The output itself gives clues about what each function does.
 */
int main(void)
{
    printf("=== Function Finder Challenge ===\n");
    printf("This program uses 6 mystery functions.\n");
    printf("Can you identify what each one does?\n\n");

    /* --- Function 1: Sorting --- */
    printf("--- Function Alpha ---\n");
    int numbers[] = {64, 34, 25, 12, 22, 11, 90};
    int n = (int)(sizeof(numbers) / sizeof(numbers[0]));

    printf("Before: ");
    for (int i = 0; i < n; i++) printf("%d ", numbers[i]);
    printf("\n");

    func_alpha(numbers, n);

    printf("After:  ");
    for (int i = 0; i < n; i++) printf("%d ", numbers[i]);
    printf("\n\n");

    /* --- Function 2: Searching --- */
    printf("--- Function Beta ---\n");
    int target = 25;
    int result = func_beta(numbers, n, target);
    printf("Looking for %d: found at position %d\n", target, result);

    target = 99;
    result = func_beta(numbers, n, target);
    printf("Looking for %d: found at position %d\n\n", target, result);

    /* --- Function 3 & 4: Encode/Decode --- */
    printf("--- Functions Gamma and Delta ---\n");
    char message[] = "The Quick Brown Fox Jumps Over The Lazy Dog";
    int shift = 7;

    printf("Original:  %s\n", message);
    func_gamma(message, shift);
    printf("After Gamma (shift=%d): %s\n", shift, message);
    func_delta(message, shift);
    printf("After Delta (shift=%d): %s\n\n", shift, message);

    /* --- Function 5: Validation --- */
    printf("--- Function Epsilon ---\n");
    const char *test_emails[] = {
        "user@example.com",
        "bad-email",
        "also@bad",
        "good.name@domain.org",
        "@missing.local",
        "missing@",
        "two@@signs.com",
        "valid_user-name@sub.domain.co",
    };
    int num_emails = (int)(sizeof(test_emails) / sizeof(test_emails[0]));

    for (int i = 0; i < num_emails; i++) {
        int valid = func_epsilon(test_emails[i]);
        printf("  %-30s -> %s\n", test_emails[i], valid ? "VALID" : "INVALID");
    }
    printf("\n");

    /* --- Function 6: Compression --- */
    printf("--- Function Zeta ---\n");
    const char *test_strings[] = {
        "aaabbbcccc",
        "aabcccdddd",
        "abcdef",
        "aaaaaaaaaa",
        "Hello World",
    };
    int num_strings = (int)(sizeof(test_strings) / sizeof(test_strings[0]));

    char compressed[256];
    for (int i = 0; i < num_strings; i++) {
        func_zeta(test_strings[i], compressed, sizeof(compressed));
        printf("  %-20s -> %s\n", test_strings[i], compressed);
    }
    printf("\n");

    printf("=== Challenge Complete ===\n");
    printf("Can you name all 6 functions?\n");
    printf("Hint: use strings, ltrace, objdump, and gdb on the stripped binary.\n");

    return 0;
}
