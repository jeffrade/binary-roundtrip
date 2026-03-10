#!/usr/bin/env ruby
# =============================================================================
# GIL/GVL Demo — Module 9: Ruby MRI Essentials
# =============================================================================
#
# PURPOSE:
#   Ruby MRI has a Global VM Lock (GVL, historically called GIL — Global
#   Interpreter Lock). This means:
#
#     ONLY ONE THREAD CAN EXECUTE RUBY BYTECODE AT A TIME.
#
#   This is the single most important thing to understand about Ruby concurrency.
#   It's why Ruby web servers use multiple PROCESSES (via Puma workers), not just threads.
#
# WHAT TO OBSERVE:
#   - CPU-bound work: 2 threads take the SAME time as 1 thread (GVL blocks parallelism)
#   - IO-bound work:  2 threads ARE faster (GVL is released during IO/sleep)
#   - Process.fork:   True parallelism (each process has its own GVL)
#
# WHY DOES THE GVL EXIST?
#   - MRI's C internals are not thread-safe
#   - Many C extensions assume single-threaded access to Ruby objects
#   - Removing the GVL would break every C extension (nokogiri, pg, etc.)
#   - Python has the same problem (the "GIL")
#   - JRuby and TruffleRuby do NOT have a GVL — they use JVM threads
#
# RUN: ruby gil_demo.rb
# =============================================================================

require 'benchmark'

puts "=" * 70
puts "GIL/GVL DEMONSTRATION"
puts "Ruby #{RUBY_VERSION} (#{RUBY_ENGINE})"
puts "=" * 70

# =============================================================================
# CPU-BOUND WORK: Demonstrates that the GVL blocks true parallelism
# =============================================================================
# This function does pure computation — no IO, no sleeping.
# The GVL means only one thread runs at a time, so adding threads
# does NOT speed this up. In fact it may be slightly SLOWER due to
# thread scheduling overhead.
# =============================================================================

# A deliberately CPU-intensive computation
def cpu_work(iterations)
  sum = 0
  iterations.times do |i|
    sum += i * i
    sum %= 1_000_000_007  # keep the number from growing too large
  end
  sum
end

WORK_ITERATIONS = 5_000_000

puts "\n#{'=' * 70}"
puts "  PART 1: CPU-BOUND WORK (GVL prevents parallelism)"
puts "#{'=' * 70}"
puts "\n  Each unit of work: #{WORK_ITERATIONS.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} iterations of arithmetic"
puts "  If threads were truly parallel, 2 threads should be ~2x faster."
puts "  But the GVL means they run sequentially...\n\n"

# --- Single threaded: do 2 units of work sequentially ---
time_1_thread = Benchmark.realtime do
  cpu_work(WORK_ITERATIONS)
  cpu_work(WORK_ITERATIONS)
end
puts "  1 thread,  2 units of work:  #{'%.3f' % time_1_thread}s"

# --- 2 threads: do 1 unit of work each (should be same speed due to GVL) ---
time_2_threads = Benchmark.realtime do
  threads = 2.times.map do
    Thread.new { cpu_work(WORK_ITERATIONS) }
  end
  threads.each(&:join)
end
puts "  2 threads, 1 unit each:      #{'%.3f' % time_2_threads}s"

# --- 4 threads: do 1/2 unit of work each ---
time_4_threads = Benchmark.realtime do
  threads = 4.times.map do
    Thread.new { cpu_work(WORK_ITERATIONS / 2) }
  end
  threads.each(&:join)
end
puts "  4 threads, 1/2 unit each:    #{'%.3f' % time_4_threads}s"

speedup_2 = time_1_thread / time_2_threads
speedup_4 = time_1_thread / time_4_threads
puts "\n  Speedup with 2 threads: #{'%.2f' % speedup_2}x (ideal would be 2.0x)"
puts "  Speedup with 4 threads: #{'%.2f' % speedup_4}x (ideal would be 2.0x)"
puts "\n  RESULT: No speedup! The GVL serializes all CPU-bound Ruby code."
puts "  All threads must take turns holding the lock to execute bytecode."

# =============================================================================
# IO-BOUND WORK: Demonstrates that the GVL IS released during IO
# =============================================================================
# When a thread does IO (file read, network call, sleep), it releases the GVL.
# This allows OTHER threads to run their Ruby code while one thread waits.
# This is why threaded servers (Puma) work well for IO-heavy Ruby apps.
# =============================================================================

puts "\n#{'=' * 70}"
puts "  PART 2: IO-BOUND WORK (GVL is released during IO)"
puts "#{'=' * 70}"
puts "\n  Using sleep() to simulate IO (network calls, file reads, etc.)"
puts "  The GVL is released during sleep, so threads CAN run in parallel.\n\n"

SLEEP_TIME = 0.5  # seconds — simulate a slow API call

# --- Sequential: 4 sleeps in a row ---
time_sequential = Benchmark.realtime do
  4.times { sleep(SLEEP_TIME) }
end
puts "  Sequential (4 sleeps):      #{'%.3f' % time_sequential}s"

