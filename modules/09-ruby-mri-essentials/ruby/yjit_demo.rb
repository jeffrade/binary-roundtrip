#!/usr/bin/env ruby
# =============================================================================
# YJIT Demo — Module 9: Ruby MRI Essentials
# =============================================================================
#
# PURPOSE:
#   YJIT (Yet another JIT) is Ruby's built-in JIT compiler, added in Ruby 3.1.
#   It compiles YARV bytecode to native machine code at runtime.
#
#   Unlike the JVM's JIT (C1/C2/Graal), YJIT is:
#     - OPT-IN: you must pass --yjit flag to enable it
#     - NOT TIERED: it compiles methods once, no escalating optimization levels
#     - SIMPLER: designed for Ruby's dynamic nature, not aggressive speculation
#     - LAZY: compiles methods as they're called, not based on hotness counters
#
#   YJIT typically gives 15-30% speedup on Rails apps. Not as dramatic as
#   JVM's JIT (which can achieve 10-50x over interpreted), but significant
#   for a dynamic language.
#
# WHAT TO OBSERVE:
#   - With YJIT off: pure YARV interpretation (slower)
#   - With YJIT on:  native code execution (faster)
#   - The speedup varies: simple arithmetic benefits most; method dispatch less so
#
# RUN:
#   ruby yjit_demo.rb          # without YJIT (interpreted only)
#   ruby --yjit yjit_demo.rb   # with YJIT (JIT compilation enabled)
#
# =============================================================================

require 'benchmark'

puts "=" * 70
puts "YJIT DEMONSTRATION"
puts "Ruby #{RUBY_VERSION} (#{RUBY_ENGINE})"
puts "=" * 70

# =============================================================================
# Check YJIT availability
# =============================================================================
yjit_available = defined?(RubyVM::YJIT) && (RubyVM::YJIT.enabled? rescue false)

if yjit_available
  puts "\n  ** YJIT is ENABLED **"
  puts "  YARV bytecode will be JIT-compiled to native machine code."
else
  puts "\n  ** YJIT is NOT enabled **"
  puts "  Running in pure YARV interpreter mode."
  puts "\n  To enable YJIT, re-run with:  ruby --yjit #{$0}"

  # Check if YJIT is available but not enabled
  if defined?(RubyVM::YJIT)
    puts "  (YJIT is available in this Ruby build but not activated)"
  else
    puts "  (YJIT requires Ruby 3.1+ built with YJIT support)"
    puts "   Your Ruby: #{RUBY_VERSION}"
    if RUBY_VERSION < "3.1"
      puts "   You need Ruby 3.1 or later for YJIT."
    else
      puts "   Your Ruby may not have been compiled with YJIT support."
      puts "   Reinstall with: RUBY_CONFIGURE_OPTS='--enable-yjit' rbenv install #{RUBY_VERSION}"
    end
  end
end

# =============================================================================
# BENCHMARK 1: Fibonacci — Deep recursion + arithmetic
# =============================================================================
# Recursive fibonacci is a worst-case for method dispatch overhead.
# YJIT helps by compiling the recursive calls to native code.
# =============================================================================

def fibonacci(n)
  return n if n <= 1
  fibonacci(n - 1) + fibonacci(n - 2)
end

puts "\n#{'=' * 70}"
puts "  BENCHMARK 1: Recursive Fibonacci"
puts "#{'=' * 70}"
puts "  Tests: method dispatch, recursion, arithmetic"

fib_n = 35
puts "  Computing fibonacci(#{fib_n})...\n\n"

time = Benchmark.realtime { result = fibonacci(fib_n) }
puts "  fibonacci(#{fib_n}) = #{fibonacci(fib_n)}"
puts "  Time: #{'%.3f' % time}s"
puts "  Mode: #{yjit_available ? 'YJIT (native code)' : 'YARV (interpreted)'}"

# =============================================================================
# BENCHMARK 2: Tight Loop with Arithmetic
# =============================================================================
# This is where YJIT shines most: the inner loop compiles to efficient
# native instructions. opt_plus, opt_lt, etc. become real CPU instructions.
# =============================================================================

def tight_loop(iterations)
  sum = 0
  i = 0
  while i < iterations
    sum += i * i + i
    sum %= 1_000_000_007
    i += 1
  end
  sum
end

puts "\n#{'=' * 70}"
puts "  BENCHMARK 2: Tight Arithmetic Loop"
puts "#{'=' * 70}"
puts "  Tests: loop overhead, integer arithmetic, local variable access"

loop_n = 10_000_000
puts "  Running #{loop_n.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} iterations...\n\n"

time = Benchmark.realtime { tight_loop(loop_n) }
puts "  Time: #{'%.3f' % time}s"
puts "  Mode: #{yjit_available ? 'YJIT (native code)' : 'YARV (interpreted)'}"

# =============================================================================
# BENCHMARK 3: Method Dispatch Overhead
# =============================================================================
# Ruby's method dispatch is complex: it checks method cache, ancestors chain,
# method_missing, etc. YJIT can inline simple method calls.
# =============================================================================

class Counter
  attr_reader :value

  def initialize
    @value = 0
  end

  def increment
    @value += 1
  end

  def add(n)
    @value += n
  end

  def reset
    @value = 0
  end
end

def method_dispatch_test(iterations)
  counter = Counter.new
  iterations.times do
    counter.increment
    counter.add(2)
    counter.value
  end
  counter.reset
end

puts "\n#{'=' * 70}"
puts "  BENCHMARK 3: Method Dispatch"
puts "#{'=' * 70}"
puts "  Tests: method lookup, instance variable access, attr_reader"

