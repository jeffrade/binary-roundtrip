#!/usr/bin/env ruby
# =============================================================================
# MRI Internals Explorer — Module 9: Ruby MRI Essentials
# =============================================================================
#
# PURPOSE:
#   MRI (Matz's Ruby Interpreter) is itself a C program. When you type "ruby",
#   you're running a compiled C binary that:
#     1. Reads your .rb file
#     2. Tokenizes and parses it
#     3. Compiles it to YARV bytecode
#     4. Executes the bytecode in a virtual machine
#
#   This script explores the internals of that C program from Ruby's perspective.
#   Everything here is introspection — Ruby looking at its own runtime.
#
# WHAT TO OBSERVE:
#   - Ruby itself is a native binary (compiled from ~400,000 lines of C)
#   - YARV has a fixed set of opcodes (similar to JVM's ~200 bytecodes)
#   - ObjectSpace tracks every living Ruby object in the heap
#   - GC statistics reveal how memory is managed
#   - The runtime is surprisingly introspectable
#
# RUN: ruby mri_internals.rb
# =============================================================================

puts "=" * 70
puts "MRI INTERNALS EXPLORER"
puts "Ruby #{RUBY_VERSION}p#{RUBY_PATCHLEVEL} (#{RUBY_RELEASE_DATE}) [#{RUBY_PLATFORM}]"
puts "Engine: #{RUBY_ENGINE} #{RUBY_ENGINE_VERSION}"
puts "=" * 70

# =============================================================================
# SECTION 1: Ruby is a C Binary
# =============================================================================
# The "ruby" command is a compiled C program — an ELF binary on Linux.
# This is no different from gcc, python3, or any other interpreter.
# The binary contains the YARV VM, parser, GC, and core library — all in C.
# =============================================================================

puts "\n#{'=' * 70}"
puts "  SECTION 1: Ruby is a C Binary"
puts "#{'=' * 70}"

ruby_path = RbConfig.ruby
puts "\n  Ruby binary path: #{ruby_path}"

# Show file type
puts "\n  'file' command output:"
file_output = `file #{ruby_path} 2>/dev/null`.strip
puts "    #{file_output}"

# Show binary size
if File.exist?(ruby_path)
  size_bytes = File.size(ruby_path)
  size_mb = size_bytes / (1024.0 * 1024.0)
  puts "\n  Binary size: #{size_bytes.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')} bytes (#{'%.1f' % size_mb} MB)"
end

# Show linked libraries (what C libraries MRI depends on)
puts "\n  Shared library dependencies (ldd output):"
ldd_output = `ldd #{ruby_path} 2>/dev/null`
if $?.success? && !ldd_output.empty?
  ldd_output.each_line.first(10) do |line|
    puts "    #{line.strip}"
  end
  total_libs = ldd_output.lines.count
  puts "    ... (#{total_libs} total libraries)" if total_libs > 10
else
  puts "    (ldd not available or not a dynamically linked binary)"
end

puts <<~EXPLAIN

  KEY INSIGHT: Ruby is a C program that reads your .rb files and interprets them.
  When you run 'ruby script.rb', the sequence is:
    1. OS loads the ruby ELF binary (just like any C program — see Module 5)
    2. main() in ruby's C code runs
    3. It parses your script into an AST (see ast_explorer.rb)
    4. Compiles the AST to YARV bytecode (see yarv_bytecode.rb)
    5. The YARV VM loop (a big switch/case in C) executes each instruction

  This is fundamentally the same as: python3 script.py, node script.js, java Main
  All are native programs that interpret/compile another language.
EXPLAIN

# =============================================================================
# SECTION 2: YARV Instruction Set (All Opcodes)
# =============================================================================
# YARV has a fixed set of instructions, just like the JVM has ~200 bytecodes.
# Each instruction is handled by a case in a giant C switch statement.
# This list shows every operation the YARV VM can execute.
# =============================================================================

puts "#{'=' * 70}"
puts "  SECTION 2: YARV Instruction Set (All Opcodes)"
puts "#{'=' * 70}"

opcodes = RubyVM::INSTRUCTION_NAMES
puts "\n  Total YARV opcodes: #{opcodes.length}"
puts "  (Compare: JVM has ~200 bytecodes, x86-64 has ~1000+ instructions)\n\n"

# Categorize opcodes
categories = {
  "Stack manipulation" => opcodes.select { |op| op =~ /^(put|pop|dup|swap|top|setn|adjuststack|nop|defined)/ },
  "Local variables"    => opcodes.select { |op| op =~ /^(get|set)local/ },
  "Instance/Class vars"=> opcodes.select { |op| op =~ /^(get|set)(instance|class|const|global|special|block)/ },
  "Method dispatch"    => opcodes.select { |op| op =~ /^(send|opt_send|invoke)/ },
  "Optimized ops"      => opcodes.select { |op| op =~ /^opt_(?!send)/ },
  "Control flow"       => opcodes.select { |op| op =~ /^(branch|jump|leave|throw|once)/ },
  "Class/Module"       => opcodes.select { |op| op =~ /^define/ },
  "Array/Hash/Range"   => opcodes.select { |op| op =~ /^(newarray|newhash|newrange|duparray|duphash|expand|concat|splat|splatarray)/ },
  "String"             => opcodes.select { |op| op =~ /^(tostring|freezestring|objtostring)/ },
  "Block/Proc"         => opcodes.select { |op| op =~ /^(getblockparam|setblockparam)/ },
}

