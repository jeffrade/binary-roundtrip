/*
 * =============================================================================
 * fast_math.c — A Ruby C Extension for High-Performance Math
 * =============================================================================
 *
 * PURPOSE:
 *   This C extension demonstrates how Ruby gems achieve native performance.
 *   Instead of executing YARV bytecode, this code runs as compiled machine
 *   code — the same as any C program.
 *
 * HOW C EXTENSIONS WORK:
 *   1. Ruby calls require('./fast_math')
 *   2. The dynamic linker loads fast_math.so into the process
 *   3. Ruby calls Init_fast_math() — our initialization function
 *   4. Init_fast_math() registers C functions as Ruby methods
 *   5. When Ruby code calls FastMath.fibonacci(n), it directly calls our C function
 *   6. No YARV bytecode is involved — it's pure native execution
 *
 * THE RUBY C API:
 *   Ruby objects in C are represented as VALUE (which is an unsigned long).
 *   Small integers are stored directly in the VALUE (tagged pointers).
 *   Everything else is a pointer to a heap-allocated structure.
 *
 *   Key functions:
 *     NUM2LONG(val)   — Convert Ruby Integer to C long
 *     LONG2NUM(n)     — Convert C long to Ruby Integer
 *     rb_ary_entry()  — Get element from Ruby array
 *     RARRAY_LEN()    — Get length of Ruby array
 *
 * IMPORTANT:
 *   C extensions bypass the GVL for their own C code, BUT:
 *   - You must NOT call Ruby API functions without the GVL
 *   - Use rb_thread_call_without_gvl() for long-running C work
 *   - This is how gems like pg do non-blocking database queries
 *
 * COMPILE:
 *   ruby extconf.rb && make
 *
 * THIS IS HOW THESE GEMS WORK:
 *   - nokogiri: wraps libxml2 (C) for XML/HTML parsing
 *   - pg: wraps libpq (C) for PostgreSQL client
 *   - mysql2: wraps libmysqlclient (C) for MySQL client
 *   - grpc: wraps gRPC core (C++) for RPC
 *   - ffi: generic foreign function interface to call any C library
 * =============================================================================
 */

#include <ruby.h>
/* ruby.h includes the entire Ruby C API:
 *   - VALUE type (the universal Ruby object type)
 *   - rb_define_module, rb_define_method, etc.
 *   - Type conversion macros (NUM2LONG, etc.)
 *   - Exception raising (rb_raise)
 *   - GC interaction (rb_gc_mark, etc.)
 */

/*
 * ---------------------------------------------------------------------------
 * FastMath.fibonacci(n) — Iterative Fibonacci in C
 * ---------------------------------------------------------------------------
 * This is dramatically faster than the pure Ruby version because:
 *   1. No YARV bytecode interpretation overhead
 *   2. C long arithmetic compiles to single CPU instructions
 *   3. The loop compiles to a tight set of machine instructions
 *   4. No method dispatch (no send instruction, no method cache lookup)
 *   5. No GC pressure (no Ruby objects created during computation)
 *
 * For fibonacci(40):
 *   - Pure Ruby (recursive): ~10-20 seconds
 *   - Pure Ruby (iterative): ~0.001 seconds
 *   - C extension (iterative): ~0.0001 seconds
 *   - The recursive version's slowness is algorithmic, not language
 *
 * Note: We use the iterative algorithm here. The speed difference between
 * C and Ruby is in the per-iteration overhead, not the algorithm.
 * ---------------------------------------------------------------------------
 */
static VALUE
fast_fibonacci(VALUE self, VALUE rb_n)
{
    /* Convert Ruby Integer to C long
     * NUM2LONG handles:
     *   - Fixnum (small integers stored in VALUE directly)
     *   - Bignum (arbitrary precision integers on the heap)
     * If rb_n is not a number, it raises TypeError automatically.
     */
    long n = NUM2LONG(rb_n);

    /* Validate input */
    if (n < 0) {
        /* rb_raise throws a Ruby exception from C code.
         * This unwinds the C stack and returns control to Ruby's
         * exception handling mechanism. */
        rb_raise(rb_eArgError, "fibonacci requires non-negative integer, got %ld", n);
    }

    if (n <= 1) {
        return rb_n;  /* Can return the original VALUE directly */
    }

    /* Iterative fibonacci — pure C arithmetic.
     * This loop compiles to about 5-6 machine instructions:
     *   mov, add, mov, cmp, jl (loop)
     * Compare to Ruby's YARV version which would be:
     *   getlocal, getlocal, opt_plus, setlocal, ... (many more) */
    long a = 0, b = 1, temp;
    for (long i = 2; i <= n; i++) {
        temp = a + b;
        a = b;
        b = temp;
    }

    /* Convert C long back to Ruby Integer.
     * LONG2NUM automatically chooses Fixnum or Bignum representation. */
    return LONG2NUM(b);
}

/*
 * ---------------------------------------------------------------------------
 * FastMath.sum(array) — Sum all elements of a Ruby Array in C
 * ---------------------------------------------------------------------------
 * This shows how to work with Ruby collections from C.
 * We directly access the array's internal buffer — no iterator overhead.
 *
 * Speed advantage comes from:
 *   1. Direct memory access to array elements (no bounds checking per element)
 *   2. No block/closure allocation (Ruby's .sum creates a block internally)
 *   3. No method dispatch per element (no .+ method lookup)
 *   4. Loop compiles to tight machine code
 * ---------------------------------------------------------------------------
 */
