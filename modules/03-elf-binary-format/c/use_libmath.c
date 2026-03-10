/*
 * use_libmath.c — A program that uses our custom shared library (libmath.so)
 * ============================================================================
 *
 * MODULE 3: ELF Binary Format
 * EXERCISE: Link against a shared library you built yourself
 *
 * BUILD (after building libmath.so):
 *   gcc -Wall -Wextra -o use_libmath use_libmath.c -L. -lmath
 *
 *   -L.     tells the linker to look in the current directory for libraries
 *   -lmath  tells the linker to link against "libmath.so" (lib + math + .so)
 *
 * RUN:
 *   ./use_libmath
 *   # ERROR! "error while loading shared libraries: libmath.so: cannot open"
 *
 *   WHY? The dynamic linker (ld-linux) doesn't know where libmath.so is!
 *   At link time, -L. told the COMPILER where to find it.
 *   At run time, the DYNAMIC LINKER needs to find it too.
 *
 * THREE WAYS TO FIX THIS:
 *
 *   Option 1: LD_LIBRARY_PATH (temporary, good for development)
 *     LD_LIBRARY_PATH=. ./use_libmath
 *
 *   Option 2: rpath (baked into the binary at compile time)
 *     gcc -Wall -Wextra -o use_libmath use_libmath.c -L. -lmath -Wl,-rpath,.
 *     ./use_libmath   # Works without LD_LIBRARY_PATH!
 *     readelf -d use_libmath | grep RPATH   # See the baked-in path
 *
 *   Option 3: Install to system library path (for production)
 *     sudo cp libmath.so /usr/local/lib/
 *     sudo ldconfig   # Rebuild the library cache
 *     ./use_libmath   # Now it finds the library system-wide
 *
 * WHAT TO OBSERVE:
 *   ldd use_libmath
 *     # Shows libmath.so => not found (until you fix the path)
 *     # After LD_LIBRARY_PATH=.: shows libmath.so => ./libmath.so
 *
 *   readelf -d use_libmath | grep NEEDED
 *     # Shows: NEEDED libmath.so  (and libc.so.6)
 *
 *   nm -D use_libmath | grep ' U '
 *     # Shows UNDEFINED symbols — functions we need from shared libraries
 *     # You'll see square, cube, factorial, fibonacci, is_prime
 *     # These get resolved at runtime by the dynamic linker
 *
 * KEY CONCEPT — The Search Order for Shared Libraries:
 *   1. DT_RPATH in the binary (deprecated, but still works)
 *   2. LD_LIBRARY_PATH environment variable
 *   3. DT_RUNPATH in the binary (modern replacement for RPATH)
 *   4. /etc/ld.so.cache (system library cache, built by ldconfig)
 *   5. /lib and /usr/lib (default system paths)
 */

#include <stdio.h>
#include "libmath.h"

int main(void) {
    printf("=== Using Our Custom Shared Library (libmath.so) ===\n\n");

    /* Test square */
    printf("square(7) = %ld\n", square(7));
    printf("square(12) = %ld\n", square(12));

    /* Test cube */
    printf("cube(3) = %ld\n", cube(3));
    printf("cube(5) = %ld\n", cube(5));

    /* Test factorial */
    printf("\nFactorials:\n");
    for (int i = 0; i <= 10; i++) {
        printf("  %2d! = %ld\n", i, factorial(i));
    }

    /* Test fibonacci */
    printf("\nFibonacci sequence:\n  ");
    for (int i = 0; i <= 15; i++) {
        printf("%ld ", fibonacci(i));
    }
    printf("\n");

    /* Test is_prime */
    printf("\nPrimes up to 50:\n  ");
    for (int i = 2; i <= 50; i++) {
        if (is_prime(i)) {
            printf("%d ", i);
        }
    }
    printf("\n");

    printf("\n=== Exploration Commands ===\n");
    printf("Examine the binary's dependencies:\n");
    printf("  ldd use_libmath                          # Show shared library deps\n");
    printf("  readelf -d use_libmath | grep NEEDED     # Required libraries\n");
    printf("  nm -D use_libmath | grep ' U '           # Undefined (external) symbols\n\n");

    printf("Examine the shared library itself:\n");
    printf("  file libmath.so                          # ELF shared object\n");
    printf("  nm -D libmath.so                         # Exported symbols\n");
    printf("  readelf -S libmath.so                    # Sections in the .so\n");
    printf("  objdump -d libmath.so                    # Disassemble library code\n\n");

    printf("Watch dynamic linking in action:\n");
    printf("  LD_DEBUG=libs LD_LIBRARY_PATH=. ./use_libmath 2>&1 | head -20\n");
    printf("  LD_DEBUG=symbols LD_LIBRARY_PATH=. ./use_libmath 2>&1 | head -30\n\n");

    printf("Compare PIC vs non-PIC code:\n");
    printf("  gcc -S libmath.c -o libmath_nopic.s\n");
    printf("  gcc -S -fPIC libmath.c -o libmath_pic.s\n");
    printf("  diff libmath_nopic.s libmath_pic.s\n");

    return 0;
}
