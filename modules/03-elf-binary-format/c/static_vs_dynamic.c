/*
 * static_vs_dynamic.c — Explore dynamic linking via printf and sqrt
 * ============================================================================
 *
 * MODULE 3: ELF Binary Format
 * EXERCISE: Understand dynamic linking, PLT, GOT, and shared libraries
 *
 * CONCEPT:
 *   Most programs don't contain their own copy of printf or sqrt. Instead,
 *   they link DYNAMICALLY against shared libraries (.so files) at runtime.
 *   The ELF binary contains special sections that make this work:
 *
 *   .plt    (Procedure Linkage Table) — Trampolines for calling library functions
 *   .got    (Global Offset Table)     — Pointers to actual function addresses
 *   .dynamic                          — Info about which shared libraries are needed
 *   .interp                           — Path to the dynamic linker (ld-linux-x86-64.so.2)
 *
 * BUILD:
 *   gcc -Wall -Wextra -o static_vs_dynamic static_vs_dynamic.c -lm
 *
 *   Note: -lm links against libm (the math library) for sqrt().
 *   printf() comes from libc, which is linked automatically.
 *
 * COMPARE STATIC vs DYNAMIC:
 *   gcc -Wall -Wextra -o dynamic_binary static_vs_dynamic.c -lm
 *   gcc -Wall -Wextra -static -o static_binary static_vs_dynamic.c -lm
 *   ls -lh dynamic_binary static_binary
 *   # Static binary is MUCH larger — it contains all library code internally.
 *
 *   ldd dynamic_binary    # Shows shared library dependencies
 *   ldd static_binary     # "not a dynamic executable"
 *
 *   file dynamic_binary   # "dynamically linked"
 *   file static_binary    # "statically linked"
 *
 * WHAT TO OBSERVE:
 *   1. `ldd` output shows which .so files are needed and where they live
 *   2. `readelf -d` shows the NEEDED entries (required shared libraries)
 *   3. `readelf -S` shows .plt, .got, .got.plt sections
 *   4. `objdump -d -j .plt` shows the PLT trampolines
 *   5. `objdump -R` shows relocation entries (addresses to be patched at load time)
 */

#include <stdio.h>
#include <math.h>
#include <string.h>

/*
 * HOW DYNAMIC LINKING WORKS — The PLT/GOT Dance:
 * ================================================
 *
 * When your program calls printf() for the FIRST time:
 *
 * 1. Your code calls printf@plt (a stub in the PLT section)
 * 2. The PLT stub jumps to an address stored in the GOT
 * 3. Initially, the GOT points BACK to the PLT (to the resolver)
 * 4. The resolver asks the dynamic linker: "Where is the real printf?"
 * 5. The dynamic linker finds printf in libc.so, patches the GOT entry
 * 6. Now the GOT points directly to the real printf
 * 7. Future calls to printf@plt jump through GOT straight to the real printf
 *
 * This is called "lazy binding" — symbols are resolved on first use.
 * You can disable it with LD_BIND_NOW=1 to resolve all symbols at startup.
 *
 * OBSERVE the PLT/GOT in action:
 *   objdump -d -j .plt static_vs_dynamic       # See PLT stubs
 *   objdump -d static_vs_dynamic | grep '@plt'  # See calls to PLT entries
 *   readelf -r static_vs_dynamic                 # See relocations (GOT patches)
 *
 * ADVANCED: Watch lazy binding happen:
 *   LD_DEBUG=bindings ./static_vs_dynamic 2>&1 | head -30
 *   # Shows each symbol being resolved at runtime!
 */

/*
 * demonstrate_printf — Uses printf (from libc.so, linked automatically)
 *
 * printf is part of the C standard library (libc.so.6 on most Linux systems).
 * You never need to pass -lc to gcc because libc is linked by default.
 *
 * OBSERVE: `nm -D static_vs_dynamic | grep printf`
 *   You'll see: U printf   ('U' = undefined, needs to be resolved at runtime)
 *
 * OBSERVE: `objdump -d static_vs_dynamic | grep 'printf@plt'`
 *   You'll see calls go through the PLT trampoline, not directly to libc.
 */
