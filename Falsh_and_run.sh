#!/usr/bin/env bash
set -e

# ------------------------------------------------------------
# flash_and_run.sh
# Ternary-OS Kernel bring-up script
# ------------------------------------------------------------
# This script:
# 1. Builds the Microkit image
# 2. Launches it under QEMU (or flashes to hardware if configured)
# ------------------------------------------------------------

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
SYSTEM_IMAGE="$BUILD_DIR/ternary_image.img"

# ------------------------------------------------------------
# Configuration
# ------------------------------------------------------------

# Default: RISC-V QEMU
ARCH="riscv64"
QEMU_BIN="qemu-system-riscv64"

# Machine + CPU chosen to match seL4 Microkit examples
QEMU_MACHINE="virt"
QEMU_CPU="rv64"
QEMU_MEM="1024M"

# Serial console
QEMU_SERIAL="-nographic -serial mon:stdio"

# ------------------------------------------------------------
# Sanity checks
# ------------------------------------------------------------

if [ -z "$MICROKIT_SDK" ]; then
    echo "[!] MICROKIT_SDK not set"
    echo "    export MICROKIT_SDK=/path/to/microkit-sdk"
    exit 1
fi

if ! command -v "$QEMU_BIN" >/dev/null 2>&1; then
    echo "[!] $QEMU_BIN not found"
    echo "    Install qemu-system-riscv64"
    exit 1
fi

# ------------------------------------------------------------
# Build
# ------------------------------------------------------------

echo "[*] Building Ternary Kernel image..."

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. 
make -j$(nproc)

if [ ! -f "$SYSTEM_IMAGE" ]; then
    echo "[!] Image not produced: $SYSTEM_IMAGE"
    exit 1
fi

echo "[✓] Build complete"

# ------------------------------------------------------------
# Run (QEMU)
# ------------------------------------------------------------

echo "[*] Launching under QEMU..."
echo "    Bay-0 heartbeat, rails, airlocks should appear on console"

"$QEMU_BIN" \
    -machine "$QEMU_MACHINE" \
    -cpu "$QEMU_CPU" \
    -m "$QEMU_MEM" \
    -kernel "$SYSTEM_IMAGE" \
    $QEMU_SERIAL

# ------------------------------------------------------------
# Notes
# ------------------------------------------------------------
# Expected output order:
#  - Bay-0 online
#  - Rail2 (Interface) alive
#  - Rail3 (Reasoning) starts
#  - 500ms heartbeat crowbars Rail3
#  - Rail2 never flickers
# ------------------------------------------------------------
