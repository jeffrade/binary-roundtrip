# ==============================================================================
# Module 8: JVM Essentials — Makefile
# ==============================================================================
#
# Demonstrates what the JVM adds (and costs) vs. a native binary:
# class loading, JIT warmup, bytecode structure, garbage collection,
# and native execution comparison.
#
# Use `make help` to see all available targets.
#
# Targets:
#   make all             — Compile all Java files
#   make classloading    — Compile and run ClassLoadingDemo
#   make jit             — Compile and run JITWarmup (with -XX:+PrintCompilation)
#   make bytecode        — Compile BytecodeInspector, run javap -c -verbose
#   make gc              — Run GCDemo with -verbose:gc
#   make native-compare  — Run NativeCompare
#   make jruby           — Run jruby_compare.rb (if jruby available, else MRI)
#   make decompile       — Run javap -c on all .class files
#   make clean           — Remove all build artifacts
#   make help            — Show this help message
# ==============================================================================

# Java tools
JAVAC   := javac
JAVA    := java
JAVAP   := javap

# Directories
JAVA_DIR := java
RUBY_DIR := ruby
BUILD    := build

# Source files
CLASSLOAD_SRC  := $(JAVA_DIR)/ClassLoadingDemo.java
JIT_SRC        := $(JAVA_DIR)/JITWarmup.java
BYTECODE_SRC   := $(JAVA_DIR)/BytecodeInspector.java
GC_SRC         := $(JAVA_DIR)/GCDemo.java
NATIVE_SRC     := $(JAVA_DIR)/NativeCompare.java
JRUBY_SRC      := $(RUBY_DIR)/jruby_compare.rb

# Class files (output of javac)
CLASSLOAD_CLS  := $(BUILD)/ClassLoadingDemo.class
JIT_CLS        := $(BUILD)/JITWarmup.class
BYTECODE_CLS   := $(BUILD)/BytecodeInspector.class
GC_CLS         := $(BUILD)/GCDemo.class
NATIVE_CLS     := $(BUILD)/NativeCompare.class

# ==============================================================================
# Phony targets
# ==============================================================================
.PHONY: all classloading jit bytecode gc native-compare jruby decompile \
        clean help

# ==============================================================================
# Build targets
# ==============================================================================

all: $(BUILD) $(CLASSLOAD_CLS) $(JIT_CLS) $(BYTECODE_CLS) $(GC_CLS) $(NATIVE_CLS)
	@echo ""
	@echo "All Java files compiled successfully to $(BUILD)/."
	@echo "Run 'make help' for all available targets."

$(BUILD):
	mkdir -p $(BUILD)

$(CLASSLOAD_CLS): $(CLASSLOAD_SRC) | $(BUILD)
	$(JAVAC) -d $(BUILD) $<

$(JIT_CLS): $(JIT_SRC) | $(BUILD)
	$(JAVAC) -d $(BUILD) $<

$(BYTECODE_CLS): $(BYTECODE_SRC) | $(BUILD)
	$(JAVAC) -d $(BUILD) $<

$(GC_CLS): $(GC_SRC) | $(BUILD)
	$(JAVAC) -d $(BUILD) $<

$(NATIVE_CLS): $(NATIVE_SRC) | $(BUILD)
	$(JAVAC) -d $(BUILD) $<

# ==============================================================================
# Demonstration targets
# ==============================================================================

# --------------------------------------------------------------------------
# Class Loading
# --------------------------------------------------------------------------
classloading: $(CLASSLOAD_CLS)
	@echo "======================================================================"
	@echo " CLASS LOADING DEMO"
	@echo "======================================================================"
	@echo ""
	@echo "--- Running ClassLoadingDemo ---"
	@echo "Observe: classes are loaded lazily, only on first use."
	@echo ""
	$(JAVA) -cp $(BUILD) ClassLoadingDemo
	@echo ""
	@echo "--- Run again with verbose class loading: ---"
	@echo "  $(JAVA) -verbose:class -cp $(BUILD) ClassLoadingDemo"
	@echo "  (You'll see hundreds of JDK classes loaded before main() starts.)"
	@echo ""
	@echo "--- Run with a command-line argument to trigger conditional loading: ---"
	@echo "  $(JAVA) -cp $(BUILD) ClassLoadingDemo somearg"

# --------------------------------------------------------------------------
# JIT Warmup
# --------------------------------------------------------------------------
jit: $(JIT_CLS)
	@echo "======================================================================"
	@echo " JIT WARMUP EFFECT"
	@echo "======================================================================"
	@echo ""
	@echo "--- Running JITWarmup with -XX:+PrintCompilation ---"
	@echo "Watch for tier transitions (3 = C1, 4 = C2, % = OSR)."
	@echo ""
	$(JAVA) -XX:+PrintCompilation -cp $(BUILD) JITWarmup 2>&1 | \
		grep -E 'JITWarmup|matmul|sumOfSquares|Iteration|Trial|Speedup|Phase|Warmup|===|---' || true
	@echo ""
	@echo "--- Full output (without filtering): ---"
	@echo "  $(JAVA) -XX:+PrintCompilation -cp $(BUILD) JITWarmup"
	@echo ""
	@echo "--- Disable JIT (pure interpretation): ---"
	@echo "  $(JAVA) -Xint -cp $(BUILD) JITWarmup"
	@echo "  (All iterations will be slow — no warmup improvement.)"