# --- 4 threads: 1 sleep each (should be ~4x faster) ---
time_threaded = Benchmark.realtime do
  threads = 4.times.map do
    Thread.new { sleep(SLEEP_TIME) }
  end
  threads.each(&:join)
end
puts "  4 threads  (1 sleep each):  #{'%.3f' % time_threaded}s"

io_speedup = time_sequential / time_threaded
puts "\n  Speedup with threads: #{'%.2f' % io_speedup}x (ideal is 4.0x)"
puts "  RESULT: Threads DO speed up IO-bound work!"
puts "  This is why Puma uses threads: most web requests are IO-bound"
puts "  (database queries, API calls, file reads)."

# =============================================================================
# PROCESS FORK: True parallelism (bypasses GVL entirely)
# =============================================================================
# Each forked process gets its own copy of everything, including its own GVL.
# This is how Puma's "workers" achieve true parallelism.
# It's also why Unicorn and Sidekiq use multiple processes.
# =============================================================================

puts "\n#{'=' * 70}"
puts "  PART 3: Process.fork — True Parallelism (bypasses GVL)"
puts "#{'=' * 70}"

if Process.respond_to?(:fork)
  puts "\n  Each forked process has its OWN GVL — true parallel execution.\n\n"

  # --- Sequential: 2 units of CPU work ---
  time_seq = Benchmark.realtime do
    cpu_work(WORK_ITERATIONS)
    cpu_work(WORK_ITERATIONS)
  end
  puts "  Sequential (2 units):         #{'%.3f' % time_seq}s"

  # --- 2 forked processes: 1 unit each ---
  time_forked = Benchmark.realtime do
    pids = 2.times.map do
      Process.fork { cpu_work(WORK_ITERATIONS) }
    end
    pids.each { |pid| Process.wait(pid) }
  end
  puts "  2 processes (1 unit each):    #{'%.3f' % time_forked}s"

  fork_speedup = time_seq / time_forked
  puts "\n  Speedup with fork: #{'%.2f' % fork_speedup}x (ideal is 2.0x)"
  puts "  RESULT: True speedup! Each process has its own GVL."
  puts "\n  This is why Puma has both workers (processes) AND threads:"
  puts "    - Workers (processes) = true parallelism for CPU work"
  puts "    - Threads = concurrency for IO-bound work within each worker"
else
  puts "\n  Process.fork not available on this platform."
  puts "  (fork works on Linux/macOS, not Windows)"
end

# =============================================================================
# BONUS: Show thread states during execution
# =============================================================================
puts "\n#{'=' * 70}"
puts "  PART 4: Visualizing GVL Contention"
puts "#{'=' * 70}"
puts "\n  Watch how threads take turns with CPU-bound work:\n\n"

# Shorter work for visualization
VIS_WORK = 500_000
$thread_log = []
$log_mutex = Mutex.new

def logged_cpu_work(thread_id, iterations)
  chunks = 10
  chunk_size = iterations / chunks
  chunks.times do |c|
    cpu_work(chunk_size)
    $log_mutex.synchronize do
      $thread_log << [thread_id, c]
    end
  end
end

threads = 3.times.map do |id|
  Thread.new { logged_cpu_work(id, VIS_WORK) }
end
threads.each(&:join)

# Show execution timeline
puts "  Execution order (each dot = 1 chunk of work):"
puts "  Thread 0: #{$thread_log.select { |id, _| id == 0 }.map { |_, c| c }.join(' ')}"
puts "  Thread 1: #{$thread_log.select { |id, _| id == 1 }.map { |_, c| c }.join(' ')}"
puts "  Thread 2: #{$thread_log.select { |id, _| id == 2 }.map { |_, c| c }.join(' ')}"

puts "\n  Global order: #{$thread_log.map { |id, c| "T#{id}" }.join(' → ')}"
puts "\n  Notice: threads interleave (context switching) but NEVER truly overlap."
puts "  The GVL ensures each chunk runs to completion before switching."

# =============================================================================
# SUMMARY
# =============================================================================
puts "\n#{'=' * 70}"
puts "  SUMMARY: Ruby's Concurrency Model"
puts "#{'=' * 70}"
puts <<~SUMMARY

  +-----------+--------------------+-----------------------------+
  | Approach  | CPU-Bound Speedup  | IO-Bound Speedup           |
  +-----------+--------------------+-----------------------------+
  | Threads   | NO  (GVL blocks)   | YES (GVL released for IO)  |
  | Processes | YES (own GVL each) | YES (fully independent)    |
  | Ractors   | YES (Ruby 3.0+)    | YES (experimental)         |
  +-----------+--------------------+-----------------------------+

  Why Puma uses BOTH workers and threads:
    - Workers (processes): N workers = N CPU cores utilized
    - Threads per worker: M threads = M concurrent IO operations
    - Total concurrency: N * M simultaneous requests

  Compare to other runtimes:
    - JRuby/TruffleRuby: No GVL — threads give real CPU parallelism
    - Python (CPython): Same GIL problem — use multiprocessing
    - Java: No GIL — threads are truly parallel from the start
    - Go: Goroutines multiplexed across OS threads — no GIL
    - Rust: No GIL — ownership model ensures thread safety at compile time

SUMMARY
