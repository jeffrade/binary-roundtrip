#!/usr/bin/env ruby
# =============================================================================
# extconf.rb — Build Configuration for Ruby C Extension
# =============================================================================
#
# PURPOSE:
#   This file tells Ruby how to compile a C extension into a shared library
#   (.so on Linux, .bundle on macOS) that Ruby can load with `require`.
#
# HOW IT WORKS:
#   1. You run: ruby extconf.rb
#   2. It probes the system (compiler, Ruby headers location, platform)
#   3. It generates a Makefile tailored to your system
#   4. You run: make
#   5. It compiles fast_math.c into fast_math.so
#   6. Ruby can then: require './fast_math' (loads the .so)
#
# THE PIPELINE:
#   extconf.rb → Makefile → gcc compiles fast_math.c → fast_math.so → require
#
#   This is exactly like the C compilation pipeline from Module 2,
#   except the output is a shared library (.so) instead of an executable,
#   and it links against Ruby's C API headers.
#
# WHY C EXTENSIONS EXIST:
#   - Performance: C code runs 10-100x faster than Ruby for CPU-bound work
#   - FFI: Interface with C libraries (databases, XML parsers, crypto)
#   - Examples: nokogiri (XML), pg (PostgreSQL), mysql2, ffi, grpc
#   - About 30% of popular gems have C extensions
#
# RUN:
#   cd c_extension/
#   ruby extconf.rb    # generates Makefile
#   make               # compiles fast_math.so
#   ruby test_ext.rb   # tests the extension
# =============================================================================

require 'mkmf'

# mkmf = "Make Makefile" — Ruby's build configuration library
# It provides functions to:
#   - Check for headers: have_header('stdio.h')
#   - Check for libraries: have_library('pthread')
#   - Check for functions: have_func('malloc')
#   - Set compiler flags: $CFLAGS += '-O2'

# The extension name MUST match the .c file name (without extension)
# and the Init_ function name inside the C file.
#
# So for 'fast_math':
#   - Source file: fast_math.c
#   - Init function: Init_fast_math()
#   - Output: fast_math.so (Linux) or fast_math.bundle (macOS)

# Optional: check for math library (for advanced math functions)
# have_library('m')  # libm — math library

# Optional: enable optimization flags
# These are the same flags you'd pass to gcc manually
$CFLAGS += ' -O2'         # Optimization level 2 (good performance)
$CFLAGS += ' -Wall'       # Enable all warnings
$CFLAGS += ' -Wextra'     # Enable extra warnings
# $CFLAGS += ' -std=c99'  # Use C99 standard (uncomment if needed)

# This generates the Makefile
# The argument must match:
#   1. The .c filename (fast_math.c)
#   2. The Init_ function in the .c file (Init_fast_math)
create_makefile('fast_math')

puts "\nMakefile generated successfully!"
puts "Now run 'make' to compile the C extension."
puts "Then run 'ruby test_ext.rb' to test it."