categories.each do |category, ops|
  next if ops.empty?
  puts "  #{category}:"
  ops.each_slice(5) do |group|
    puts "    #{group.join(', ')}"
  end
  puts
end

# Show uncategorized
categorized = categories.values.flatten
uncategorized = opcodes - categorized
unless uncategorized.empty?
  puts "  Other:"
  uncategorized.each_slice(5) do |group|
    puts "    #{group.join(', ')}"
  end
  puts
end

# =============================================================================
# SECTION 3: ObjectSpace — Every Living Ruby Object
# =============================================================================
# ObjectSpace is Ruby's heap inspector. It can iterate over EVERY object
# in memory. This is like JVM's heap dump, but accessible at runtime.
# Rails apps can have millions of objects in memory.
# =============================================================================

puts "#{'=' * 70}"
puts "  SECTION 3: ObjectSpace — Live Object Census"
puts "#{'=' * 70}"

# Count objects by type
counts = ObjectSpace.count_objects
total = counts[:TOTAL] || counts.values.sum

puts "\n  Live objects in the Ruby heap:\n\n"

# Sort by count, descending
sorted = counts.sort_by { |_, v| -v }
sorted.each do |type, count|
  bar_len = (count.to_f / total * 50).ceil
  bar = "#" * [bar_len, 1].max
  pct = (count.to_f / total * 100)
  puts "  %-12s %10s  (%5.1f%%)  %s" % [type, count.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,'), pct, bar]
end

puts "\n  Total objects: #{total.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')}"

# Count specific class instances
puts "\n  Detailed class breakdown (top types):"
class_counts = Hash.new(0)
ObjectSpace.each_object do |obj|
  class_counts[obj.class.name || obj.class.to_s] += 1
end

class_counts.sort_by { |_, v| -v }.first(15).each do |klass, count|
  puts "    %-30s %s" % [klass, count.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')]
end

puts <<~EXPLAIN

  KEY INSIGHT: Ruby tracks every single object. This enables:
    - Garbage collection (it knows what's alive)
    - ObjectSpace.each_object for debugging
    - Memory profiling (find leaks by counting object types)

  Compare to C: C has no object tracking — free() is manual.
  Compare to JVM: JVM tracks objects but needs special tools (jmap, VisualVM)
  Ruby makes it trivially easy to inspect the heap.
EXPLAIN

# =============================================================================
# SECTION 4: Garbage Collector Statistics
# =============================================================================
# Ruby uses a generational, incremental, mark-and-sweep GC.
# Since Ruby 2.1: generational (young/old objects)
# Since Ruby 2.2: incremental (doesn't stop-the-world for full GC)
# This is simpler than JVM's G1/ZGC but effective for typical Ruby apps.
# =============================================================================

puts "#{'=' * 70}"
puts "  SECTION 4: Garbage Collector Statistics"
puts "#{'=' * 70}"

gc_stats = GC.stat
puts "\n  GC algorithm: Generational Mark-and-Sweep (incremental since 2.2)"
puts "\n  Key statistics:\n\n"

interesting_gc = {
  count:              "Total GC runs",
  minor_gc_count:     "Minor GC runs (young generation only)",
  major_gc_count:     "Major GC runs (full heap scan)",
  heap_allocated_pages: "Heap pages allocated",
  heap_live_slots:    "Live object slots",
  heap_free_slots:    "Free object slots",
  total_allocated_objects: "Total objects ever allocated",
  total_freed_objects: "Total objects ever freed",
  malloc_increase_bytes: "Bytes allocated since last GC",
}

interesting_gc.each do |key, desc|
  value = gc_stats[key]
  next unless value
  formatted = if value.is_a?(Integer) && value > 1000
                value.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,')
              else
                value.to_s
              end
  puts "    %-30s %-15s  %s" % [key.to_s + ":", formatted, "(#{desc})"]
end

# Show ALL GC stats
puts "\n  Full GC.stat dump (#{gc_stats.size} entries):"
gc_stats.each do |key, value|
  formatted = value.is_a?(Integer) && value > 1000 ? value.to_s.gsub(/(\d)(?=(\d{3})+$)/, '\\1,') : value.to_s
  puts "    %-40s %s" % [key.to_s, formatted]
end

puts <<~EXPLAIN

  COMPARE TO JVM GC:
    - JVM has pluggable GC algorithms (G1, ZGC, Shenandoah, etc.)
    - JVM GC is much more sophisticated (concurrent, region-based)
    - Ruby GC is simpler but adequate: most pauses are <10ms
    - JVM's GC handles much larger heaps (TBs) while Ruby tops out at ~4-8GB

  Ruby GC generations:
    - Young objects: recently created, collected frequently (minor GC)
    - Old objects: survived several GC cycles, collected less often (major GC)
    - This is the same generational hypothesis as JVM (most objects die young)
EXPLAIN

# =============================================================================
# SECTION 5: Memory Usage
# =============================================================================

puts "#{'=' * 70}"
puts "  SECTION 5: Memory Usage"
puts "#{'=' * 70}"

# Process memory from /proc
pid = Process.pid
puts "\n  Process ID: #{pid}"

if File.exist?("/proc/#{pid}/status")
  status = File.read("/proc/#{pid}/status")

  memory_fields = {
    "VmSize"  => "Virtual memory size (total address space)",
    "VmRSS"   => "Resident set size (actually in RAM)",
    "VmData"  => "Data segment size (heap)",
    "VmStk"   => "Stack size",
    "VmExe"   => "Text segment size (code)",
    "VmLib"   => "Shared library size",
  }

  puts "\n  Memory breakdown:\n\n"
  memory_fields.each do |field, desc|
    line = status.lines.find { |l| l.start_with?(field) }
    if line
      value = line.split(":").last.strip
      puts "    %-12s %-15s  %s" % [field + ":", value, desc]
    end
  end
end

# Also show from Ruby's perspective
puts "\n  Ruby memory info:"
puts "    GC heap pages:         #{GC.stat[:heap_allocated_pages]}"
puts "    GC heap slots:         #{GC.stat[:heap_live_slots] + GC.stat[:heap_free_slots]}"
puts "    Slot size:             #{GC::INTERNAL_CONSTANTS[:RVALUE_SIZE] rescue '40'} bytes"

slot_size = (GC::INTERNAL_CONSTANTS[:RVALUE_SIZE] rescue 40)
total_slots = GC.stat[:heap_live_slots] + GC.stat[:heap_free_slots]
heap_mb = (total_slots * slot_size) / (1024.0 * 1024.0)
puts "    Estimated heap size:   #{'%.1f' % heap_mb} MB"

# =============================================================================
# SECTION 6: Runtime Configuration
# =============================================================================

puts "\n#{'=' * 70}"
puts "  SECTION 6: Runtime Configuration"
puts "#{'=' * 70}"

puts "\n  Build configuration:"
puts "    CC (C compiler used):    #{RbConfig::CONFIG['CC'] rescue 'unknown'}"
puts "    CFLAGS:                  #{RbConfig::CONFIG['CFLAGS']&.slice(0, 60) rescue 'unknown'}..."
puts "    configure args:          #{RbConfig::CONFIG['configure_args']&.slice(0, 60) rescue 'unknown'}..."
puts "    Target CPU:              #{RbConfig::CONFIG['target_cpu'] rescue 'unknown'}"
puts "    Target OS:               #{RbConfig::CONFIG['target_os'] rescue 'unknown'}"
puts "    sizeof(void*):           #{RbConfig::CONFIG['SIZEOF_VOIDP'] rescue 'unknown'} bytes"

puts "\n  Runtime constants:"
puts "    RUBY_VERSION:            #{RUBY_VERSION}"
puts "    RUBY_PLATFORM:           #{RUBY_PLATFORM}"
puts "    RUBY_ENGINE:             #{RUBY_ENGINE}"
puts "    RUBY_DESCRIPTION:        #{RUBY_DESCRIPTION}"
puts "    Encoding.default_external: #{Encoding.default_external}"
puts "    $LOAD_PATH entries:      #{$LOAD_PATH.length}"

# =============================================================================
# SUMMARY
# =============================================================================
puts "\n#{'=' * 70}"
puts "  SUMMARY: What is MRI?"
puts "#{'=' * 70}"
puts <<~SUMMARY

  MRI (Matz's Ruby Interpreter) is a C program with these major components:

  1. PARSER (parse.y)
     - ~13,000 lines of Bison grammar
     - Converts Ruby source to AST
     - One of the most complex parsers in any language (Ruby's grammar is huge)

  2. COMPILER (compile.c)
     - Walks the AST, emits YARV bytecode
     - Handles optimizations like opt_plus, opt_lt (specialized fast paths)

  3. YARV VM (vm_exec.c, insns.def)
     - A big switch/case loop over opcodes
     - Each opcode: pop args from stack, do work, push result
     - Direct-threaded dispatch for speed (computed goto)

  4. GARBAGE COLLECTOR (gc.c)
     - Generational mark-and-sweep
     - ~10,000 lines of careful C code
     - Manages all Ruby object memory

  5. CORE LIBRARY (string.c, array.c, hash.c, ...)
     - Built-in classes implemented in C for speed
     - String, Array, Hash, Integer, etc.
     - These are where most time is spent in Ruby programs

  6. C EXTENSION API (ruby.h, intern.h)
     - Allows C/C++ code to create Ruby objects, call methods, etc.
     - Used by gems like nokogiri, pg, mysql2, ffi
     - See the c_extension/ directory in this module

  Total: ~400,000 lines of C code to implement Ruby.

SUMMARY
