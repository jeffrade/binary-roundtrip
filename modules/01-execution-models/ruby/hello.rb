#!/usr/bin/env ruby
#
# Module 1: Execution Models — Ruby (Interpreted Language)
# ========================================================
#
# WHAT THIS FILE DEMONSTRATES:
#   Ruby is an interpreted language. When you run `ruby hello.rb`, the MRI
#   (Matz's Ruby Interpreter) reads your source code, parses it into an
#   Abstract Syntax Tree (AST), compiles it to YARV bytecode, and then
#   the YARV virtual machine executes that bytecode.
#
# THE EXECUTION MODEL:
#   hello.rb → [MRI Parser] → AST → [Compiler] → YARV Bytecode → [YARV VM] → Output
#
#   1. Lexing/Parsing:  MRI reads the .rb file and builds an AST.
#   2. Compilation:     The AST is compiled to YARV bytecode instructions.
#   3. Execution:       The YARV VM interprets the bytecode instructions.
#
#   Note: There is NO separate "compile" step you run manually. This all
#   happens every time you run the script. The bytecode lives in memory
#   only (unlike Java's .class files).
#
# WHAT TO OBSERVE:
#   - Run:     ruby hello.rb
#   - Notice:  No compilation step needed — just run the source directly.
#   - Notice:  No binary artifact is produced (no .exe, no .class file).
#   - Compare: `file hello.rb` → "Ruby script, ASCII text" (it's just text!)
#   - Try:     ruby -e 'puts RubyVM::InstructionSequence.compile("1+2").disasm'
#              This shows you the YARV bytecode that Ruby generates internally.
#
# KEY INSIGHT:
#   Unlike C, your CPU never directly executes your Ruby code. The MRI
#   interpreter is the native program that runs, and it processes your
#   Ruby code as data. This adds overhead but provides flexibility:
#   eval(), dynamic method definition, monkey patching, etc.
#
# COMPARE WITH:
#   - C/Rust:  Compiled to native code — CPU runs your code directly.
#   - Java:    Compiled to bytecode files (.class) — JVM runs the bytecode.
#   - Ruby:    Parsed + compiled to bytecode at runtime — YARV VM runs it.

puts "Hello from Ruby!"
puts
puts "Execution model: INTERPRETED (with internal bytecode compilation)"
puts "  - MRI parsed this .rb file into an AST."
puts "  - The AST was compiled to YARV bytecode (in memory, not saved to disk)."
puts "  - The YARV virtual machine is executing that bytecode right now."
puts

# Show some runtime info to prove we're running inside MRI
puts "Runtime environment:"
puts "  Ruby version:    #{RUBY_VERSION}"
puts "  Ruby platform:   #{RUBY_PLATFORM}"
puts "  Ruby engine:     #{RUBY_ENGINE} (#{RUBY_ENGINE_VERSION})"
puts "  Process ID:      #{Process.pid}"
puts "  Script file:     #{__FILE__}"
puts

# Show the actual MRI process — it's the interpreter binary that's running
puts "What's actually running on the CPU:"
puts "  The `ruby` interpreter binary (an ELF executable written in C)"
puts "  Your .rb file is just data that the interpreter processes."
puts
puts "Try: `file $(which ruby)` to see that the interpreter itself is a native binary."
puts "Try: `ruby bytecode_explorer.rb` to see the bytecode Ruby generates."
