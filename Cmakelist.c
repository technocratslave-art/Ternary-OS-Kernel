cmake_minimum_required(VERSION 3.16)
project(TernaryKernel C)

# ------------------------------------------------------------
# Microkit SDK
# ------------------------------------------------------------
if(NOT DEFINED ENV{MICROKIT_SDK})
    message(FATAL_ERROR "MICROKIT_SDK environment variable not set")
endif()

set(MICROKIT_SDK $ENV{MICROKIT_SDK})
include(${MICROKIT_SDK}/microkit.cmake)

microkit_init()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-ffreestanding -fno-builtin -fno-stack-protector -fno-pic")

include_directories(
    ${CMAKE_SOURCE_DIR}/../include
)

# ------------------------------------------------------------
# Protection Domains (PDs)
# ------------------------------------------------------------

# Bay-0 / Monitor / Heartbeat
add_executable(bay0.elf
    ../Bay0.c
)
microkit_link(bay0.elf)

# Rail 1 – Sensors (untrusted)
add_executable(rail1.elf
    ../Rail1.c
)
microkit_link(rail1.elf)

# Rail 2 – Interface / Display (persistent)
add_executable(rail2.elf
    ../Rail2.c
)
microkit_link(rail2.elf)

# Rail 3 – Reasoning / AI (ephemeral)
add_executable(rail3.elf
    ../Rail3.c
)
microkit_link(rail3.elf)

# ------------------------------------------------------------
# System Image
# ------------------------------------------------------------
microkit_system(
    ternary_image
    SYSTEM ${CMAKE_SOURCE_DIR}/ternary.system
    ELF
        bay0.elf
        rail1.elf
        rail2.elf
        rail3.elf
)

# ------------------------------------------------------------
# Convenience target
# ------------------------------------------------------------
add_custom_target(image ALL
    DEPENDS ternary_image
)