void demonstrate_printf(void) {
    printf("  printf() is from libc.so (linked automatically by gcc)\n");
    printf("  Format strings like this one go to .rodata\n");

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "  snprintf is also from libc.so");
    printf("  %s\n", buffer);

    printf("  strlen(\"hello\") = %zu — also libc\n", strlen("hello"));
}

/*
 * demonstrate_math — Uses sqrt (from libm.so, needs -lm)
 *
 * sqrt is part of the math library (libm.so), which is separate from libc.
 * You MUST pass -lm to gcc when linking, or you'll get:
 *   undefined reference to `sqrt'
 *
 * WHY is math separate from libc? Historical reasons from Unix:
 *   - Not all programs need math functions
 *   - Floating-point was expensive, sometimes done in software
 *   - Keeping libm separate reduced binary size for simple programs
 *
 * OBSERVE: `ldd static_vs_dynamic`
 *   You'll see BOTH libc.so and libm.so listed as dependencies.
 *
 * OBSERVE: `readelf -d static_vs_dynamic | grep NEEDED`
 *   Shows: NEEDED libm.so.6 and NEEDED libc.so.6
 */
void demonstrate_math(void) {
    double x = 144.0;
    double result = sqrt(x);
    printf("  sqrt(%.1f) = %.1f — from libm.so (requires -lm)\n", x, result);

    double y = 2.0;
    double power = pow(y, 10.0);
    printf("  pow(%.1f, 10) = %.0f — also from libm.so\n", y, power);

    double angle = M_PI / 4.0;  /* M_PI is a constant from math.h */
    printf("  sin(pi/4) = %.6f — also from libm.so\n", sin(angle));
    printf("  cos(pi/4) = %.6f — also from libm.so\n", cos(angle));
}

int main(void) {
    printf("=== Static vs Dynamic Linking Explorer ===\n\n");

    printf("--- printf (from libc.so, auto-linked) ---\n");
    demonstrate_printf();

    printf("\n--- sqrt/pow/sin/cos (from libm.so, needs -lm) ---\n");
    demonstrate_math();

    printf("\n=== How Dynamic Linking Works ===\n");
    printf("1. Your binary does NOT contain printf/sqrt code\n");
    printf("2. Instead, it has PLT stubs (tiny trampolines)\n");
    printf("3. On first call, the dynamic linker resolves the real address\n");
    printf("4. The GOT is patched so future calls go directly to the library\n");

    printf("\n=== Exploration Commands ===\n");
    printf("Build both versions first:\n");
    printf("  gcc -Wall -Wextra -o dynamic_binary static_vs_dynamic.c -lm\n");
    printf("  gcc -Wall -Wextra -static -o static_binary static_vs_dynamic.c -lm\n\n");

    printf("Compare sizes (static is MUCH bigger):\n");
    printf("  ls -lh dynamic_binary static_binary\n\n");

    printf("Show shared library dependencies:\n");
    printf("  ldd dynamic_binary\n");
    printf("  ldd static_binary    # 'not a dynamic executable'\n\n");

    printf("Show required libraries in ELF header:\n");
    printf("  readelf -d dynamic_binary | grep NEEDED\n\n");

    printf("Show the dynamic linker (interpreter):\n");
    printf("  readelf -l dynamic_binary | grep interpreter\n");
    printf("  # Usually /lib64/ld-linux-x86-64.so.2\n\n");

    printf("Examine PLT/GOT (the dynamic linking machinery):\n");
    printf("  objdump -d -j .plt dynamic_binary         # PLT trampolines\n");
    printf("  objdump -d dynamic_binary | grep '@plt'   # Calls through PLT\n");
    printf("  readelf -r dynamic_binary                  # Relocations\n");
    printf("  objdump -R dynamic_binary                  # Dynamic relocations\n\n");

    printf("Watch lazy binding happen in real-time:\n");
    printf("  LD_DEBUG=bindings ./dynamic_binary 2>&1 | head -40\n\n");

    printf("Compare symbol tables:\n");
    printf("  nm dynamic_binary | wc -l    # Fewer symbols\n");
    printf("  nm static_binary | wc -l     # MANY more symbols (all of libc!)\n");

    return 0;
}
