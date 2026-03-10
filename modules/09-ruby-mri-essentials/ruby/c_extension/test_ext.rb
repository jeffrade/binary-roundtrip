#!/usr/bin/env ruby
# =============================================================================
# C Extension Test — Module 9: Ruby MRI Essentials
# =============================================================================
#
# PURPOSE:
#   Tests the FastMath C extension and benchmarks it against pure Ruby.
#   This demonstrates the dramatic performance difference between:
#     - YARV bytecode interpretation (pure Ruby)
#     - Native machine code execution (C extension)
#
# WHAT TO OBSERVE:
#   - C extension functions are called exactly like normal Ruby methods
#   - The API is seamless — you can't tell from the caller's side
#   - Performance difference is typically 10-50x for CPU-bound work
#   - This is why nokogiri, pg, and other gems use C extensions
#
# PREREQUISITES:
#   cd c_extension/
#   ruby extconf.rb && make    # compile the C extension first
#
# RUN: ruby test_ext.rb
# =============================================================================

require 'benchmark'

# Load the compiled C extension
# This does: dlopen("fast_math.so") → Init_fast_math()
begin
  require_relative './fast_math'
  puts "FastMath C extension loaded successfully!"
rescue LoadError => e
  puts "ERROR: Could not load FastMath C extension."
  puts "  #{e.message}"
  puts ""
  puts "  You need to compile it first:"
  puts "    cd #{File.dirname(__FILE__)}"
  puts "    ruby extconf.rb"
  puts "    make"
  puts ""
  puts "  Or from the module root:"
  puts "    make c-extension"
  exit 1
end

# Load pure Ruby equivalent for comparison
require_relative './pure_ruby_math'

puts "=" * 70
puts "C EXTENSION vs PURE RUBY BENCHMARK"
puts "=" * 70

# =============================================================================
# TEST 1: Correctness — Make sure C extension gives same results as Ruby
# =============================================================================
puts "\n#{'=' * 70}"
puts "  TEST 1: Correctness Verification"
puts "#{'=' * 70}"

test_cases = [0, 1, 2, 5, 10, 20, 30, 40, 50]
all_correct = true

puts "\n  Fibonacci correctness:"
test_cases.each do |n|
  c_result = FastMath.fibonacci(n)
  ruby_result = PureRubyMath.fibonacci(n)
  match = c_result == ruby_result
  all_correct &&= match
  status = match ? "OK" : "MISMATCH"
  puts "    fibonacci(%-3d) = %-15d (C) vs %-15d (Ruby) [%s]" % [n, c_result, ruby_result, status]
end

puts "\n  Sum correctness:"
test_arrays = [
  [],
  [1],
  [1, 2, 3, 4, 5],
  (1..100).to_a,
  (1..1000).to_a,
]

test_arrays.each do |arr|
  c_result = FastMath.sum(arr)
  ruby_result = PureRubyMath.sum(arr)
  match = c_result == ruby_result
  all_correct &&= match
  status = match ? "OK" : "MISMATCH"
  label = arr.length <= 5 ? arr.inspect : "[1..#{arr.last}]"
  puts "    sum(%-15s) = %-10d (C) vs %-10d (Ruby) [%s]" % [label, c_result, ruby_result, status]
end

puts "\n  Dot product correctness:"
dot_tests = [
  [[1, 2, 3], [4, 5, 6]],
  [[0, 0, 0], [1, 2, 3]],
  [[1, 1, 1, 1], [2, 2, 2, 2]],
]

dot_tests.each do |a, b|
  c_result = FastMath.dot_product(a, b)
  ruby_result = PureRubyMath.dot_product(a, b)
  match = c_result == ruby_result
  all_correct &&= match
  status = match ? "OK" : "MISMATCH"
  puts "    dot(%-12s, %-12s) = %-6d (C) vs %-6d (Ruby) [%s]" % [a.inspect, b.inspect, c_result, ruby_result, status]
end

puts "\n  Primality correctness:"
prime_tests = [2, 3, 4, 17, 100, 997, 1000003, 999999937]
prime_tests.each do |n|
  c_result = FastMath.is_prime?(n)
  ruby_result = PureRubyMath.is_prime?(n)
  match = c_result == ruby_result
  all_correct &&= match
  status = match ? "OK" : "MISMATCH"
  puts "    is_prime?(%-12d) = %-6s (C) vs %-6s (Ruby) [%s]" % [n, c_result, ruby_result, status]
end

puts "\n  #{all_correct ? 'ALL TESTS PASSED' : 'SOME TESTS FAILED'}"

# =============================================================================
# TEST 2: Performance — Fibonacci
# =============================================================================
puts "\n#{'=' * 70}"
puts "  TEST 2: Fibonacci Performance (iterative, n=70, 100000 iterations)"
puts "#{'=' * 70}"

