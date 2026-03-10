#!/usr/bin/env ruby
# =============================================================================
# YARV Bytecode Explorer — Module 9: Ruby MRI Essentials
# =============================================================================
#
# PURPOSE:
#   Ruby doesn't interpret source code directly. Like Java, it compiles to
#   bytecode first — but unlike Java, this happens EVERY TIME you run a script.
#
#   The pipeline is:
#     Source Code → Tokenize → Parse (AST) → Compile → YARV Bytecode → Execute
#
#   YARV = "Yet Another Ruby VM" — the bytecode interpreter inside MRI since
#   Ruby 1.9. Before 1.9, Ruby walked the AST directly (very slow).
#
# WHAT TO OBSERVE:
#   - Each Ruby method compiles to a sequence of YARV instructions
#   - Simple operations (+ - * /) use optimized "opt_" instructions
#   - Control flow uses branchif/branchunless/jump (like JVM's if_icmpXX/goto)
#   - Method calls use "send" (Ruby's universal dispatch mechanism)
#   - Blocks create separate instruction sequences (like inner classes in JVM)
#
# KEY DIFFERENCE FROM JVM:
#   - JVM bytecode is saved to .class files and loaded later
#   - YARV bytecode lives only in memory — recompiled every run
#   - This is why Ruby startup is slower than Java (once JVM is warm)
#
# RUN: ruby yarv_bytecode.rb
# =============================================================================

puts "=" * 70
puts "YARV BYTECODE EXPLORER"
puts "Ruby #{RUBY_VERSION} (#{RUBY_ENGINE})"
puts "=" * 70

# =============================================================================
# HELPER: Pretty-print disassembly with annotations
# =============================================================================
def show_bytecode(label, iseq)
  puts "\n#{'=' * 70}"
  puts "  #{label}"
  puts "#{'=' * 70}"

  disasm = iseq.disasm
  puts disasm

  # Annotate key instructions
  annotations = {
    "putself"       => "Push 'self' onto the stack (receiver for method calls)",
    "putobject"     => "Push a literal object (number, symbol, etc.) onto the stack",
    "putstring"     => "Push a string literal onto the stack",
    "putnil"        => "Push nil onto the stack",
    "opt_plus"      => "Optimized addition — avoids full method dispatch for Fixnum#+",
    "opt_minus"     => "Optimized subtraction",
    "opt_mult"      => "Optimized multiplication",
    "opt_lt"        => "Optimized less-than comparison",
    "opt_le"        => "Optimized less-than-or-equal",
    "opt_gt"        => "Optimized greater-than comparison",
    "opt_ge"        => "Optimized greater-than-or-equal",
    "opt_eq"        => "Optimized equality check",
    "opt_send_without_block" => "Method call (no block passed)",
    "leave"         => "Return from this method/block (like JVM 'return')",
    "branchif"      => "Pop stack; jump if truthy (like JVM 'ifne')",
    "branchunless"  => "Pop stack; jump if falsy (like JVM 'ifeq')",
    "jump"          => "Unconditional jump (like JVM 'goto')",
    "send"          => "Full method dispatch with block (most general call)",
    "getlocal_WC_0" => "Load a local variable onto the stack",
    "setlocal_WC_0" => "Store top of stack into a local variable",
    "duparray"      => "Duplicate an array literal onto the stack",
    "newarray"      => "Create a new array from stack elements",
    "pop"           => "Discard top of stack",
    "dup"           => "Duplicate top of stack",
    "nop"           => "No operation (alignment/placeholder)",
  }

  found = []
  annotations.each do |insn, desc|
    if disasm.include?(insn)
      found << "  %-30s — %s" % [insn, desc]
    end
  end

  unless found.empty?
    puts "\n  INSTRUCTION GLOSSARY (instructions seen above):"
    puts "  #{'-' * 60}"
    found.each { |line| puts line }
  end

  puts
end

# =============================================================================
# EXAMPLE 1: Simple Addition
# =============================================================================
# This is the simplest possible method. Observe:
#   - putobject loads the arguments
#   - opt_plus does the addition (optimized path, no method lookup)
#   - leave returns the result
# =============================================================================

def simple_add(a, b)
  a + b
end

iseq1 = RubyVM::InstructionSequence.of(method(:simple_add))
show_bytecode("EXAMPLE 1: simple_add(a, b) — Simple Addition", iseq1)

puts <<~EXPLAIN
  WHAT TO NOTICE:
  - 'opt_plus' is an OPTIMIZED instruction for addition
  - Ruby could call Fixnum#+ as a normal method (using 'send')
  - But opt_plus shortcuts this for known types — a key optimization
  - If you override '+' on a class, Ruby falls back to 'send'
  - Compare this to JVM's 'iadd' which is always a CPU instruction

EXPLAIN

# =============================================================================
# EXAMPLE 2: Conditional (if/else)
# =============================================================================
# Control flow in YARV uses branch instructions:
#   - branchunless: jump if the condition is false
#   - branchif: jump if the condition is true
#   - jump: unconditional (used to skip the else after the if-body)
# This is almost identical to how JVM handles if/else!
# =============================================================================

def conditional(x)
  if x > 10
    "big"
  else
    "small"
  end
end

iseq2 = RubyVM::InstructionSequence.of(method(:conditional))
show_bytecode("EXAMPLE 2: conditional(x) — If/Else Branching", iseq2)

