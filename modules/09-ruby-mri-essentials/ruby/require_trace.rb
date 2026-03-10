#!/usr/bin/env ruby
# =============================================================================
# Require Trace — Module 9: Ruby MRI Essentials
# =============================================================================
#
# PURPOSE:
#   Every time you call `require 'something'` in Ruby, a complex process runs:
#     1. Search $LOAD_PATH directories for the file
#     2. Read the file from disk
#     3. Parse it into an AST
#     4. Compile the AST to YARV bytecode
#     5. Execute all top-level code in the file
#     6. Register it in $LOADED_FEATURES so it's not loaded again
#
#   A large Ruby app loads THOUSANDS of files on boot. Each require goes
#   through this entire pipeline. This is why startup is slow (2-20 seconds)
#   and why tools like Bootsnap exist (they cache the compiled bytecode).
#
# WHAT TO OBSERVE:
#   - Each require has a measurable time cost
#   - Some libraries are fast (just define classes/modules)
#   - Some are slow (they do work at require time — HTTP connections, FFI loading, etc.)
#   - The cumulative effect of hundreds of requires is significant
#
# RUN: ruby require_trace.rb
# =============================================================================

puts "=" * 70
puts "REQUIRE TRACE — Understanding Ruby's File Loading"
puts "Ruby #{RUBY_VERSION}"
puts "=" * 70

# =============================================================================
# Monkey-patch Kernel#require to trace all load times
# =============================================================================
# This is a real technique used by profiling tools like bootsnap and
# ruby-prof. We wrap the original require to measure time around each call.
#
# NOTE: In large Ruby apps, Bootsnap replaces this pipeline with:
#   1. Check if cached bytecode exists
#   2. If yes: load cached bytecode directly (skip parse + compile)
#   3. If no: normal require + cache the bytecode for next time
# =============================================================================

module RequireTracer
  # Store trace data
  @traces = []
  @depth = 0
  @enabled = false

  class << self
    attr_accessor :traces, :depth, :enabled
  end

  def self.reset!
    @traces = []
    @depth = 0
  end
end

# Save original require
module Kernel
  alias_method :original_require, :require

  def require(path)
    unless RequireTracer.enabled
      return original_require(path)
    end

    start_time = Process.clock_gettime(Process::CLOCK_MONOTONIC, :microsecond)
    depth = RequireTracer.depth
    RequireTracer.depth += 1

    begin
      result = original_require(path)
    ensure
      RequireTracer.depth -= 1
      end_time = Process.clock_gettime(Process::CLOCK_MONOTONIC, :microsecond)
      elapsed_us = end_time - start_time

      RequireTracer.traces << {
        path: path,
        elapsed_us: elapsed_us,
        depth: depth,
        loaded: result,  # true if actually loaded, false if already loaded
      }
    end

    result
  end
end

# =============================================================================
# Show $LOAD_PATH — where Ruby looks for files
# =============================================================================
puts "\n#{'=' * 70}"
puts "  SECTION 1: $LOAD_PATH (where Ruby searches for files)"
puts "#{'=' * 70}"
puts "\n  Ruby searches these directories IN ORDER when you call require:\n\n"

$LOAD_PATH.each_with_index do |path, i|
  exists = File.directory?(path) ? "exists" : "MISSING"
  puts "  %3d. %-60s [%s]" % [i + 1, path, exists]
end

puts "\n  Total: #{$LOAD_PATH.length} directories"
puts "  (A large Ruby app adds many more: lib/, app/, gems, etc.)"

# =============================================================================
# Show already-loaded files
# =============================================================================
puts "\n#{'=' * 70}"
puts "  SECTION 2: $LOADED_FEATURES (already loaded files)"
puts "#{'=' * 70}"

loaded_before = $LOADED_FEATURES.length
puts "\n  Files already loaded (just to run this script): #{loaded_before}"
puts "\n  First 20 loaded features:"
$LOADED_FEATURES.first(20).each_with_index do |feat, i|
  puts "  %3d. %s" % [i + 1, feat]
end
puts "  ..."

# Count by type
so_count = $LOADED_FEATURES.count { |f| f.end_with?('.so') || f.end_with?('.bundle') }
rb_count = $LOADED_FEATURES.count { |f| f.end_with?('.rb') }
puts "\n  Breakdown: #{rb_count} .rb files, #{so_count} .so/.bundle files (C extensions)"

# =============================================================================
# Trace some standard library requires
# =============================================================================
puts "\n#{'=' * 70}"
puts "  SECTION 3: Tracing require calls (standard library)"
puts "#{'=' * 70}"
puts "\n  Loading several standard library modules and measuring time...\n"

RequireTracer.enabled = true
RequireTracer.reset!

# Require several stdlib libraries of varying complexity
libraries = %w[
  json
  set
  ostruct
  uri
  digest
  base64
  csv
  optparse
  fileutils
  tempfile
  logger
  socket
  open3
  pathname
]

total_start = Process.clock_gettime(Process::CLOCK_MONOTONIC, :microsecond)

libraries.each do |lib|
  begin
    require lib
  rescue LoadError => e
    puts "  WARNING: Could not load '#{lib}': #{e.message}"
  end
end

total_end = Process.clock_gettime(Process::CLOCK_MONOTONIC, :microsecond)
total_time = total_end - total_start

RequireTracer.enabled = false

# =============================================================================
# Display results
# =============================================================================
puts "\n  Trace results (sorted by load time, descending):\n\n"
puts "  %-8s %-8s %-6s %s" % ["Time", "Status", "Depth", "Path"]
puts "  #{'-' * 70}"

# Show top-level requires (libraries we explicitly loaded)
top_level = RequireTracer.traces.select { |t| t[:depth] == 0 }

