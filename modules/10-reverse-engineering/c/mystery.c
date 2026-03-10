/*
 * =============================================================================
 * MYSTERY PROGRAM — Module 10: Reverse Engineering a C Binary
 * =============================================================================
 *
 * YOUR MISSION:
 *   1. Compile and strip this:  gcc -O0 -o mystery mystery.c && strip mystery
 *   2. Run it:                  ./mystery
 *   3. Figure out WHAT it does WITHOUT reading this source code!
 *
 * APPROACH:
 *   - First, run it and observe the output
 *   - Use 'strings mystery' to find embedded text
 *   - Use 'strace ./mystery' to see system calls (what files/resources it accesses)
 *   - Use 'ltrace ./mystery' to see library calls (printf, strlen, etc.)
 *   - Use 'objdump -d mystery' to read the disassembly
 *   - Use 'gdb ./mystery' to step through and inspect variables
 *
 * WHAT THIS PROGRAM DOES (SPOILER — don't read until you've tried RE):
 *
 *   This program implements a ROT13 cipher followed by a character reversal.
 *   It encodes a hidden message, then decodes it back, printing both forms.
 *   It also reads from stdin if data is available, encoding user input.
 *
 *   The program structure:
 *     - transform_alpha(): ROT13 on a single character
 *     - process_buffer(): Apply ROT13 to entire string
 *     - reverse_string(): Reverse a string in-place
 *     - generate_output(): Combine operations on the hidden message
 *     - main(): Orchestrate everything
 *
 *   When stripped, all function names disappear. The student must identify
 *   each function by analyzing:
 *     - What libc functions it calls (visible via ltrace)
 *     - What constants it uses (13, 26 for ROT13 are telltale)
 *     - What the function's control flow looks like in disassembly
 *
 * Compile cleanly with: gcc -Wall -Wextra -o mystery mystery.c
 * =============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

/*
 * ROT13 transformation on a single character.
 * Only affects letters (a-z, A-Z); leaves everything else unchanged.
 *
 * In the disassembly, look for:
 *   - Comparisons with 'a' (0x61), 'z' (0x7A), 'A' (0x41), 'Z' (0x5A)
 *   - Addition/subtraction of 13 (0x0D) — the ROT13 constant
 *   - Modular arithmetic with 26 (0x1A) — the alphabet size
 *
 * These constants are dead giveaways that this is a rotation cipher.
 */
static char transform_alpha(char c)
{
    if (c >= 'a' && c <= 'z') {
        return 'a' + (c - 'a' + 13) % 26;
    }
    if (c >= 'A' && c <= 'Z') {
        return 'A' + (c - 'A' + 13) % 26;
    }
    return c;
}

/*
 * Apply ROT13 to every character in a buffer.
 *
 * In ltrace, you'll see this calls strlen() once, then loops.
 * In strace, this is invisible (pure computation, no syscalls).
 */
static void process_buffer(char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        buf[i] = transform_alpha(buf[i]);
    }
}

/*
 * Reverse a string in-place.
 *
 * In the disassembly, look for:
 *   - Two index variables moving toward each other
 *   - A swap pattern (temp = a; a = b; b = temp)
 *   - strlen() call at the beginning
 */
static void reverse_string(char *str)
{
    size_t len = strlen(str);
    if (len <= 1) return;

    size_t left = 0;
    size_t right = len - 1;
    while (left < right) {
        char temp = str[left];
        str[left] = str[right];
        str[right] = temp;
        left++;
        right--;
    }
}

/*
 * Generate a visual separator line.
 * In ltrace, you'll see repeated putchar(0x2D) calls (0x2D = '-').
 */
static void print_separator(int width, char ch)
{
    for (int i = 0; i < width; i++) {
        putchar(ch);
    }
    putchar('\n');
}

/*
 * The core function that processes the hidden message.
 * Uses multiple transformations to obscure the original text.
 */