puts <<~EXPLAIN
  WHAT TO NOTICE:
  - 'opt_gt' checks x > 10 (optimized greater-than)
  - 'branchunless' jumps to the else branch if false
  - After the if-body, 'jump' skips over the else-body
  - This is exactly like JVM's: if_icmple → else_label, goto end_label
  - The pattern is universal: condition → branch → body → jump

EXPLAIN

# =============================================================================
# EXAMPLE 3: While Loop
# =============================================================================
# Loops compile to:
#   - A label at the top
#   - Condition check
#   - branchunless to exit
#   - Body
#   - jump back to top
# Again, identical structure to JVM/x86 loops.
# =============================================================================

def loop_example
  i = 0
  sum = 0
  while i < 10
    sum += i
    i += 1
  end
  sum
end

iseq3 = RubyVM::InstructionSequence.of(method(:loop_example))
show_bytecode("EXAMPLE 3: loop_example — While Loop", iseq3)

puts <<~EXPLAIN
  WHAT TO NOTICE:
  - 'opt_lt' checks i < 10
  - 'branchunless' exits the loop when condition is false
  - 'jump' at the end goes back to the condition check
  - 'setlocal_WC_0' / 'getlocal_WC_0' read/write local variables
  - Local variables are stored by INDEX, not name (like JVM local var slots)
  - WC_0 means "in the current scope" (WC = "wildcard" for scope depth)

EXPLAIN

# =============================================================================
# EXAMPLE 4: Block (Ruby's Closure)
# =============================================================================
# Blocks are one of Ruby's most distinctive features. Observe:
#   - The block compiles to a SEPARATE instruction sequence
#   - 'send' is used with a block argument (not opt_send_without_block)
#   - The block captures variables from the enclosing scope
# This is similar to JVM lambdas but happens at the bytecode level.
# =============================================================================

def block_example
  [1, 2, 3].each do |x|
    puts x * 2
  end
end

iseq4 = RubyVM::InstructionSequence.of(method(:block_example))
show_bytecode("EXAMPLE 4: block_example — Block/Closure (.each)", iseq4)

puts <<~EXPLAIN
  WHAT TO NOTICE:
  - 'send' is used instead of 'opt_send_without_block' — because there's a block!
  - The block itself is a SEPARATE instruction sequence (you can see "block in block_example")
  - Inside the block: getlocal loads 'x', opt_mult multiplies, putself + send calls 'puts'
  - Blocks are closures: they can access variables from the enclosing scope
  - This is why Ruby method dispatch is complex: any call might receive a block

EXPLAIN

# =============================================================================
# EXAMPLE 5: Compile arbitrary code strings
# =============================================================================
# You can compile any string of Ruby code to YARV bytecode.
# This is useful for experimentation.
# =============================================================================

puts "=" * 70
puts "  EXAMPLE 5: Compiling Code Strings Directly"
puts "=" * 70

examples = {
  "1 + 2"             => "Simple arithmetic",
  "x = 5; x * x"     => "Variable assignment and use",
  "'hello'.upcase"    => "Method call on string",
  "[1,2,3].map {|x| x**2}" => "Map with block",
  "def foo; 42; end"  => "Method definition",
}

examples.each do |code, desc|
  puts "\n  --- #{desc}: #{code.inspect} ---"
  iseq = RubyVM::InstructionSequence.compile(code)
  puts iseq.disasm
end

# =============================================================================
# EXAMPLE 6: Bytecode as data (to_a)
# =============================================================================
# The instruction sequence can be dumped as a Ruby array.
# This shows the raw data structure that YARV works with.
# =============================================================================

puts "\n#{'=' * 70}"
puts "  EXAMPLE 6: Bytecode as Data Structure (to_a)"
puts "#{'=' * 70}"

iseq = RubyVM::InstructionSequence.compile("1 + 2")
data = iseq.to_a

puts "\n  Bytecode metadata:"
puts "    Magic:      #{data[0]}"
puts "    Major ver:  #{data[1]}"
puts "    Minor ver:  #{data[2]}"
puts "    Format:     #{data[3]}"
puts "    Type:       #{data[9]}"
puts "    Name:       #{data[5]}"

puts "\n  Raw instruction array (last element):"
data.last.each do |insn|
  puts "    #{insn.inspect}" if insn.is_a?(Array)
end

# =============================================================================
# SUMMARY
# =============================================================================
puts "\n#{'=' * 70}"
puts "  SUMMARY: Ruby's Execution Pipeline"
puts "#{'=' * 70}"
puts <<~SUMMARY

  Source Code (.rb file)
       |
       v
  Tokenizer (lexer) — breaks code into tokens
       |
       v
  Parser — builds Abstract Syntax Tree (see ast_explorer.rb)
       |
       v
  Compiler — generates YARV bytecode (what we just explored)
       |
       v
  YARV VM — executes bytecode instruction by instruction
       |
       v
  (Optional) YJIT — JIT-compiles hot bytecode to machine code (see yjit_demo.rb)

  KEY INSIGHT:
  Unlike JVM, all of this happens EVERY TIME you run a Ruby script.
  There's no equivalent of .class files — bytecode is never saved to disk.
  This is why Ruby has slower startup than Java (but faster than you'd think,
  because YARV compilation is very fast — it's execution that's slower).

SUMMARY
