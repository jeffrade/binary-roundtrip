#!/usr/bin/env bash
# =============================================================================
# setup.sh — Install all tools needed for the "Binary Roundtrip" course
# =============================================================================
# Usage: bash setup.sh [--check-only]
# --check-only: just report what's installed, don't install anything
# =============================================================================
set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

CHECK_ONLY=false
[[ "${1:-}" == "--check-only" ]] && CHECK_ONLY=true

MISSING=()

check_cmd() {
    local cmd="$1"
    local pkg="${2:-$1}"
    local desc="${3:-}"
    if command -v "$cmd" &>/dev/null; then
        local ver
        ver=$("$cmd" --version 2>&1 | head -1 || echo "installed")
        printf '%b[OK]%b  %-20s %s\n' "$GREEN" "$NC" "$cmd" "$ver"
    else
        printf '%b[MISS]%b %-20s %s\n' "$RED" "$NC" "$cmd" "${desc:-(package: $pkg)}"
        MISSING+=("$pkg")
    fi
}

echo "============================================="
echo " Syllabus Tool Check: Binary Roundtrip"
echo "============================================="
echo ""

echo "--- Core C/C++ Toolchain ---"
check_cmd gcc gcc "GNU C Compiler"
check_cmd g++ g++ "GNU C++ Compiler"
check_cmd make make "GNU Make"
check_cmd ld binutils "GNU Linker"
check_cmd as binutils "GNU Assembler"
check_cmd cpp cpp "C Preprocessor"

echo ""
echo "--- Binary Analysis Tools ---"
check_cmd readelf binutils "ELF reader"
check_cmd objdump binutils "Object file disassembler"
check_cmd nm binutils "Symbol table lister"
check_cmd ldd libc-bin "Shared library dependency lister"
check_cmd strings binutils "String extractor"
check_cmd file file "File type identifier"
check_cmd strace strace "System call tracer"
check_cmd ltrace ltrace "Library call tracer"
check_cmd hexdump bsdmainutils "Hex dumper"

echo ""
echo "--- Debugger ---"
check_cmd gdb gdb "GNU Debugger"

echo ""
echo "--- Java (JDK) ---"
check_cmd java default-jdk "Java Runtime"
check_cmd javac default-jdk "Java Compiler"
check_cmd javap default-jdk "Java Class File Disassembler"

echo ""
echo "--- Ruby ---"
check_cmd ruby ruby "Ruby Interpreter (MRI)"
check_cmd irb ruby "Interactive Ruby"
check_cmd gem ruby "RubyGems"
check_cmd bundle ruby-bundler "Bundler"

echo ""
echo "--- Rust ---"
check_cmd rustc rust "Rust Compiler"
check_cmd cargo rust "Rust Package Manager"
check_cmd rustup rustup "Rust Toolchain Manager"

echo ""
echo "--- Optional / Advanced ---"
check_cmd ghidra ghidra "Ghidra Decompiler (GUI)"
check_cmd r2 radare2 "Radare2 Disassembler"
check_cmd shellcheck shellcheck "Shell script linter"
check_cmd nasm nasm "Netwide Assembler"

echo ""
echo "============================================="

if [[ ${#MISSING[@]} -eq 0 ]]; then
    printf '%b%s%b\n' "$GREEN" "All tools are installed!" "$NC"
    exit 0
fi

printf '%bMissing %d tool(s).%b\n' "$YELLOW" "${#MISSING[@]}" "$NC"
echo ""

if $CHECK_ONLY; then
    echo "Run without --check-only to install missing packages."
    exit 1
fi

# Deduplicate
mapfile -t UNIQUE_PKGS < <(printf '%s\n' "${MISSING[@]}" | sort -u)

echo "The following packages will be installed via apt:"
printf "  %s\n" "${UNIQUE_PKGS[@]}"
echo ""

# Filter out non-apt packages
APT_PKGS=()
NON_APT=()
for pkg in "${UNIQUE_PKGS[@]}"; do
    case "$pkg" in
        rust|rustup)
            NON_APT+=("$pkg")
            ;;
        ghidra)
            NON_APT+=("$pkg")
            ;;
        *)
            APT_PKGS+=("$pkg")
            ;;
    esac
done

if [[ ${#APT_PKGS[@]} -gt 0 ]]; then
    echo "--- Installing via apt ---"
    sudo apt-get update -qq
    sudo apt-get install -y "${APT_PKGS[@]}"
fi

for pkg in "${NON_APT[@]}"; do
    case "$pkg" in
        rust|rustup)
            echo ""
            echo "--- Installing Rust via rustup ---"
            if ! command -v rustup &>/dev/null; then
                curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
                # shellcheck source=/dev/null
                source "$HOME/.cargo/env"
            fi
            ;;
        ghidra)
            echo ""
            printf '%b[MANUAL]%b Ghidra must be installed manually:\n' "$YELLOW" "$NC"
            echo "  1. Download from: https://github.com/NationalSecurityAgency/ghidra/releases"
            echo "  2. Extract and add to PATH"
            echo "  3. Requires JDK 17+"
            ;;
    esac
done

echo ""
echo "============================================="
echo " Setup complete. Re-run with --check-only to verify."
echo "============================================="