static void generate_output(void)
{
    /* The hidden message, pre-encoded with ROT13.
     * When ROT13 is applied, it decodes to the readable message.
     * ROT13("Uryyb sebz gur bgure fvqr!") = "Hello from the other side!"
     * ROT13("Lbh penpxrq gur zlfgrel cebtenz.") = "You cracked the mystery program."
     */
    char encoded_msg1[] = "Uryyb sebz gur bgure fvqr!";
    char encoded_msg2[] = "Lbh penpxrq gur zlfgrel cebtenz.";

    /* Working copies */
    char work1[128];
    char work2[128];
    strncpy(work1, encoded_msg1, sizeof(work1) - 1);
    work1[sizeof(work1) - 1] = '\0';
    strncpy(work2, encoded_msg2, sizeof(work2) - 1);
    work2[sizeof(work2) - 1] = '\0';

    /* Step 1: Show the encoded form */
    print_separator(50, '=');
    printf("Encoded: %s\n", work1);

    /* Step 2: Apply ROT13 to decode */
    process_buffer(work1, strlen(work1));
    printf("Decoded: %s\n", work1);

    /* Step 3: Reverse the decoded message */
    reverse_string(work1);
    printf("Reversed: %s\n", work1);

    /* Step 4: Reverse again (back to decoded) and show */
    reverse_string(work1);
    printf("Final: %s\n", work1);

    print_separator(50, '-');

    /* Process second message */
    process_buffer(work2, strlen(work2));
    printf("%s\n", work2);

    print_separator(50, '=');
}

/*
 * Process a character counter — counts occurrences of each ASCII character.
 * This function adds complexity for the student to analyze.
 *
 * In the disassembly, look for:
 *   - An array of 256 elements (one per ASCII value)
 *   - Array indexing by character value
 *   - A counting loop
 */
static void analyze_text(const char *text)
{
    int freq[256];
    memset(freq, 0, sizeof(freq));

    size_t len = strlen(text);
    for (size_t i = 0; i < len; i++) {
        freq[(unsigned char)text[i]]++;
    }

    printf("Character frequency analysis (%zu chars):\n", len);
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0 && isprint(i)) {
            printf("  '%c' (0x%02X): %d\n", i, i, freq[i]);
        }
    }
}

int main(int argc, char *argv[])
{
    /* Generate the mystery output */
    generate_output();

    /* If a command-line argument is provided, encode it with ROT13 */
    if (argc > 1) {
        printf("\nProcessing your input:\n");
        print_separator(50, '-');

        char input_buf[512];
        strncpy(input_buf, argv[1], sizeof(input_buf) - 1);
        input_buf[sizeof(input_buf) - 1] = '\0';

        printf("Original:  %s\n", input_buf);

        /* Analyze character frequency */
        analyze_text(input_buf);

        /* ROT13 encode */
        process_buffer(input_buf, strlen(input_buf));
        printf("Encoded:   %s\n", input_buf);

        /* ROT13 again = original (ROT13 is its own inverse) */
        process_buffer(input_buf, strlen(input_buf));
        printf("Decoded:   %s\n", input_buf);

        /* Reverse */
        reverse_string(input_buf);
        printf("Reversed:  %s\n", input_buf);

        print_separator(50, '-');
    } else {
        printf("\nTip: pass a string argument to encode it.\n");
        printf("Example: ./mystery \"Hello World\"\n");
    }

    /* Check if stdin has data (piped input) */
    if (!isatty(STDIN_FILENO)) {
        char line[1024];
        printf("\nReading from stdin:\n");
        while (fgets(line, sizeof(line), stdin)) {
            size_t line_len = strlen(line);
            if (line_len > 0 && line[line_len - 1] == '\n') {
                line[line_len - 1] = '\0';
                line_len--;
            }
            process_buffer(line, line_len);
            printf("  -> %s\n", line);
        }
    }

    return 0;
}
