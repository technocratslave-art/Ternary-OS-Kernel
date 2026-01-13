BUILD GUIDE — Ternary-OS-Kernel (Microkit Spine)

Goal: build a bootable Microkit system image that proves:

Protection Domain isolation (rails)

State-0 Airlock (no_std Rust)

Bay-0 crowbar + hinge cycles (periodic restart of Rail3)

Rail2 stays alive while Rail3 is killed/restarted

Scar buffer continuity artifact


This is not a phone OS. This is the enforcement plane.

1. Prerequisites



You need:

seL4 Microkit SDK (matching your repo’s Microkit version)

CMake + Ninja (or Make)

Clang or GCC cross toolchain for your target

Rust toolchain (for Airlock, no_std)


Environment variables:

MICROKIT_SDK = path to Microkit SDK root


Example: export MICROKIT_SDK=~/microkit-sdk

2. Expected Repo Layout



Minimum required files for this build: sel4/ ternary.system CMakeLists.txt

include/ ternary_contract.h ternary_weights.h

components/ bay0/bay0.c rail1_sensors/rail1.c rail2_interface/rail2.c rail3_reasoning/rail3.c airlock/Cargo.toml airlock/rust-toolchain.toml airlock/src/lib.rs airlock/src/sha256.rs

If your names differ, update:

program_image paths in sel4/ternary.system

target names in sel4/CMakeLists.txt


3. Build Order (Important)



Build order matters because Microkit wants ELF binaries for each PD.

Order: A) Build C PDs (bay0, rail1, rail2, rail3) B) Build Rust PD (airlock) into airlock.elf C) Run Microkit image build using ternary.system

4. Build Step A — C Protection Domains



From repo root:

mkdir -p build cd build cmake -G Ninja ../sel4 -DMICROKIT_SDK="$MICROKIT_SDK" ninja

Expected outputs (names may vary): build/bay0.elf build/rail1.elf build/rail2.elf build/rail3.elf

If your CMakeLists emits differently, adjust program_image path=... in ternary.system.

5. Build Step B — Airlock (Rust no_std)



Airlock is a staticlib crate that needs to become an ELF PD image.

There are two ways to do this.

Option 1 (recommended): build airlock as a Microkit component using the SDK’s Rust support if your Microkit toolchain includes it.

Option 2 (generic): compile Rust to a static library, then link it into an ELF with Microkit’s linker script.

Below is the generic, predictable method.

B1) Install Rust target rustup target add riscv64imac-unknown-none-elf

B2) Build staticlib cd components/airlock cargo build --release

This produces: components/airlock/target/riscv64imac-unknown-none-elf/release/libairlock.a

B3) Link to airlock.elf using Microkit toolchain You need a tiny C shim (or linker entry) if your Microkit expects specific symbols. If your lib.rs exports init() and notified(), that’s typically enough.

From repo root (example; adjust toolchain names to your SDK):

$MICROKIT_SDK/bin/riscv64-unknown-elf-gcc 
-nostdlib 
-Wl,--gc-sections 
-Wl,-T,$MICROKIT_SDK/board/<YOUR_BOARD>/microkit.ld 
-o build/airlock.elf 
components/airlock/target/riscv64imac-unknown-none-elf/release/libairlock.a 
-lc -lgcc

Important: the exact linker script path depends on your Microkit SDK board/platform. Replace <YOUR_BOARD> accordingly.

If your SDK already provides a helper like microkit_link(...), use that instead of manual linking.

6. Build Step C — Build the Microkit System Image



Microkit builds the system image from the .system file plus PD ELFs.

Your sel4/ternary.system must reference the correct ELF paths.

Example expected: <program_image path="bay0.elf" /> <program_image path="airlock.elf" /> <program_image path="rail1.elf" /> <program_image path="rail2.elf" /> <program_image path="rail3.elf" />

If your build outputs are in build/, either:

copy ELFs into the directory where Microkit expects them, or

change program_image path= to include relative paths.


Then run the Microkit image build command. This differs depending on SDK.

Common patterns:

microkit build sel4/ternary.system

or a CMake target like ninja image


If your SDK uses CMake integration, you’ll likely get something like: ninja ternary-image

Output typically includes:

a bootable image (often loader.img / image.elf / system.img)

or QEMU runnable artifact


7. Running in QEMU



This depends on your Microkit platform target (riscv/arm/x86). Example shape:

qemu-system-riscv64 
-machine virt 
-m 1024 
-nographic 
-kernel build/system.elf

Or Microkit may output a loader.elf which is what QEMU boots.

Your SDK should document:

exact QEMU machine type

required devices

correct “kernel” file to boot


8. What “Success” Looks Like



On boot you should see logs like:

bay0: online airlock: online rail2: interface online (persistent) rail3: online (untrusted)

Then every heartbeat (if timer wired): bay0: heartbeat bay0: HINGE -> restart rail3 rail3: restarted by crowbar

On a violation (airlock detects “NEEDLE:” or matches needles_ro): airlock: violation -> bay0 crowbar bay0: VIOLATION -> crowbar rail3

Rail2 should keep printing / remain alive while rail3 is repeatedly restarted.

9. Common Failure Modes



A) “system won’t build” / missing ELF

Your ternary.system program_image paths don’t match actual output names/locations.


B) Airlock links but crashes immediately

ABI mismatch: Microkit expects init() / notified() symbols exactly.

Wrong linker script for target board.

Missing #![no_std] or panic strategy not aborting.


C) No heartbeat hinge events

Your platform IRQ number is wrong.

Timer not programmed in bay0.c.

For early bring-up, temporarily trigger hinge via a channel to prove restart semantics first.


D) Rail2 dies when Rail3 dies

You accidentally restarted all PDs or used wrong PD name in microkit_pd_restart.

Fix: only restart "rail3" in Bay-0.


10. Minimal “Bring-up First” Strategy



Do this in order:

1. Boot without timer/IRQ. Prove PDs run.


2. Trigger crowbar via airlock violation to prove restart works.


3. Wire timer IRQ for the hinge cycle.


4. Only then add real device mappings (display, sensor buses, storage).


5. Notes on “Weights”



ternary_weights.h defines the contract, not a model. In this repo:

“weights” are treated like immutable firmware blobs.

Rail3 can be restarted any time; loader must be restartable.

Airlock/Bay-0 can verify a root hash and chunk chain without trusting Rail3.


Real hardware mapping (NPU regs, UFS controller, SRAM windows) belongs in ternary.system as memory_region + map entries once you target a specific board.
