#!/usr/bin/env ruby
#
# Module 1: Execution Models — Ruby Bytecode Explorer
# ====================================================
#
# WHAT THIS FILE DEMONSTRATES:
#   Ruby doesn't just "interpret" your code line-by-line. Internally, MRI
#   parses your Ruby code into an Abstract Syntax Tree (AST), then compiles
#   that AST into YARV (Yet Another Ruby VM) bytecode instructions. This
#   script lets you see both the AST and the bytecode for any expression.
#
# USAGE:
#   ruby bytecode_explorer.rb                  # Uses default expressions
#   ruby bytecode_explorer.rb "2 + 3 * 4"     # Explore a custom expression
#   ruby bytecode_explorer.rb "def foo; 42; end; foo"
#
# YARV BYTECODE INSTRUCTIONS REFERENCE:
#   putnil          — Push nil onto the stack
#   putself         — Push the current self onto the stack
#   putobject N     — Push a literal value (integer, string, etc.) onto the stack
#   putstring "s"   — Push a string onto the stack
#   opt_plus        — Optimized addition (pops two values, pushes result)
#   opt_minus       — Optimized subtraction
#   opt_mult        — Optimized multiplication
#   opt_div         — Optimized division
#   opt_lt          — Optimized less-than comparison
#   opt_send_without_block — Call a method
#   setlocal_WC_0   — Store top of stack into a local variable
#   getlocal_WC_0   — Push a local variable onto the stack
#   leave           — Return from the current scope
#   branchif        — Conditional branch (jump if top of stack is truthy)
#   branchunless    — Conditional branch (jump if top of stack is falsy)
#   jump            — Unconditional jump
#
# KEY INSIGHT:
#   YARV is a stack-based virtual machine, similar in concept to the JVM.
#   But unlike Java, Ruby compiles to bytecode every time you run the
#   script — there is no persistent .class file equivalent.

# --------------------------------------------------------------------------
# Helper: Print a section banner
# --------------------------------------------------------------------------
def banner(title)
  width = 70
  puts
  puts "=" * width
  puts " #{title}"
  puts "=" * width
  puts
end

# --------------------------------------------------------------------------
# Helper: Print the AST for a Ruby expression
# --------------------------------------------------------------------------
def show_ast(expression)
  banner("STAGE 1: Abstract Syntax Tree (AST)")
  puts "Expression: #{expression}"
  puts
  begin
    # RubyVM::AbstractSyntaxTree.parse returns an AST node.
    # This is the tree structure the parser builds from your source code.
    # The compiler then walks this tree to generate bytecode.
    ast = RubyVM::AbstractSyntaxTree.parse(expression)
    puts "AST structure:"
    print_ast_node(ast, indent: "  ")
  rescue SyntaxError => e
    puts "Syntax error: #{e.message}"
  end
end

# --------------------------------------------------------------------------
# Helper: Recursively print AST nodes
# --------------------------------------------------------------------------
def print_ast_node(node, indent: "")
  return unless node.is_a?(RubyVM::AbstractSyntaxTree::Node)

  # Each AST node has a type (e.g., :OPCALL for operator calls, :LIT for
  # literals, :SCOPE for scope boundaries). The children are sub-nodes or
  # literal values.
  puts "#{indent}#{node.type}"

  node.children.each_with_index do |child, i|
    if child.is_a?(RubyVM::AbstractSyntaxTree::Node)
      print_ast_node(child, indent: indent + "  ")
    elsif !child.nil?
      puts "#{indent}  [#{i}] #{child.inspect}"
    end
  end
end

# --------------------------------------------------------------------------
# Helper: Show YARV bytecode disassembly
# --------------------------------------------------------------------------
def show_bytecode(expression)
  banner("STAGE 2: YARV Bytecode (Disassembly)")
  puts "Expression: #{expression}"
  puts

  # RubyVM::InstructionSequence.compile() takes Ruby source code and
  # compiles it to YARV bytecode. The .disasm method gives us a
  # human-readable disassembly of those bytecode instructions.
  #
  # This is exactly what MRI does internally every time you run a .rb file,
  # except normally you never see this bytecode — it stays in memory.
  iseq = RubyVM::InstructionSequence.compile(expression)
  disasm = iseq.disasm

  puts "Disassembly:"
  puts
  # Print each line with annotation for common instructions
  disasm.each_line do |line|
    print "  #{line}"
  end
  puts
  puts "Reading the bytecode:"
  puts "  - Numbers on the left (0000, 0001, ...) are instruction offsets."
  puts "  - YARV is stack-based: values are pushed/popped from a stack."
  puts "  - 'putobject N' pushes the value N onto the stack."
  puts "  - 'opt_plus' pops two values, adds them, pushes the result."
  puts "  - 'leave' returns from the current scope."
end

# --------------------------------------------------------------------------
# Helper: Execute the expression and show the result
# --------------------------------------------------------------------------
def show_execution(expression)
  banner("STAGE 3: Execution Result")
  puts "Expression: #{expression}"
  puts
  begin
    # eval() takes a string of Ruby code and executes it. This mirrors
    # what MRI does: parse → compile to bytecode → execute on YARV VM.
    result = eval(expression)  # rubocop:disable Security/Eval
    puts "Result:     #{result.inspect}"
    puts "Type:       #{result.class}"
  rescue => e
    puts "Error: #{e.class}: #{e.message}"
  end
end

# --------------------------------------------------------------------------
# Main: Explore one or more expressions
# --------------------------------------------------------------------------

# If the user provides an expression on the command line, use that.
# Otherwise, use a curated set of examples that demonstrate different
# bytecode patterns.
if ARGV.length > 0
  expressions = [ARGV.join(" ")]
else
  expressions = [
    # Simple arithmetic — shows putobject and opt_plus/opt_mult
    "2 + 3 * 4",

    # Variable assignment — shows setlocal and getlocal
    "x = 10; y = 20; x + y",

    # Conditional — shows branchunless and jump instructions
    "x = 42; if x > 0 then 'positive' else 'negative' end",

    # Method call — shows opt_send_without_block
    "'hello world'.upcase.length",
  ]
end

puts "#{'=' * 70}"
puts " Ruby Bytecode Explorer"
puts " Execution model: Source → AST → YARV Bytecode → YARV VM → Result"
puts "#{'=' * 70}"

expressions.each do |expr|
  puts
  puts "*" * 70
  puts " Exploring: #{expr}"
  puts "*" * 70

  show_ast(expr)
  show_bytecode(expr)
  show_execution(expr)
end

banner("SUMMARY")
puts "What you just saw:"
puts "  1. PARSING:     Ruby source code → Abstract Syntax Tree (AST)"
puts "  2. COMPILATION: AST → YARV bytecode instructions (in memory)"
puts "  3. EXECUTION:   YARV virtual machine interprets the bytecode"
puts
puts "This happens every time you run a Ruby script. Unlike Java, no"
puts ".class file is saved to disk. Unlike C, no native machine code"
puts "is generated — the YARV VM (written in C) interprets the bytecode."
puts
puts "Try your own expressions:"
puts "  ruby #{__FILE__} \"def greet(name); puts \\\"Hello \\\#{name}\\\"; end; greet('world')\""
puts "  ruby #{__FILE__} \"[1,2,3].map { |x| x * x }\""
puts "  ruby #{__FILE__} \"(1..10).select(&:even?).sum\""
