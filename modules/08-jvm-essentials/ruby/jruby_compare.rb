# ==============================================================================
# Module 8: JVM Essentials — JRuby vs MRI Comparison
# ==============================================================================
#
# CONCEPT: Ruby has multiple implementations that run on different VMs:
#
#   MRI (Matz's Ruby Interpreter) / CRuby:
#     - The default Ruby implementation, written in C
#     - Compiles Ruby to YARV bytecode (Yet Another Ruby VM)
#     - Has a Global Interpreter Lock (GIL) — only one thread runs at a time
#     - Uses mark-and-sweep garbage collection
#     - Module 9 covers this in detail
#
#   JRuby:
#     - Ruby implementation running on the JVM
#     - Compiles Ruby to JVM bytecode
#     - Gets JIT compilation from the JVM's HotSpot compiler
#     - No GIL — true multithreading (JVM handles thread safety)
#     - Uses JVM's garbage collector (G1, ZGC, etc.)
#     - Everything from Module 8 applies to JRuby
#
#   TruffleRuby:
#     - Ruby on GraalVM (a polyglot JVM-based VM)
#     - Often the fastest Ruby implementation
#     - Uses Truffle framework for automatic optimization
#
# WHY THIS MATTERS:
#   The SAME Ruby source code can execute in fundamentally different ways
#   depending on which Ruby implementation runs it. This demonstrates that
#   the "source to binary" pipeline is not fixed — it depends on the runtime.
#
#   MRI Ruby:
#     .rb -> YARV bytecode -> interpreted by YARV VM (C code)
#     → Module 9 applies
#
#   JRuby:
#     .rb -> JVM bytecode -> interpreted then JIT compiled by JVM
#     → Module 8 applies (class loading, JIT warmup, GC, etc.)
#
# TO RUN:
#   With MRI:    ruby jruby_compare.rb
#   With JRuby:  jruby jruby_compare.rb
#   Compare the output — same source code, different execution model.
#
# ==============================================================================

puts "=" * 60
puts " Module 8: Ruby Implementation Comparison"
puts "=" * 60
puts

# ==============================================================================
# Detect which Ruby implementation is running
# ==============================================================================

puts "--- Runtime Detection ---"
puts "  RUBY_ENGINE:   #{RUBY_ENGINE}"
puts "  RUBY_VERSION:  #{RUBY_VERSION}"
puts "  RUBY_PLATFORM: #{RUBY_PLATFORM}"
puts "  RUBY_ENGINE_VERSION: #{RUBY_ENGINE_VERSION}" if defined?(RUBY_ENGINE_VERSION)
puts

case RUBY_ENGINE
when "ruby"
  puts "  You are running MRI (CRuby)."
  puts "  This is the default C-based Ruby interpreter."
  puts "  → Module 9 (Ruby/MRI Essentials) applies to this runtime."
  puts
  puts "  Execution model:"
  puts "    .rb source → Parse → AST → Compile → YARV bytecode → Interpret"
  puts "    - YARV is a stack-based bytecode VM (written in C)"
  puts "    - No JIT compilation (unless you use --jit flag in Ruby 2.6+/YJIT in 3.1+)"
  puts "    - Global Interpreter Lock (GIL): only one thread runs Ruby at a time"
  puts "    - GC: mark-and-sweep with generational collection"
  puts
when "jruby"
  puts "  You are running JRuby."
  puts "  This runs Ruby on the Java Virtual Machine (JVM)."
  puts "  → Module 8 (JVM Essentials) applies to this runtime."
  puts
  puts "  Execution model:"
  puts "    .rb source → Parse → AST → JVM bytecode → Interpret → JIT compile"
  puts "    - JVM bytecode is the same format as javac output"
  puts "    - HotSpot JIT compiles hot methods to native code"
  puts "    - No GIL: true multithreading via JVM threads"
  puts "    - GC: JVM garbage collector (G1, ZGC, etc.)"
  puts
  puts "  JVM-specific info:"
  puts "    JVM version: #{java.lang.System.getProperty('java.version')}" rescue nil
  puts "    JVM name:    #{java.lang.System.getProperty('java.vm.name')}" rescue nil
  puts
when "truffleruby"
  puts "  You are running TruffleRuby (GraalVM)."
  puts "  This is often the fastest Ruby implementation."
  puts "  → Module 8 concepts apply (it's JVM-based under the hood)."
  puts
else
  puts "  Unknown Ruby engine: #{RUBY_ENGINE}"
  puts
end

# ==============================================================================
# Computation benchmark: Sum of squares of even numbers
# ==============================================================================
# Same algorithm as Module 7 (C/Rust) and Module 8 (Java).
# This lets you compare Ruby speed against all other languages.

puts "--- Benchmark: Sum of Squares of Even Numbers (1..1,000,000) ---"
puts

n = 1_000_000

