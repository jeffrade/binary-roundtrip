/*
 * =============================================================================
 * CRACKME CHALLENGE — Module 10: Reverse Engineering a C Binary
 * =============================================================================
 *
 * YOUR MISSION:
 *   1. Compile this program:    gcc -O0 -o crackme crackme.c
 *   2. Strip the binary:        strip crackme
 *   3. Try to find the password WITHOUT reading this source code!
 *
 * TOOLS YOU CAN USE:
 *   - strings crackme          — look for readable strings in the binary
 *   - objdump -d crackme       — disassemble the binary
 *   - gdb ./crackme            — step through execution
 *   - strace ./crackme test    — trace system calls
 *   - ltrace ./crackme test    — trace library calls
 *
 * HINT: The password is encoded, not stored in plaintext.
 *       Look at the disassembly to understand how the check works.
 *
 * Compile cleanly with: gcc -Wall -Wextra -o crackme crackme.c
 * =============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * The password is NOT stored as a plain string — running `strings` on the
 * stripped binary won't directly reveal it.
 *
 * Instead, each character of the password is stored as an encoded value.
 * The encoding: each byte is XOR'd with a key, then has an offset added.
 *
 * To reverse engineer this, you need to:
 *   1. Find the encoded array in the disassembly (or via gdb)
 *   2. Figure out the XOR key and offset
 *   3. Reverse the transformation to get the plaintext password
 *
 * The password is: "OpenSesame42"
 *
 * Encoding: encoded[i] = (password[i] ^ XOR_KEY) + OFFSET
 * Decoding: password[i] = (encoded[i] - OFFSET) ^ XOR_KEY
 */

/* XOR key and offset — these will appear as immediates in the disassembly */
#define XOR_KEY  0x5A
#define OFFSET   0x13

/*
 * Pre-computed encoded password bytes.
 * Each value = (char ^ 0x5A) + 0x13
 *
 * 'O' = 0x4F → 0x4F ^ 0x5A = 0x15 → 0x15 + 0x13 = 0x28
 * 'p' = 0x70 → 0x70 ^ 0x5A = 0x2A → 0x2A + 0x13 = 0x3D
 * 'e' = 0x65 → 0x65 ^ 0x5A = 0x3F → 0x3F + 0x13 = 0x52
 * 'n' = 0x6E → 0x6E ^ 0x5A = 0x34 → 0x34 + 0x13 = 0x47
 * 'S' = 0x53 → 0x53 ^ 0x5A = 0x09 → 0x09 + 0x13 = 0x1C
 * 'e' = 0x65 → 0x65 ^ 0x5A = 0x3F → 0x3F + 0x13 = 0x52
 * 's' = 0x73 → 0x73 ^ 0x5A = 0x29 → 0x29 + 0x13 = 0x3C
 * 'a' = 0x61 → 0x61 ^ 0x5A = 0x3B → 0x3B + 0x13 = 0x4E
 * 'm' = 0x6D → 0x6D ^ 0x5A = 0x37 → 0x37 + 0x13 = 0x4A
 * 'e' = 0x65 → 0x65 ^ 0x5A = 0x3F → 0x3F + 0x13 = 0x52
 * '4' = 0x34 → 0x34 ^ 0x5A = 0x6E → 0x6E + 0x13 = 0x81
 * '2' = 0x32 → 0x32 ^ 0x5A = 0x68 → 0x68 + 0x13 = 0x7B
 */
static const unsigned char encoded_pw[] = {
    0x28, 0x3D, 0x52, 0x47, 0x1C, 0x52, 0x3C, 0x4E,
    0x4A, 0x52, 0x81, 0x7B
};

static const int encoded_len = sizeof(encoded_pw) / sizeof(encoded_pw[0]);

/*
 * Decode a single character from the encoded password.
 * In the disassembly, you'll see:
 *   - Load byte from encoded array
 *   - Subtract OFFSET (0x13)
 *   - XOR with XOR_KEY (0x5A)
 * These constants are the key to cracking it!
 */
static char decode_char(unsigned char encoded)
{
    return (char)((encoded - OFFSET) ^ XOR_KEY);
}

/*
 * Verify the password using constant-time comparison.
 * This avoids timing side-channel attacks (where you measure how long
 * the check takes to determine how many characters matched).
 *
 * In the disassembly, look for:
 *   - A loop comparing each character
 *   - XOR and subtraction operations (the decoding)
 *   - An OR accumulator (constant-time comparison pattern)
 */
static int verify_password(const char *input)
{
    int input_len = (int)strlen(input);
    int diff = 0;

    /* First check: length must match */
    diff |= (input_len ^ encoded_len);

    /* Compare each character against the decoded password.
     * We always check ALL characters (constant time) — no early exit.
     * The |= accumulates any differences. If diff is 0 at the end,
     * all characters matched. */
    int check_len = input_len < encoded_len ? input_len : encoded_len;
    for (int i = 0; i < check_len; i++) {
        char decoded = decode_char(encoded_pw[i]);
        diff |= (input[i] ^ decoded);
    }

    return diff == 0;
}

/*
 * A red herring function — this looks interesting in the disassembly
 * but doesn't actually contribute to the password check.
 * It's here to make reverse engineering slightly more challenging.
 */
static unsigned int compute_checksum(const char *str)
{
    unsigned int hash = 0x811C9DC5;  /* FNV-1a offset basis — looks suspicious! */
    while (*str) {
        hash ^= (unsigned char)*str;
        hash *= 0x01000193;  /* FNV-1a prime — another suspicious constant */
        str++;
    }
    return hash;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <password>\n", argv[0]);
        fprintf(stderr, "Can you find the secret password?\n");
        return 1;
    }

    /* Red herring: compute a checksum but don't use it for the real check */
    volatile unsigned int checksum = compute_checksum(argv[1]);
    (void)checksum;  /* Suppress unused warning, but 'volatile' keeps it in binary */

    if (verify_password(argv[1])) {
        printf("Access Granted!\n");
        printf("Congratulations, you cracked it!\n");
        return 0;
    } else {
        printf("Wrong password!\n");
        printf("Hint: the password has %d characters.\n", encoded_len);
        return 1;
    }
}
