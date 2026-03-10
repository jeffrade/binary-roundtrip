# =============================================================================
# Makefile — Module 9: Ruby MRI Essentials
# =============================================================================
#
# Usage:
#   make all            — run all demos (except C extension which needs compilation)
#   make bytecode       — explore YARV bytecode
#   make ast            — explore the Ruby AST
#   make gil            — demonstrate the GIL/GVL
#   make yjit           — benchmark YJIT (with --yjit flag if supported)
#   make internals      — explore MRI internals
#   make require-trace  — trace require calls
#   make c-extension    — build the C extension
#   make test-extension — build and test the C extension
#   make benchmark      — compare C extension vs pure Ruby
#   make clean          — remove compiled artifacts
# =============================================================================

RUBY := ruby
RUBY_DIR := ruby
EXT_DIR := $(RUBY_DIR)/c_extension

# Check for YJIT support
YJIT_FLAG := $(shell $(RUBY) -e 'puts "--yjit" if RUBY_VERSION >= "3.1"' 2>/dev/null)

.PHONY: all bytecode ast gil yjit internals require-trace \
        c-extension test-extension benchmark clean help

# =============================================================================
# Default target: run all pure-Ruby demos
# =============================================================================
all: bytecode ast gil yjit internals require-trace
	@echo ""
	@echo "================================================"
	@echo "  All Ruby demos complete!"
	@echo "  For C extension demos, run: make test-extension"
	@echo "================================================"

# =============================================================================
# Individual demo targets
# =============================================================================

bytecode:
	@echo "========================================"
	@echo "  Running YARV Bytecode Explorer..."
	@echo "========================================"
	$(RUBY) $(RUBY_DIR)/yarv_bytecode.rb

ast:
	@echo "========================================"
	@echo "  Running AST Explorer..."
	@echo "========================================"
	$(RUBY) $(RUBY_DIR)/ast_explorer.rb

gil:
	@echo "========================================"
	@echo "  Running GIL/GVL Demo..."
	@echo "  (this takes ~15-30 seconds)"
	@echo "========================================"
	$(RUBY) $(RUBY_DIR)/gil_demo.rb

yjit:
	@echo "========================================"
	@echo "  Running YJIT Demo..."
	@echo "========================================"
ifdef YJIT_FLAG
	@echo "  First run: without YJIT (baseline)"
	$(RUBY) $(RUBY_DIR)/yjit_demo.rb
	@echo ""
	@echo "  Second run: with YJIT enabled"
	$(RUBY) --yjit $(RUBY_DIR)/yjit_demo.rb
else
	@echo "  YJIT requires Ruby 3.1+ (you have: $$($(RUBY) -e 'print RUBY_VERSION'))"
	@echo "  Running without YJIT..."
	$(RUBY) $(RUBY_DIR)/yjit_demo.rb
endif

internals:
	@echo "========================================"
	@echo "  Running MRI Internals Explorer..."
	@echo "========================================"
	$(RUBY) $(RUBY_DIR)/mri_internals.rb

require-trace:
	@echo "========================================"
	@echo "  Running Require Trace..."
	@echo "========================================"
	$(RUBY) $(RUBY_DIR)/require_trace.rb

# =============================================================================
# C Extension targets
# =============================================================================

c-extension:
	@echo "========================================"
	@echo "  Building C Extension..."
	@echo "========================================"
	cd $(EXT_DIR) && $(RUBY) extconf.rb && $(MAKE)
	@echo ""
	@echo "  C extension compiled successfully!"
	@echo "  Output: $(EXT_DIR)/fast_math.so"

test-extension: c-extension
	@echo ""
	@echo "========================================"
	@echo "  Testing C Extension..."
	@echo "========================================"
	cd $(EXT_DIR) && $(RUBY) test_ext.rb

benchmark: c-extension
	@echo ""
	@echo "========================================"
	@echo "  Benchmarking C Extension vs Pure Ruby..."
	@echo "========================================"
	cd $(EXT_DIR) && $(RUBY) test_ext.rb

# =============================================================================
# Clean up
# =============================================================================
clean:
	@echo "Cleaning up compiled artifacts..."
	cd $(EXT_DIR) && \
		rm -f Makefile fast_math.so fast_math.o fast_math.bundle mkmf.log .fast_math.time
	@echo "Clean complete."

# =============================================================================
# Help
# =============================================================================
help:
	@echo "Module 9: Ruby MRI Essentials"
	@echo ""
	@echo "Available targets:"
	@echo "  make all            — run all pure-Ruby demos"
	@echo "  make bytecode       — explore YARV bytecode"
	@echo "  make ast            — explore the Ruby AST"
	@echo "  make gil            — demonstrate the GIL/GVL"
	@echo "  make yjit           — benchmark YJIT (Ruby 3.1+)"
	@echo "  make internals      — explore MRI internals"
	@echo "  make require-trace  — trace require calls"
	@echo "  make c-extension    — compile the C extension"
	@echo "  make test-extension — compile and test C extension"
	@echo "  make benchmark      — C extension vs Ruby benchmark"
	@echo "  make clean          — remove compiled files"
	@echo ""
	@echo "Ruby version: $$($(RUBY) -v)"
