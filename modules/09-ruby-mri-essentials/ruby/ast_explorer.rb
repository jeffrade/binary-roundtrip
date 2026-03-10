#!/usr/bin/env ruby
# =============================================================================
# Ruby AST Explorer — Module 9: Ruby MRI Essentials
# =============================================================================
#
# PURPOSE:
#   Before Ruby code becomes YARV bytecode, it passes through the parser which
#   produces an Abstract Syntax Tree (AST). This is the FIRST representation
#   of your code that Ruby creates internally.
#
#   The full pipeline:
#     Source Code → Tokenize → PARSE (AST) → Compile → YARV Bytecode → Execute
#                               ^^^^^^^^^^^
#                               WE ARE HERE
#
# WHAT TO OBSERVE:
#   - The AST is a tree of nodes, each with a type (e.g., :OPCALL, :IF, :LIT)
#   - Leaf nodes are literals (numbers, strings, symbols)
#   - Interior nodes represent operations (calls, assignments, control flow)
#   - The AST is what the compiler walks to generate YARV bytecode
#   - Compare to: clang's AST for C, javac's AST for Java, rustc's HIR
#
# RUN:
#   ruby ast_explorer.rb                         # uses built-in examples
#   ruby ast_explorer.rb "1 + 2 * 3"            # parse a custom expression
#   ruby ast_explorer.rb "def greet(name); puts \"Hello, \#{name}\"; end"
#
# NOTE: RubyVM::AbstractSyntaxTree is available in Ruby 2.6+
# =============================================================================

puts "=" * 70
puts "RUBY AST EXPLORER"
puts "Ruby #{RUBY_VERSION} — RubyVM::AbstractSyntaxTree"
puts "=" * 70

# =============================================================================
# HELPER: Recursively print the AST in a readable tree format
# =============================================================================
# The AST nodes from RubyVM::AbstractSyntaxTree have:
#   .type      — Symbol like :OPCALL, :IF, :LIT, :SCOPE, etc.
#   .children  — Array of child nodes (may include raw values like integers)
#   .first_lineno, .first_column — source location
# =============================================================================
def print_ast(node, indent = 0, prefix = "")
  # Handle non-AST-node values (literals, nil, symbols that appear as children)
  unless node.is_a?(RubyVM::AbstractSyntaxTree::Node)
    label = node.nil? ? "nil" : node.inspect
    puts "#{' ' * indent}#{prefix}#{label}"
    return
  end

  # Print this node's type and source location
  location = "  (line #{node.first_lineno}, col #{node.first_column})"
  puts "#{' ' * indent}#{prefix}#{node.type}#{location}"

  # Recursively print children
  node.children.each_with_index do |child, i|
    child_prefix = if i == node.children.length - 1
                     "\u2514\u2500 "   # corner: last child
                   else
                     "\u251C\u2500 "   # tee: not last child
                   end
    print_ast(child, indent + 2, child_prefix)
  end
end

# =============================================================================
# NODE TYPE REFERENCE
# =============================================================================
# Here's what the most common AST node types mean:
# =============================================================================
NODE_TYPES = {
  "SCOPE"    => "Top-level scope or method body (contains local var table + body)",
  "OPCALL"   => "Operator call like +, -, *, / (syntactic sugar for method call)",
  "CALL"     => "Normal method call (receiver.method(args))",
  "FCALL"    => "Function-style call (method(args) — implicit self receiver)",
  "VCALL"    => "Variable-or-call (bare word — could be local var or method)",
  "IF"       => "If/elsif/else conditional",
  "UNLESS"   => "Unless conditional (inverted if)",
  "WHILE"    => "While loop",
  "UNTIL"    => "Until loop (inverted while)",
  "FOR"      => "For loop (desugars to .each)",
  "ITER"     => "Block iteration (method call with a block)",
  "BLOCK"    => "Block body (do...end or {...})",
  "LAMBDA"   => "Lambda literal (-> { })",
  "DEFN"     => "Method definition (def name...end)",
  "DEFS"     => "Singleton method definition (def self.name...end)",
  "CLASS"    => "Class definition",
  "MODULE"   => "Module definition",
  "LIT"      => "Literal value (number, symbol, regex)",
  "STR"      => "String literal",
  "DSTR"     => "Dynamic string (string interpolation)",
  "ARRAY"    => "Array literal",
  "HASH"     => "Hash literal",
  "LASGN"    => "Local variable assignment",
  "LVAR"     => "Local variable reference",
  "IASGN"    => "Instance variable assignment (@var = ...)",
  "IVAR"     => "Instance variable reference (@var)",
  "DASGN"    => "Dynamic variable assignment (block-local)",
  "DVAR"     => "Dynamic variable reference (block-local)",
  "RETURN"   => "Explicit return statement",
  "YIELD"    => "Yield to block",
  "AND"      => "Logical AND (&&)",
  "OR"       => "Logical OR (||)",
  "MATCH3"   => "Regex match (=~ operator)",
}

# =============================================================================
# Parse and display code
# =============================================================================
def explore(label, code)
  puts "\n#{'=' * 70}"
  puts "  #{label}"
  puts "#{'=' * 70}"
  puts "\n  Source code:"
  code.each_line.with_index(1) do |line, num|
    puts "    #{num}: #{line}"
  end

  begin
    ast = RubyVM::AbstractSyntaxTree.parse(code)
    puts "\n  Abstract Syntax Tree:"
    print_ast(ast, 4)

    # Also show the bytecode this compiles to, for comparison
    puts "\n  Compiled YARV bytecode:"
    iseq = RubyVM::InstructionSequence.compile(code)
    iseq.disasm.each_line do |line|
      puts "    #{line}"
    end
  rescue SyntaxError => e
    puts "\n  SYNTAX ERROR: #{e.message}"
  end

  puts
