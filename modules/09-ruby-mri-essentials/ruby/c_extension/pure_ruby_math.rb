#!/usr/bin/env ruby
# =============================================================================
# Pure Ruby Math — Module 9: Ruby MRI Essentials
# =============================================================================
#
# PURPOSE:
#   Pure Ruby implementations of the same functions provided by the FastMath
#   C extension. Used for:
#     1. Benchmarking: compare YARV interpretation vs native C execution
#     2. Correctness: verify C extension produces the same results
#     3. Education: see the Ruby code that the C extension replaces
#
# WHAT TO OBSERVE:
#   - The algorithms are IDENTICAL to the C extension
#   - The only difference is execution method: YARV bytecode vs machine code
#   - Any speed difference is purely due to interpretation overhead
#   - This is a clean apples-to-apples comparison
#
# RUN: This file is loaded by test_ext.rb — don't run it directly.
# =============================================================================

module PureRubyMath
  # ===========================================================================
  # fibonacci(n) — Iterative Fibonacci in pure Ruby
  # ===========================================================================
  # Same algorithm as the C extension: O(n) time, O(1) space.
  # The speed difference comes from:
  #   - Each '+' is an opt_plus YARV instruction (type check + add)
  #   - Each loop iteration: getlocal, putobject, opt_lt, branchunless, ...
  #   - Each assignment: setlocal_WC_0 (store to local variable slot)
  # In C, the loop body is about 5 machine instructions.
  # In Ruby, it's about 15-20 YARV instructions, each requiring fetch+decode.
  # ===========================================================================
  def self.fibonacci(n)
    raise ArgumentError, "fibonacci requires non-negative integer" if n < 0
    return n if n <= 1

    a, b = 0, 1
    (n - 1).times do
      a, b = b, a + b
    end
    b
  end

  # ===========================================================================
  # sum(array) — Sum all elements of an array in pure Ruby
  # ===========================================================================
  # Ruby's inject/reduce method creates a block (closure) for each call.
  # We use a simple loop here to match the C extension's approach.
  # Even the loop version is slower than C because:
  #   - Array element access: send(:[], i) → method lookup → bounds check
  #   - Addition: opt_plus with type checking on every iteration
  #   - Local variable access: getlocal/setlocal on the YARV stack
  # ===========================================================================
  def self.sum(array)
    total = 0
    i = 0
    len = array.length
    while i < len
      total += array[i]
      i += 1
    end
    total
  end

  # ===========================================================================
  # dot_product(array1, array2) — Dot product of two arrays
  # ===========================================================================
  def self.dot_product(array1, array2)
    raise ArgumentError, "arrays must be same length" unless array1.length == array2.length

    result = 0
    i = 0
    len = array1.length
    while i < len
      result += array1[i] * array2[i]
      i += 1
    end
    result
  end

  # ===========================================================================
  # is_prime?(n) — Primality test in pure Ruby
  # ===========================================================================
  # Same trial division algorithm as the C version.
  # Speed difference comes from modulo operations and comparisons
  # being YARV instructions (opt_mod, opt_eq, opt_lt) instead of
  # single CPU instructions.
  # ===========================================================================
  def self.is_prime?(n)
    return false if n < 2
    return true  if n < 4
    return false if n % 2 == 0
    return false if n % 3 == 0

    i = 5
    while i * i <= n
      return false if n % i == 0
      return false if n % (i + 2) == 0
      i += 6
    end
    true
  end
end

# ===========================================================================
# Quick self-test when run directly
# ===========================================================================
if __FILE__ == $0
  puts "PureRubyMath self-test:"
  puts "  fibonacci(10) = #{PureRubyMath.fibonacci(10)}"
  puts "  fibonacci(20) = #{PureRubyMath.fibonacci(20)}"
  puts "  sum([1,2,3,4,5]) = #{PureRubyMath.sum([1, 2, 3, 4, 5])}"
  puts "  dot_product([1,2,3], [4,5,6]) = #{PureRubyMath.dot_product([1, 2, 3], [4, 5, 6])}"
  puts "  is_prime?(997) = #{PureRubyMath.is_prime?(997)}"
  puts "  is_prime?(1000) = #{PureRubyMath.is_prime?(1000)}"
  puts "All OK!"
end