dispatch_n = 2_000_000
puts "  #{dispatch_n.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} iterations of method calls...\n\n"

time = Benchmark.realtime { method_dispatch_test(dispatch_n) }
puts "  Time: #{'%.3f' % time}s"
puts "  Mode: #{yjit_available ? 'YJIT (native code)' : 'YARV (interpreted)'}"

# =============================================================================
# BENCHMARK 4: String Operations
# =============================================================================
# String operations involve heap allocation, which YJIT can't optimize as much.
# This shows that YJIT's benefit varies by workload.
# =============================================================================

def string_test(iterations)
  result = ""
  iterations.times do |i|
    s = "hello_#{i}"
    result = s.upcase.reverse.downcase
  end
  result
end

puts "\n#{'=' * 70}"
puts "  BENCHMARK 4: String Operations"
puts "#{'=' * 70}"
puts "  Tests: string interpolation, method chaining, heap allocation"

str_n = 500_000
puts "  #{str_n.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} iterations of string manipulation...\n\n"

time = Benchmark.realtime { string_test(str_n) }
puts "  Time: #{'%.3f' % time}s"
puts "  Mode: #{yjit_available ? 'YJIT (native code)' : 'YARV (interpreted)'}"

# =============================================================================
# BENCHMARK 5: Array/Hash Operations (typical Rails patterns)
# =============================================================================

def collection_test(iterations)
  iterations.times do
    arr = (1..100).to_a
    arr.select { |x| x.even? }.map { |x| x * 2 }.sum
  end
end

puts "\n#{'=' * 70}"
puts "  BENCHMARK 5: Collection Operations (Rails-like)"
puts "#{'=' * 70}"
puts "  Tests: array creation, select, map, sum (common Rails patterns)"

coll_n = 50_000
puts "  #{coll_n.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} iterations...\n\n"

time = Benchmark.realtime { collection_test(coll_n) }
puts "  Time: #{'%.3f' % time}s"
puts "  Mode: #{yjit_available ? 'YJIT (native code)' : 'YARV (interpreted)'}"

# =============================================================================
# YJIT Runtime Statistics (if available)
# =============================================================================
if yjit_available
  puts "\n#{'=' * 70}"
  puts "  YJIT RUNTIME STATISTICS"
  puts "#{'=' * 70}"

  begin
    stats = RubyVM::YJIT.runtime_stats
    if stats.is_a?(Hash) && !stats.empty?
      puts "\n  Compilation stats:"
      interesting_keys = %w[
        inline_code_size
        outlined_code_size
        compiled_iseq_count
        compiled_block_count
        invalidation_count
        yjit_alloc_size
      ]

      stats.each do |key, value|
        key_s = key.to_s
        if interesting_keys.include?(key_s) || interesting_keys.empty?
          formatted = if value.is_a?(Integer) && value > 1000
                        value.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')
                      else
                        value.to_s
                      end
          puts "    %-30s %s" % [key_s + ":", formatted]
        end
      end

      # Show ALL stats if user wants details
      puts "\n  All YJIT stats (#{stats.size} entries):"
      stats.each do |key, value|
        formatted = if value.is_a?(Integer) && value > 1000
                      value.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')
                    elsif value.is_a?(Float)
                      '%.4f' % value
                    else
                      value.to_s
                    end
        puts "    %-40s %s" % [key.to_s + ":", formatted]
      end
    else
      puts "\n  Stats not available (run with --yjit-stats for detailed stats)"
      puts "  Try: ruby --yjit --yjit-stats #{$0}"
    end
  rescue => e
    puts "\n  Could not retrieve stats: #{e.message}"
    puts "  Try running with: ruby --yjit --yjit-stats #{$0}"
  end
end

# =============================================================================
# SUMMARY
# =============================================================================
puts "\n#{'=' * 70}"
puts "  SUMMARY: YJIT vs JVM JIT"
puts "#{'=' * 70}"
puts <<~SUMMARY

  +-----------------------+---------------------------+---------------------------+
  | Feature               | Ruby YJIT                 | JVM JIT (C2/Graal)        |
  +-----------------------+---------------------------+---------------------------+
  | Activation            | Opt-in (--yjit flag)      | Always on (by default)    |
  | Tiered compilation    | No (single tier)          | Yes (C1 fast → C2 opt)    |
  | Typical speedup       | 15-30% over interp.       | 10-50x over interp.       |
  | Speculation           | Limited (type guards)     | Aggressive (deopt on fail)|
  | Inlining              | Basic                     | Deep + polymorphic        |
  | Warm-up time          | Fast (immediate compile)  | Slow (profile first)      |
  | Memory overhead       | Low (~10-50MB)            | High (100-500MB)          |
  | Available since       | Ruby 3.1 (2021)           | Java 1.3 (2000)           |
  +-----------------------+---------------------------+---------------------------+

  Why YJIT is less effective than JVM JIT:
  1. Ruby is MORE dynamic: almost everything is a method call
  2. Ruby objects can change shape at any time (open classes)
  3. method_missing, monkey patching make optimization risky
  4. YJIT is much newer — the JVM JIT has 20+ years of engineering

  But YJIT is getting better fast:
  - Ruby 3.2: major improvements, became production-ready
  - Ruby 3.3: further optimizations, lower memory usage
  - Each release closes the gap a bit more

  To try YJIT yourself:
    ruby #{$0}              # baseline (YARV interpreter)
    ruby --yjit #{$0}       # with YJIT enabled

SUMMARY
