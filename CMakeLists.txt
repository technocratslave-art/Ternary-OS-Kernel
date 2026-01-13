cmake_minimum_required(VERSION 3.16)

# ============================================================
# Ternary-OS Kernel (Enforcement Spine)
# Root CMakeLists.txt
# ============================================================
# This is NOT an application build.
# This builds ONLY the kernel enforcement plane:
#   - Bay-0 (monitor / heartbeat / crowbar)
#   - Rails (PDs)
#   - System image via seL4 Microkit
# ============================================================

project(TernaryOSKernel NONE)

# ------------------------------------------------------------
# Guardrails
# ------------------------------------------------------------
if(NOT DEFINED ENV{MICROKIT_SDK})
    message(FATAL_ERROR
        "MICROKIT_SDK environment variable not set.\n"
        "Export it to your Microkit SDK path before building."
    )
endif()

# ------------------------------------------------------------
# Global build settings
# ------------------------------------------------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Force out-of-tree builds
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR
        "In-tree builds are not supported.\n"
        "Create a build/ directory and run cmake from there."
    )
endif()

# ------------------------------------------------------------
# Architecture selection
# ------------------------------------------------------------
# Default target: RISC-V (QEMU / dev boards)
set(TERNARY_ARCH "riscv64" CACHE STRING "Target architecture")

message(STATUS "Building Ternary-OS Kernel for ${TERNARY_ARCH}")

# ------------------------------------------------------------
# Subprojects
# ------------------------------------------------------------
# All kernel logic lives under sel4/
# This keeps the repository kernel-first and clean.

add_subdirectory(sel4)

# ------------------------------------------------------------
# Developer convenience target
# ------------------------------------------------------------
add_custom_target(kernel
    DEPENDS image
    COMMENT "Built full Ternary-OS Microkit image"
)

# ------------------------------------------------------------
# Status banner
# ------------------------------------------------------------
message(STATUS "---------------------------------------------")
message(STATUS " Ternary-OS Kernel")
message(STATUS "  • Enforcement spine only")
message(STATUS "  • Bay-0 + Rails + Airlocks")
message(STATUS "  • No apps, no AI, no drivers")
message(STATUS "---------------------------------------------")

This root CMakeLists.txt does exactly what you asked:

• Kernel-first
• No app logic
• Delegates everything to sel4/
• Enforces out-of-tree builds
• Produces one image via Microkit
