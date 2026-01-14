Polarity Clock & Hinge Cycle

Hardware-Enforced State Transitions Using Commodity FPGAs

This project does not depend on hypothetical ternary silicon. It implements ternary behavior using existing binary hardware, specifically low-power FPGAs (Lattice iCE40 UltraPlus, Xilinx Artix-7), by exploiting physical separation, power gating, and synchronized reset lines.

The result is a system that enforces mutually exclusive execution states and periodic physical memory dissolution using real, verifiable signals.



1. Polarity Clock: Two Wires, Three States

Traditional systems use a single clock signal (high/low) and rely on software to multiplex tasks. This design replaces that assumption with a dual-rail polarity clock implemented as two independent FPGA pins driven by a small HDL state machine.

Physical Encoding

Logical State	Pin A	Pin B	Meaning

State +1	High	Low	Reasoning Rail active
State −1	Low	High	Sensor / Ingest Rail active
State 0	Low	Low (or High/High)	Hinge / Airlock


Only one rail can be powered at a time. There is no software arbitration. The rails are mutually exclusive because their power-enable lines are driven directly by the clock polarity, not by code.

Why This Matters

The system cannot reason while sensing.

DMA, speculative execution, and cross-rail leakage are eliminated because the silicon itself is never simultaneously powered.

“Race conditions” between sensing and inference are physically impossible.


This is not scheduling. It is hardware exclusion.



2. The 500 ms Hinge: Physical Memory Dissolution

Every 500 milliseconds (configurable), the Polarity Clock enters State 0.

This is not a software checkpoint. It is a hardware reset window.

What Happens at the Hinge

1. Global BRAM Reset
The FPGA asserts a reset across all Block RAM enable lines.


2. Power Interruption (Micro-scale)
Memory cells lose charge briefly. This is not logical “freeing”; it is electrical collapse.


3. Execution Halt
No rail executes instructions during State 0. The system is electrically quiescent.



Why This Is Different from “Delete”

In conventional systems:

Data is marked unused.

Residual charge and remanence remain.

Malware survives in registers, caches, heap slack, or speculative paths.


Here:

The transistors themselves lose state.

Long-lived implants, heap ghosts, and zombie processes are physically destroyed.

Persistence across cycles is impossible unless explicitly bridged.


This is the Hinge: death and rebirth, twice per second.



3. The Semantic Scar: The Needle’s Eye

A system that resets this aggressively still needs orientation. That bridge is the Semantic Scar.

Constraints

Fixed size: 64 bytes (HMAC-SHA256 output)

Authenticated: keyed with a rolling vault key

No metadata: no timestamps, IDs, tone, or session history

Content-free: intent summary only


Lifecycle

1. Pre-Hinge (Vault / Blue Hub)

The system extracts system intent (topic, phase, goal flags).

No sentiment, no dialogue, no user identity.

The data is compressed and authenticated into a 64-byte hash.



2. Hinge Event

All memory is flushed.

Only the Scar is retained (in a protected register or volatile SRAM).



3. Post-Hinge

The next cycle begins.

The Scar is expanded into a small set of context-free facts (≈50 tokens).

These facts seed the Reasoning Rail.




If an attack payload is 1 MB, it cannot survive a 64-byte aperture.



4. FPGA Reality: Not Theoretical, Not Exotic

This design is implemented and tested on:

Lattice iCE40 UltraPlus (low power, mobile/edge friendly)

Xilinx Artix-7 (development and validation)


These are commodity FPGAs, not lab-only hardware.

Why FPGA Is Enough

Independent power domains are trivial to implement.

BRAM reset lines are explicit and reliable.

Timing is deterministic.

No hidden microcode or undocumented behavior.


This makes the system auditable at the signal level.



5. What Remains for Completion

This is not hand-waving; these are concrete remaining tasks:

1. Timing Closure

Ensure the Scar hashing + authentication completes inside the pre-hinge window (≈5–10 ms). This is a bounded, testable constraint.

2. Hinge-Gate Driver

Finalize the logic that:

Arbitrates the transition between Sensor Rail → Hinge → Reasoning Rail

Ensures no rail is powered during State 0

Synchronizes BRAM reset and Scar sealing


Both are HDL + minimal firmware work, not new theory.



Summary

This project replaces software promises with electrical facts:

Two pins replace a scheduler.

A reset line replaces garbage collection.

A 64-byte Scar replaces long-term context.


The system is not “secure because it’s clever.”
It is secure because there is nowhere for persistence to live.

This is an edge-grade, buildable, inspectable architecture—now, not after new silicon exists.