top_level.sort_by { |t| -t[:elapsed_us] }.each do |trace|
  time_str = if trace[:elapsed_us] > 1000
               "#{'%.1f' % (trace[:elapsed_us] / 1000.0)}ms"
             else
               "#{trace[:elapsed_us]}us"
             end
  status = trace[:loaded] ? "loaded" : "cached"
  indent = "  " * trace[:depth]
  puts "  %-8s %-8s %-6d %s%s" % [time_str, status, trace[:depth], indent, trace[:path]]
end

# Show sub-requires (dependencies pulled in by the top-level requires)
sub_requires = RequireTracer.traces.select { |t| t[:depth] > 0 }
if sub_requires.any?
  puts "\n  Sub-dependencies loaded (top 20 by time):\n\n"
  sub_requires.sort_by { |t| -t[:elapsed_us] }.first(20).each do |trace|
    time_str = if trace[:elapsed_us] > 1000
                 "#{'%.1f' % (trace[:elapsed_us] / 1000.0)}ms"
               else
                 "#{trace[:elapsed_us]}us"
               end
    status = trace[:loaded] ? "loaded" : "cached"
    indent = "  " * [trace[:depth], 4].min
    puts "  %-8s %-8s depth=%-2d %s%s" % [time_str, status, trace[:depth], indent, trace[:path]]
  end
end

loaded_after = $LOADED_FEATURES.length
new_files = loaded_after - loaded_before

puts "\n  #{'=' * 50}"
puts "  TOTALS:"
puts "  #{'=' * 50}"
puts "  Total require calls traced:  #{RequireTracer.traces.length}"
puts "  Files actually loaded:       #{RequireTracer.traces.count { |t| t[:loaded] }}"
puts "  Already-cached (skipped):    #{RequireTracer.traces.count { |t| !t[:loaded] }}"
puts "  New $LOADED_FEATURES:        #{new_files}"
puts "  Total time:                  #{'%.1f' % (total_time / 1000.0)}ms"

# =============================================================================
# Show the require tree (dependency graph)
# =============================================================================
puts "\n#{'=' * 70}"
puts "  SECTION 4: Require Dependency Tree"
puts "#{'=' * 70}"
puts "\n  This shows which libraries pull in which sub-dependencies:\n"

current_top_level = nil
RequireTracer.traces.each do |trace|
  if trace[:depth] == 0
    current_top_level = trace[:path]
    status = trace[:loaded] ? "" : " (already loaded)"
    puts "\n  #{trace[:path]}#{status}"
  else
    indent = "  " + "  " * trace[:depth]
    marker = trace[:loaded] ? "+-" : "+- (cached)"
    puts "  #{indent}#{marker} #{trace[:path]}"
  end
end

# =============================================================================
# Simulate what a large Ruby app startup looks like
# =============================================================================
puts "\n\n#{'=' * 70}"
puts "  SECTION 5: What a Large Ruby App Startup Looks Like"
puts "#{'=' * 70}"
puts <<~EXPLAIN

  We loaded #{libraries.length} stdlib libraries, which pulled in #{new_files} total files.

  A typical large Ruby app loads:
    - ~50 gem dependencies (each with their own requires)
    - ~200-500 gem files
    - ~100-300 app files (lib/, app/, etc.)
    - ~50-100 config/initializer files
    - Total: 500-1,000+ files on boot

  At ~1-5ms per file, that's 1-5 SECONDS just for require overhead.
  This is why:
    1. Bootsnap exists — caches parsed/compiled bytecode
    2. Process warming existed — kept a preloaded process in the background
    3. Zeitwerk is used — autoloads files lazily (only when class is first used)
    4. Startup time in large Ruby apps is a constant concern

  The pipeline PER FILE:
    require 'foo'
       |
       v
    Search $LOAD_PATH for foo.rb          ~0.1ms (file stat calls)
       |
       v
    Read file from disk                   ~0.1ms (actual file read)
       |
       v
    Parse → AST                           ~0.5ms (Ruby's parser)
       |
       v
    Compile → YARV bytecode               ~0.3ms (compiler)
       |
       v
    Execute top-level code                ~0.1-10ms (depends on file)
       |
       v
    Register in $LOADED_FEATURES          ~0.01ms

  Bootsnap optimization:
    require 'foo'
       |
       v
    Check cache for foo.rb                ~0.01ms
       |
       v
    Load cached YARV bytecode             ~0.1ms (skip parse + compile!)
       |
       v
    Execute top-level code                ~0.1-10ms

  This saves ~40-60% of require time by skipping parse and compile steps.

EXPLAIN

# =============================================================================
# SECTION 6: Autoload — Lazy Loading Alternative
# =============================================================================
puts "#{'=' * 70}"
puts "  SECTION 6: Autoload — The Modern Alternative"
puts "#{'=' * 70}"

puts <<~EXPLAIN

  Large Ruby projects use autoloading (Zeitwerk is a popular library for this):

    # Instead of:
    require 'app/models/user'       # loaded immediately at boot
    require 'app/models/post'       # loaded immediately at boot
    require 'app/models/comment'    # loaded immediately at boot

    # With autoloading:
    autoload :User, 'app/models/user'       # just register the name
    autoload :Post, 'app/models/post'       # just register the name
    autoload :Comment, 'app/models/comment' # just register the name

    # Only loaded when FIRST USED:
    User.find(1)  # NOW user.rb is loaded (on first reference to User)

  This means: unused models/controllers are NEVER loaded.
  Boot time drops because only essential files are loaded eagerly.

  Ruby's autoload is built into the VM — when a constant is referenced
  and has an autoload registered, Ruby automatically requires the file.

EXPLAIN

puts "=" * 70
puts "DONE — Require tracing complete"
puts "=" * 70