end

# =============================================================================
# If user provided code on command line, parse that
# =============================================================================
if ARGV.length > 0
  user_code = ARGV.join(" ")
  explore("USER-PROVIDED CODE", user_code)
  puts "\n  (Run without arguments to see built-in examples)"

else
  # ===========================================================================
  # EXAMPLE 1: Simple arithmetic — shows how operators become OPCALL nodes
  # ===========================================================================
  # In Ruby, 1 + 2 is actually 1.+(2) — a method call on Integer.
  # The AST shows this as OPCALL (operator call), not as a primitive add.
  # This is fundamentally different from C/Java where + is a CPU instruction.
  # ===========================================================================
  explore(
    "EXAMPLE 1: Arithmetic Expression (1 + 2 * 3)",
    "1 + 2 * 3"
  )

  puts <<~NOTE
    WHAT TO NOTICE:
    - '2 * 3' is nested INSIDE '1 + ...' — operator precedence is in the tree!
    - Both + and * are OPCALL nodes — they're method calls, not primitives
    - The AST captures the STRUCTURE; bytecode will flatten it to a sequence
    - Compare to C: the C compiler's AST looks similar, but compiles to 'add'/'mul' CPU instructions
  NOTE

  # ===========================================================================
  # EXAMPLE 2: If/Else — shows conditional branching in the tree
  # ===========================================================================
  explore(
    "EXAMPLE 2: If/Else Conditional",
    <<~RUBY
      if x > 10
        "big"
      else
        "small"
      end
    RUBY
  )

  puts <<~NOTE
    WHAT TO NOTICE:
    - The IF node has 3 children: condition, then-body, else-body
    - The condition (x > 10) is an OPCALL node (> is a method in Ruby!)
    - The tree structure is clear: condition branches to two possible bodies
    - The compiler will turn this into branchunless + jump (see yarv_bytecode.rb)
  NOTE

  # ===========================================================================
  # EXAMPLE 3: Method definition — shows DEFN node structure
  # ===========================================================================
  explore(
    "EXAMPLE 3: Method Definition",
    <<~RUBY
      def greet(name)
        puts "Hello, \#{name}!"
      end
    RUBY
  )

  puts <<~NOTE
    WHAT TO NOTICE:
    - DEFN wraps the entire method definition
    - SCOPE node contains the method body (methods create their own scope)
    - DSTR = dynamic string (has interpolation) — contains STR + DVAR parts
    - FCALL = function-style call (puts with implicit self)
    - The method name and parameter list are encoded in the tree
  NOTE

  # ===========================================================================
  # EXAMPLE 4: Block iteration — shows ITER node
  # ===========================================================================
  explore(
    "EXAMPLE 4: Block Iteration",
    <<~RUBY
      [1, 2, 3].each do |x|
        puts x * 2
      end
    RUBY
  )

  puts <<~NOTE
    WHAT TO NOTICE:
    - ITER node wraps the method call + block as a unit
    - The CALL to .each is one child; the block body is another
    - Block parameters (|x|) appear in the scope's local variable table
    - This is why blocks in Ruby are special — they're baked into the AST structure
    - In JVM, lambdas are compiled to inner classes; in Ruby, blocks are first-class AST nodes
  NOTE

  # ===========================================================================
  # EXAMPLE 5: Class definition — shows the full structure
  # ===========================================================================
  explore(
    "EXAMPLE 5: Class Definition",
    <<~RUBY
      class Dog
        def initialize(name)
          @name = name
        end

        def speak
          "Woof! I'm \#{@name}"
        end
      end
    RUBY
  )

  puts <<~NOTE
    WHAT TO NOTICE:
    - CLASS node wraps everything, with the class name and superclass
    - Each method is a DEFN node inside the class body
    - @name is IASGN (instance variable assignment) and IVAR (reference)
    - Ruby classes are OPEN: this AST just sends messages to create a class object
    - Unlike Java where a class is a compile-time blueprint, Ruby class definitions are EXECUTED CODE
  NOTE
end

# =============================================================================
# NODE TYPE REFERENCE TABLE
# =============================================================================
puts "\n#{'=' * 70}"
puts "  AST NODE TYPE REFERENCE"
puts "#{'=' * 70}"
NODE_TYPES.each do |type, desc|
  puts "  %-10s — %s" % [type, desc]
end

# =============================================================================
# PIPELINE SUMMARY
# =============================================================================
puts "\n#{'=' * 70}"
puts "  THE FULL PIPELINE"
puts "#{'=' * 70}"
puts <<~SUMMARY

  +-----------------+     +---------+     +----------+     +-----------+
  |  Source Code     | --> | Tokens  | --> |   AST    | --> |   YARV    |
  |  "1 + 2 * 3"    |     | 1,+,2,  |     | OPCALL   |     | putobject |
  |                 |     | *,3     |     |  / \\     |     | putobject |
  |                 |     |         |     | 1 OPCALL |     | opt_mult  |
  |                 |     |         |     |    / \\   |     | opt_plus  |
  |                 |     |         |     |   2   3  |     | leave     |
  +-----------------+     +---------+     +----------+     +-----------+
                                            ^^^^^^^^
                                          WE EXPLORED
                                           THIS STAGE

  The AST is the structured representation of your code.
  It captures: operator precedence, nesting, scoping, names.
  The compiler walks this tree to emit flat YARV bytecode.

  This is the same pattern in ALL compiled languages:
    C:     source → AST → IR (LLVM/GCC) → machine code
    Java:  source → AST → JVM bytecode → (JIT) machine code
    Rust:  source → AST → HIR → MIR → LLVM IR → machine code
    Ruby:  source → AST → YARV bytecode → (YJIT) machine code

SUMMARY
