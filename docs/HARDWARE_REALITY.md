Ternary-OS: Engineering Reality (Discrete 3-CPU Implementation)

This document describes a buildable, non-theoretical implementation of the Ternary-OS architecture using off-the-shelf hardware.
No ternary silicon is required. No speculative physics. Just deterministic power domains, physical gating, and boring embedded engineering.

The goal is simple:

> Replace software trust with physical isolation.




1. What This Is (and Is Not)

This is:

A discrete hardware realization of the Ternary-OS model

Built with three physically isolated compute domains

Enforced by power gating, write-enable gating, and fixed wiring

Capable of hard-resetting untrusted compute on a fixed cadence

Designed to be prototyped today with commodity parts


This is not:

A claim that Linux reboots every 500ms

A claim that memory magically deletes itself without power loss

A claim that current NPUs run full LLMs without constraints

A marketing diagram pretending wires don’t matter


Every security guarantee in this design corresponds to a real electrical constraint.



2. The Core Idea: Physical Rails, Not Software Domains

The system is divided into three voltage islands (rails).
If electrons cannot flow, compromise cannot propagate.

Rail Overview

Rail	Name	Purpose	Trust Level

Rail A	Sensor Rail	Raw input capture	Untrusted
Rail B	Compute Rail	AI / inference / logic	Expendable
Rail C	Vault Rail	Control, keys, timing	Sovereign


Only Rail C is trusted.
All other rails are treated as hostile by default.



3. Rail A: Sensor Rail (Ingress Only)

Example hardware: STM32H7 (Cortex-M7)

Responsibilities:

Directly wired to camera, mic, IMU, GPS

Performs minimal preprocessing

Writes raw or reduced data into the Airlock mailbox


Hard constraints:

No physical connection to compute rail

No access to keys

Cannot read outputs

Cannot assert write-enable on the Red side of the Airlock


If compromised, the attacker gets only sensor data and nothing else.



4. Rail B: Expendable Compute Rail (Reasoning)

Example hardware options:

FPGA + NPU appliance

Fast-boot MCU + accelerator

Linux SoC with resettable AI runtime domain (not full power cycling)


Responsibilities:

Runs inference, reasoning, token generation

Reads inputs only through the Airlock

Writes outputs only through the Airlock


Critical clarification:

> The AI runtime is reset on the hinge cadence — not necessarily the entire SoC.



In v0 builds, the “death” event applies to:

NPU context

KV cache

model state

scratch memory

DMA windows


This avoids pretending Linux can cold-boot every 500ms while preserving the security invariant:
no long-lived hidden state survives the hinge.



5. Rail C: Vault / Blue Hub (Always-On)

Example hardware: STM32L4 / STM32U5

This rail never loses power.

Responsibilities:

Generates rolling cryptographic keys

Owns the hinge timer

Controls power gating of Rail B

Controls write-enable lines on the Airlock

Clears and seals shared memory

Enforces timing discipline


If this rail is compromised, the system is compromised — and that is explicitly accepted and modeled.


6. The Airlock: Dual-Port SRAM Mailbox (The Gate)

Hardware: Dual-port SRAM
Not software. Not shared RAM. Physical silicon.

Properties

Port A wired to Sensor Rail

Port B wired to Compute Rail

Write-enable on each side is independently controlled

Vault Rail physically gates the Red side write-enable


> Even full code execution on the Compute Rail cannot write unless the Vault allows it.


This turns “policy” into wiring.



7. The Hinge Cycle (What Actually Resets)

The hinge is not mystical. It is a sequenced electrical event.

Corrected Hinge Lifecycle

Phase	Duration	Vault Action	Compute Rail

Compute	~490ms	Monitoring only	Active
Seal	~2ms	Disable write-enable	Frozen
Dissolve	~8ms	Reset / power-gate AI domain	Dead
Rebirth	~10ms	Re-enable, inject scar	Clean start


Only the expendable compute state dies.
The Vault and Media rails remain stable.



8. Semantic Scar: The Only Thing That Survives

Before each hinge:

Vault hashes a compressed summary of system intent

Size is strictly bounded (e.g. 64 bytes)

Authenticated with a rolling key

No timestamps

No sentiment

No identity metadata


After reset:

Scar is re-injected as context-free facts

“Previous topic: Ternary rails”

Not “user was tired / angry / emotional”


This creates continuity without memory accumulation.



9. Continuity Without Flicker: Isolated Buffer Ring

Hard resets cause stutter unless output is decoupled.

Solution:
A dual-port buffer ring that lives outside the reset domain.

Compute rail overproduces data during active phase

DMA / display controller drains at fixed rate (e.g. 60Hz)

Read pointer never resets

Write pointer is synchronized using the Semantic Scar


The screen and audio never flicker — even while the AI is being reset.



10. Cost and Build Reality

Bench Prototype Cost (Honest)

$500–$900 depending on PCB spins and assembly

All parts are commodity

No custom silicon

No exotic fabrication


Timeline (Realistic)

Stage	Time

Dev-board proof	1–3 weeks
Custom PCB design	2–4 weeks
Fabrication + assembly	2–4 weeks
Firmware + integration	4–6 weeks


This is normal embedded work, not a moonshot.



11. Why This Is Legitimate Engineering

Red/Black isolation is standard in defense systems

Dual-port SRAM mailboxes are industrial practice

Power gating is the only reliable memory erasure

Reset-bounded compute eliminates persistence attacks

All guarantees map to physical constraints


No claim here depends on:

perfect software

future chips

unverifiable assumptions



12. Summary

This project does not ask hardware to behave philosophically.
It asks hardware to behave electrically.

The security model holds because:

wires don’t lie

power loss erases state

write-enable is not negotiable

trust is centralized and minimal


If someone wants to challenge this design, the correct response is not debate — it’s a schematic review.