# Warmup (important for JRuby's JIT, less so for MRI)
puts "  Warming up (3 iterations)..."
3.times do
  sum = 0
  (1..n).each do |i|
    sum += i * i if i.even?
  end
end

# Timed runs
puts "  Running 5 timed trials..."
puts

times = []
result = nil

5.times do |trial|
  start_time = Process.clock_gettime(Process::CLOCK_MONOTONIC, :microsecond)

  sum = 0
  (1..n).each do |i|
    sum += i * i if i.even?
  end
  result = sum

  end_time = Process.clock_gettime(Process::CLOCK_MONOTONIC, :microsecond)
  elapsed = end_time - start_time
  times << elapsed

  puts "    Trial #{trial}: #{elapsed.to_s.rjust(10)} us (result: #{result})"
end

puts
puts "  Best time: #{times.min} us"
puts "  Avg time:  #{(times.sum / times.length)} us"
puts "  Result:    #{result}"
puts

# ==============================================================================
# Functional-style benchmark (same computation)
# ==============================================================================

puts "--- Benchmark: Functional Style ---"
puts

times_func = []
result_func = nil

5.times do |trial|
  start_time = Process.clock_gettime(Process::CLOCK_MONOTONIC, :microsecond)

  result_func = (1..n).select(&:even?).map { |i| i * i }.sum

  end_time = Process.clock_gettime(Process::CLOCK_MONOTONIC, :microsecond)
  elapsed = end_time - start_time
  times_func << elapsed

  puts "    Trial #{trial}: #{elapsed.to_s.rjust(10)} us"
end

puts
puts "  Best time: #{times_func.min} us"
puts "  Results match: #{result == result_func}"
puts

# ==============================================================================
# Memory information
# ==============================================================================

puts "--- Memory Usage ---"
puts

if RUBY_ENGINE == "jruby"
  # JRuby has access to JVM memory management
  begin
    runtime = java.lang.Runtime.getRuntime
    puts "  JVM Heap used:  #{(runtime.totalMemory - runtime.freeMemory) / 1024} KB"
    puts "  JVM Heap total: #{runtime.totalMemory / 1024} KB"
    puts "  JVM Heap max:   #{runtime.maxMemory / 1024} KB"
  rescue => e
    puts "  (Could not access JVM memory info: #{e.message})"
  end
else
  # MRI: use RSS from /proc (Linux)
  begin
    if File.exist?("/proc/self/status")
      status = File.read("/proc/self/status")
      if status =~ /VmRSS:\s+(\d+)\s+kB/
        puts "  RSS (Resident Set Size): #{$1} KB"
      end
      if status =~ /VmSize:\s+(\d+)\s+kB/
        puts "  Virtual Memory: #{$1} KB"
      end
    else
      puts "  (Memory info not available on this platform)"
    end
  rescue => e
    puts "  (Could not read memory info: #{e.message})"
  end
end
puts

# ==============================================================================
# GC information
# ==============================================================================

puts "--- Garbage Collection ---"
puts

gc_stats = GC.stat
puts "  GC count:        #{gc_stats[:count] || gc_stats[:total_allocated_objects] || 'N/A'}"

if RUBY_ENGINE == "ruby"
  puts "  Heap pages:      #{gc_stats[:heap_allocated_pages] || 'N/A'}"
  puts "  Total objects:   #{gc_stats[:total_allocated_objects] || 'N/A'}"
  puts "  Total freed:     #{gc_stats[:total_freed_objects] || 'N/A'}"
  puts "  (MRI uses mark-and-sweep with generational collection)"
elsif RUBY_ENGINE == "jruby"
  puts "  (JRuby uses JVM's garbage collector — see Module 8 GCDemo.java)"
  puts "  (Use -verbose:gc or -Xlog:gc to see GC events)"
end
puts

# ==============================================================================
# Comparison with other languages
# ==============================================================================

puts "--- Cross-Language Comparison ---"
puts
puts "  Run the SAME algorithm in different languages to compare:"
puts
puts "  Ruby (this):     ruby jruby_compare.rb"
puts "  JRuby:           jruby jruby_compare.rb"
puts "  Java (Module 8): java NativeCompare"
puts "  Rust (Module 7): ./build/zero_cost"
puts "  C (Module 7):    ./build/equivalent"
puts
puts "  Expected speed ranking (fastest to slowest):"
puts "    1. C -O2 / Rust -O2     (compiled ahead-of-time, no overhead)"
puts "    2. Java (after warmup)   (JIT'd, but GC safepoints)"
puts "    3. JRuby (after warmup)  (JIT'd, but Ruby's dynamic nature limits optimization)"
puts "    4. MRI Ruby              (interpreted YARV bytecode)"
puts
puts "  Expected memory ranking (lowest to highest):"
puts "    1. C                     (no runtime overhead)"
puts "    2. Rust                  (no runtime overhead)"
puts "    3. MRI Ruby              (YARV VM is lightweight)"
puts "    4. JRuby / Java          (JVM overhead: 100+ MB)"
