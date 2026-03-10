# =============================================================================
# Top-Level Makefile — Binary Roundtrip Syllabus
# =============================================================================
# Usage:
#   make check       — verify all tools are installed
#   make setup       — install missing tools
#   make module-01   — build Module 1 exercises
#   make module-02   — build Module 2 exercises
#   ...
#   make all         — build all modules
#   make clean       — clean all build artifacts
# =============================================================================

MODULES := 01 02 03 04 05 06 07 08 09 10
MODULE_DIRS := $(addprefix modules/,$(shell ls modules/ 2>/dev/null | sort))

.PHONY: all check setup clean help $(addprefix module-,$(MODULES))

help:
	@echo "============================================="
	@echo " Binary Roundtrip — Course Makefile"
	@echo "============================================="
	@echo ""
	@echo "  make check       — verify all tools are installed"
	@echo "  make setup       — install missing tools"
	@echo "  make all         — build all module exercises"
	@echo "  make module-NN   — build a specific module (01-10)"
	@echo "  make clean       — remove all build artifacts"
	@echo ""
	@echo "Modules:"
	@for d in $(MODULE_DIRS); do echo "  $$d"; done

check:
	@bash setup.sh --check-only

setup:
	@bash setup.sh

all: $(addprefix module-,$(MODULES))

module-%:
	@dir=$$(ls -d modules/$*-* 2>/dev/null | head -1); \
	if [ -z "$$dir" ]; then \
		echo "Module $* not found"; exit 1; \
	fi; \
	echo "========== Building $$dir =========="; \
	$(MAKE) -C "$$dir" all

clean:
	@for d in $(MODULE_DIRS); do \
		if [ -f "$$d/Makefile" ]; then \
			echo "Cleaning $$d..."; \
			$(MAKE) -C "$$d" clean; \
		fi; \
	done
	@echo "All clean."