# --------------------------------------------------------------------------
# Bytecode Inspection
# --------------------------------------------------------------------------
bytecode: $(BYTECODE_CLS)
	@echo "======================================================================"
	@echo " BYTECODE INSPECTION"
	@echo "======================================================================"
	@echo ""
	@echo "--- Running BytecodeInspector ---"
	$(JAVA) -cp $(BUILD) BytecodeInspector
	@echo ""
	@echo "======================================================================"
	@echo " DISASSEMBLED BYTECODE (javap -c)"
	@echo "======================================================================"
	@echo ""
	$(JAVAP) -c -cp $(BUILD) BytecodeInspector
	@echo ""
	@echo "--- For full verbose output with constant pool: ---"
	@echo "  $(JAVAP) -c -verbose -cp $(BUILD) BytecodeInspector"

# --------------------------------------------------------------------------
# Garbage Collection
# --------------------------------------------------------------------------
gc: $(GC_CLS)
	@echo "======================================================================"
	@echo " GARBAGE COLLECTION DEMO"
	@echo "======================================================================"
	@echo ""
	@echo "--- Running GCDemo with -verbose:gc ---"
	@echo "(GC events will be interleaved with program output.)"
	@echo ""
	$(JAVA) -verbose:gc -Xms32m -Xmx128m -cp $(BUILD) GCDemo
	@echo ""
	@echo "--- For more detailed GC logging (Java 9+): ---"
	@echo "  $(JAVA) -Xlog:gc*=info -Xms32m -Xmx128m -cp $(BUILD) GCDemo"
	@echo ""
	@echo "--- Try different collectors: ---"
	@echo "  $(JAVA) -XX:+UseSerialGC -verbose:gc -cp $(BUILD) GCDemo"
	@echo "  $(JAVA) -XX:+UseG1GC -verbose:gc -cp $(BUILD) GCDemo"

# --------------------------------------------------------------------------
# Native Comparison
# --------------------------------------------------------------------------
native-compare: $(NATIVE_CLS)
	@echo "======================================================================"
	@echo " JVM vs NATIVE EXECUTION"
	@echo "======================================================================"
	@echo ""
	$(JAVA) -cp $(BUILD) NativeCompare
	@echo ""
	@echo "--- Compare with Module 7's native versions: ---"
	@echo "  cd ../07-rust-llvm-deep-dive && make zero-cost"
	@echo "  cd ../07-rust-llvm-deep-dive && make compare"

# --------------------------------------------------------------------------
# JRuby / MRI Ruby comparison
# --------------------------------------------------------------------------
jruby:
	@echo "======================================================================"
	@echo " RUBY IMPLEMENTATION COMPARISON"
	@echo "======================================================================"
	@echo ""
	@if command -v jruby >/dev/null 2>&1; then \
		echo "--- JRuby found! Running with JRuby (JVM-based): ---"; \
		echo ""; \
		jruby $(JRUBY_SRC); \
		echo ""; \
		echo "--- Now running with MRI for comparison: ---"; \
		echo ""; \
		if command -v ruby >/dev/null 2>&1; then \
			ruby $(JRUBY_SRC); \
		else \
			echo "  (MRI Ruby not found. Install with: apt install ruby)"; \
		fi; \
	elif command -v ruby >/dev/null 2>&1; then \
		echo "--- JRuby not found. Running with MRI Ruby: ---"; \
		echo "  (Install JRuby with: apt install jruby or download from jruby.org)"; \
		echo ""; \
		ruby $(JRUBY_SRC); \
	else \
		echo "  Neither Ruby nor JRuby is installed."; \
		echo "  Install Ruby:  apt install ruby"; \
		echo "  Install JRuby: apt install jruby (or download from jruby.org)"; \
	fi

# --------------------------------------------------------------------------
# Decompile all class files
# --------------------------------------------------------------------------
decompile: all
	@echo "======================================================================"
	@echo " BYTECODE DECOMPILATION (javap -c on all .class files)"
	@echo "======================================================================"
	@echo ""
	@for cls in $(BUILD)/*.class; do \
		name=$$(basename "$$cls" .class); \
		echo "----------------------------------------------------------------------"; \
		echo " $$name"; \
		echo "----------------------------------------------------------------------"; \
		echo ""; \
		$(JAVAP) -c -cp $(BUILD) "$$name" 2>/dev/null || echo "  (Could not decompile $$name — may be an inner class)"; \
		echo ""; \
	done
	@echo ""
	@echo "For verbose output with constant pool:"
	@echo "  $(JAVAP) -c -verbose -cp $(BUILD) <ClassName>"

# ==============================================================================
# Cleanup
# ==============================================================================

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD)
	@echo "Clean complete."

# ==============================================================================
# Help
# ==============================================================================

help:
	@echo "Module 8: JVM Essentials — Available Targets"
	@echo ""
	@echo "  Individual demos:"
	@echo "    make classloading    Lazy class loading and classloader hierarchy"
	@echo "    make jit             JIT warmup: interpreted -> C1 -> C2 compilation"
	@echo "    make bytecode        Bytecode inspection with javap -c"
	@echo "    make gc              Garbage collection with -verbose:gc"
	@echo "    make native-compare  JVM vs native execution timing"
	@echo "    make jruby           Ruby on JVM (JRuby) vs MRI comparison"
	@echo ""
	@echo "  Analysis:"
	@echo "    make decompile       javap -c on all compiled .class files"
	@echo ""
	@echo "  Build/Clean:"
	@echo "    make all             Compile all Java files"
	@echo "    make clean           Remove all build artifacts"
	@echo "    make help            Show this help message"