static VALUE
fast_sum(VALUE self, VALUE rb_array)
{
    /* Check that we received an Array */
    Check_Type(rb_array, T_ARRAY);

    long len = RARRAY_LEN(rb_array);  /* Get array length */
    long sum = 0;

    for (long i = 0; i < len; i++) {
        /* rb_ary_entry safely gets element at index i.
         * It handles bounds checking and returns nil for out-of-bounds.
         * For maximum speed, you could use RARRAY_AREF (no bounds check). */
        VALUE element = rb_ary_entry(rb_array, i);

        /* Convert each element to a C long and accumulate.
         * This will raise TypeError if an element isn't numeric. */
        sum += NUM2LONG(element);
    }

    return LONG2NUM(sum);
}

/*
 * ---------------------------------------------------------------------------
 * FastMath.dot_product(array1, array2) — Dot product of two arrays
 * ---------------------------------------------------------------------------
 * A slightly more complex example showing two-array processing.
 * This is common in numerical computing (vectors, matrices).
 * ---------------------------------------------------------------------------
 */
static VALUE
fast_dot_product(VALUE self, VALUE rb_arr1, VALUE rb_arr2)
{
    Check_Type(rb_arr1, T_ARRAY);
    Check_Type(rb_arr2, T_ARRAY);

    long len1 = RARRAY_LEN(rb_arr1);
    long len2 = RARRAY_LEN(rb_arr2);

    if (len1 != len2) {
        rb_raise(rb_eArgError, "arrays must be same length (%ld vs %ld)", len1, len2);
    }

    long result = 0;
    for (long i = 0; i < len1; i++) {
        long a = NUM2LONG(rb_ary_entry(rb_arr1, i));
        long b = NUM2LONG(rb_ary_entry(rb_arr2, i));
        result += a * b;
    }

    return LONG2NUM(result);
}

/*
 * ---------------------------------------------------------------------------
 * FastMath.is_prime?(n) — Primality test in C
 * ---------------------------------------------------------------------------
 * Another CPU-bound task where C dramatically outperforms Ruby.
 * Uses trial division up to sqrt(n).
 * ---------------------------------------------------------------------------
 */
static VALUE
fast_is_prime(VALUE self, VALUE rb_n)
{
    long n = NUM2LONG(rb_n);

    if (n < 2) return Qfalse;  /* Qfalse = Ruby's false */
    if (n < 4) return Qtrue;   /* Qtrue = Ruby's true */
    if (n % 2 == 0) return Qfalse;
    if (n % 3 == 0) return Qfalse;

    /* Trial division: check 6k +/- 1 up to sqrt(n)
     * This is the same algorithm in Ruby, but each comparison and
     * modulo operation is a single CPU instruction instead of
     * multiple YARV bytecodes. */
    for (long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return Qfalse;
        }
    }

    return Qtrue;
}

/*
 * =============================================================================
 * Init_fast_math — Extension Initialization (REQUIRED)
 * =============================================================================
 * This function is called by Ruby when you do: require './fast_math'
 *
 * The naming convention is MANDATORY:
 *   - File: fast_math.c → Library: fast_math.so
 *   - Init function: Init_fast_math  (Init_ + filename without extension)
 *
 * If the name doesn't match, Ruby won't find the init function and
 * require will fail with a LoadError.
 *
 * Inside, we:
 *   1. Create a Ruby module (FastMath)
 *   2. Define methods on that module
 *   3. Each method points to a C function
 *
 * After this, Ruby code can call: FastMath.fibonacci(10)
 * And it will directly invoke our fast_fibonacci() C function.
 * =============================================================================
 */
void
Init_fast_math(void)
{
    /* Create the FastMath module.
     * rb_define_module creates a new Module constant at the top level.
     * This is equivalent to: module FastMath; end  in Ruby */
    VALUE mod = rb_define_module("FastMath");

    /* Register our C functions as module methods.
     *
     * rb_define_module_function(module, ruby_name, c_function, arg_count)
     *
     * Parameters:
     *   module     — the module to add the method to
     *   ruby_name  — the name Ruby code will use to call it
     *   c_function — pointer to our C function
     *   arg_count  — number of Ruby arguments (not counting self)
     *
     * The C function signature must be:
     *   VALUE function_name(VALUE self, VALUE arg1, VALUE arg2, ...)
     * where self is the receiver (the module itself for module functions).
     */
    rb_define_module_function(mod, "fibonacci",   fast_fibonacci,   1);
    rb_define_module_function(mod, "sum",          fast_sum,         1);
    rb_define_module_function(mod, "dot_product",  fast_dot_product, 2);
    rb_define_module_function(mod, "is_prime?",    fast_is_prime,    1);

    /* That's it! After this function returns, Ruby code can call:
     *   FastMath.fibonacci(40)
     *   FastMath.sum([1, 2, 3, 4, 5])
     *   FastMath.dot_product([1,2,3], [4,5,6])
     *   FastMath.is_prime?(997)
     */
}