fib_n = 70
fib_iters = 100_000

puts "\n  Computing fibonacci(#{fib_n}) #{fib_iters.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} times...\n\n"

c_time = Benchmark.realtime do
  fib_iters.times { FastMath.fibonacci(fib_n) }
end

ruby_time = Benchmark.realtime do
  fib_iters.times { PureRubyMath.fibonacci(fib_n) }
end

speedup = ruby_time / c_time
puts "  C extension: #{'%.3f' % c_time}s"
puts "  Pure Ruby:   #{'%.3f' % ruby_time}s"
puts "  Speedup:     #{'%.1f' % speedup}x faster with C"

# =============================================================================
# TEST 3: Performance — Array Sum
# =============================================================================
puts "\n#{'=' * 70}"
puts "  TEST 3: Array Sum Performance (10000 elements, 10000 iterations)"
puts "#{'=' * 70}"

sum_arr = (1..10_000).to_a
sum_iters = 10_000

puts "\n  Summing #{sum_arr.length.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} elements #{sum_iters.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} times...\n\n"

c_time = Benchmark.realtime do
  sum_iters.times { FastMath.sum(sum_arr) }
end

ruby_time = Benchmark.realtime do
  sum_iters.times { PureRubyMath.sum(sum_arr) }
end

speedup = ruby_time / c_time
puts "  C extension: #{'%.3f' % c_time}s"
puts "  Pure Ruby:   #{'%.3f' % ruby_time}s"
puts "  Speedup:     #{'%.1f' % speedup}x faster with C"

# =============================================================================
# TEST 4: Performance — Primality Testing
# =============================================================================
puts "\n#{'=' * 70}"
puts "  TEST 4: Primality Test Performance (large primes)"
puts "#{'=' * 70}"

prime_iters = 50_000
test_prime = 999999937  # A large prime

puts "\n  Testing is_prime?(#{test_prime}) #{prime_iters.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} times...\n\n"

c_time = Benchmark.realtime do
  prime_iters.times { FastMath.is_prime?(test_prime) }
end

ruby_time = Benchmark.realtime do
  prime_iters.times { PureRubyMath.is_prime?(test_prime) }
end

speedup = ruby_time / c_time
puts "  C extension: #{'%.3f' % c_time}s"
puts "  Pure Ruby:   #{'%.3f' % ruby_time}s"
puts "  Speedup:     #{'%.1f' % speedup}x faster with C"

# =============================================================================
# TEST 5: Performance — Dot Product
# =============================================================================
puts "\n#{'=' * 70}"
puts "  TEST 5: Dot Product Performance (10000 elements, 10000 iterations)"
puts "#{'=' * 70}"

dp_arr1 = (1..10_000).to_a
dp_arr2 = (1..10_000).to_a.reverse
dp_iters = 10_000

puts "\n  Computing dot product of #{dp_arr1.length.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} element vectors #{dp_iters.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} times...\n\n"

c_time = Benchmark.realtime do
  dp_iters.times { FastMath.dot_product(dp_arr1, dp_arr2) }
end

ruby_time = Benchmark.realtime do
  dp_iters.times { PureRubyMath.dot_product(dp_arr1, dp_arr2) }
end

speedup = ruby_time / c_time
puts "  C extension: #{'%.3f' % c_time}s"
puts "  Pure Ruby:   #{'%.3f' % ruby_time}s"
puts "  Speedup:     #{'%.1f' % speedup}x faster with C"

# =============================================================================
# SUMMARY
# =============================================================================
puts "\n#{'=' * 70}"
puts "  SUMMARY"
puts "#{'=' * 70}"
puts <<~SUMMARY

  C extensions achieve their speed by bypassing the YARV interpreter entirely.
  When you call FastMath.fibonacci(n):

  Pure Ruby path:
    Ruby call → method lookup → YARV bytecode loop →
      getlocal → putobject → opt_lt → branchunless → getlocal →
      getlocal → opt_plus → setlocal → ... (per iteration)

  C extension path:
    Ruby call → method lookup → DIRECT C function call →
      mov, add, cmp, jl (CPU instructions, per iteration)

  The C path skips:
    - YARV instruction fetch & decode
    - Stack manipulation for each operation
    - Type checking for optimized operations (opt_plus checks types)
    - GC write barriers for local variables

  This is also why C extensions can bypass the GVL:
    - The GVL protects Ruby's internal data structures
    - If your C code doesn't touch Ruby objects, you can release the GVL
    - This enables TRUE parallelism for CPU-bound C code
    - Example: pg gem releases GVL during database queries

SUMMARY
