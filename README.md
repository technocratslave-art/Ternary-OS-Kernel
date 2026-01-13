Ternary Kernel

┌──────────────────────────────────────────────────────────┐
│                    TERNARY KERNEL                         │
│        Minimal Microkernel with Structural Isolation      │
│                                                          │
│   ┌──────────────┐      ┌──────────────┐                │
│   │  State +1    │ ───▶ │   State 0    │ ───▶ State -1  │
│   │  Untrusted   │      │   Airlock    │      Trusted   │
│   │              │ ◀─── │ (No Execute) │ ◀───           │
│   └──────────────┘      └──────────────┘                │
│        ▲   │                     │                      │
│        │   └───── reset / purge ─┘                      │
│        │                                                 │
│   Compromise allowed                          Compromise impossible
│                                                          │
└──────────────────────────────────────────────────────────┘


Overview

The Ternary Kernel is a minimal, state-enforced microkernel designed to make post-exploit persistence structurally impossible.

Unlike monolithic kernels that rely on privilege levels, patching, and software trust assumptions, the Ternary Kernel enforces security through explicit execution states that are physically and logically isolated. Compromise is assumed, not avoided—and the system is built so that compromise cannot spread or persist.

This repository contains only the kernel.

No applications.
No AI.
No drivers.
No policy engines.

Those live above the kernel as plugins or external systems.


The Ternary Execution Model

The kernel defines three execution states, each with strict invariants:

State +1 — Untrusted

Runs all complex and failure-prone code

Applications, drivers, networking, AI, UI

Expected to be compromised

Has no authority over hardware or identity


State 0 — Neutral (Airlock)

Non-executing shared memory window

Bounded, fixed-size message exchange only

No code execution, no persistence

Prevents data smuggling and implicit trust


State −1 — Trusted

Minimal sealed system state

Holds identity, keys, invariants

No exposure to untrusted memory

Cannot be influenced by State +1


There is no escalation path between states.


What This Kernel Does

The Ternary Kernel is intentionally small and boring.

It is responsible for:

Enforcing state boundaries

Managing memory isolation

Routing permitted messages

Performing execution discontinuities

Resetting compromised domains


It does not:

Schedule applications

Implement filesystems

Run networking stacks

Execute AI models

Enforce user policy

Provide drivers


Those are explicitly excluded by design.


Threat Model

The kernel assumes:

Memory corruption will occur

Drivers will be exploited

Input is hostile

Userland is unreliable

Long-running compromise is likely


The kernel guarantees:

Exploits cannot cross state boundaries

Secrets cannot be reached from compromised code

Compromise cannot persist across resets

Trusted state cannot be influenced indirectly


This is post-exploit containment, not prevention.


Execution Discontinuity

All transitions between states trigger a hard execution break:

General-purpose registers cleared

Vector / SIMD registers cleared

Speculative execution flushed

Stack relocated

No return paths preserved


After a transition, execution cannot infer prior state.

This prevents:

Register leakage

Speculation attacks

Context smuggling

Temporal privilege escalation


Why Ternary (Not Binary)

Binary security models fail because they assume a safe boundary between “user” and “kernel.”

The neutral State 0 exists to:

Eliminate implicit trust

Eliminate shared execution context

Eliminate policy creep

Make transitions explicit and auditable


It is:

Small

Passive

Non-executable

Deterministic


What This Is Not

This kernel is not:

A general-purpose OS

An Android replacement

A Linux distribution

An AI runtime

A policy engine

A sandbox framework


Those systems may run on top of the Ternary Kernel, but they are not part of it.


Intended Use

The Ternary Kernel is designed as a foundation layer for:

Secure mobile devices

Embedded systems

Repairable / modular hardware

Sovereign computing platforms

Long-lived devices where trust matters


It is especially suited for environments where:

Updates are infrequent

Physical compromise is possible

Cloud trust is unacceptable

Identity must survive compromise


Design Goal

> Assume compromise.
Contain failure.
Eliminate persistence.



This kernel does not promise perfect security.
It promises bounded damage.


Repository Scope

This repository contains:

Core kernel source

State transition logic

Memory isolation primitives

Architecture documentation


It deliberately excludes:

Drivers

Applications

AI systems

User interfaces

Hardware-specific integrations


Those belong in separate repositories as plugins.

Status

Early architecture implementation.
Correctness and isolation take priority over features.

PART 2 
ENDGAME ANCHOR and project implementation 

kernel-first README. AI as a plug-in workload and keeps the repo coherent around the kernel, the rails, Bay-0, the Airlocks, and the enforcement primitives.

Ternary-OS-Kernel

A kernel-first architecture for deterministic isolation, radial I/O topology, and state discontinuities (hinge cycles). This repository is about the kernel and enforcement plane. AI/LLM workloads are optional untrusted plug-ins that run on top of the kernel’s rails and gates.

What this is

Ternary-OS-Kernel is a design and prototype lane that uses a 3-Rail Radial Isolation Topology and capability-based authority to turn security from “software promise” into mechanically enforceable boundaries.

Instead of one monolithic OS where drivers, UI, media, sensors, and “reasoning” compete for the same memory bus and privilege surface, the system is modeled as:

a trusted hub that owns identity, keys, invariants, audit, and the only authority to grant capabilities

multiple untrusted rails that can run rich stacks (Linux, media pipelines, apps, model runtimes), but are forced through Airlocks for any cross-rail interaction

an always-on Bay-0 control plane that never “thinks,” never learns, never accumulates, and exists only to orchestrate, budget, observe, and kill/restart


This is an architecture breakout from the legacy “everything shares one OS and one fate” model.

What this is not

Not a new AI application framework.

Not a monolithic “phone OS replacement” you flash and daily-drive today.

Not a promise of perfect security against physics.

Not a claim that existing silicon magically becomes ternary logic.


This repo is the enforcement kernel lane: the system map, protection domains, gates, audit paths, restart semantics, and the minimal components required to prove the topology works.

The core claim

The endgame is not “more sandboxing.” The endgame is radial isolation with deterministic chokepoints.

A rail can be compromised. A driver can be hostile. A model can be jailbroken. The system stays sovereign because the compromised domain cannot:

1. mint authority


2. bypass gates


3. persist across hinge discontinuities


4. access vault-only material without explicit capability grants



Architecture: 3-Rail Radial Isolation

Think of the system as a hub-and-rings machine.

Blue Hub (Vault / Trusted Kernel Plane)
Holds: identity keys, invariant set (“needles”), audit log root, boot attestation, gate policy, hinge controller.
Owns: capability minting and revocation.

Rail 1: Sensor Rail (Untrusted)
Camera, mic, GPS, IMU, biometrics sensors as devices.
No direct path to reasoning. Must cross a Sensor Airlock into the hub or into the allowed consumer.

Rail 2: Media Rail (Untrusted)
GPU/display/audio/video pipeline.
Goal: zero-copy passthrough for frames and audio where possible, under a capability contract.
Must cross a Media Airlock for anything that touches identity or needs policy mediation.

Rail 3: Reasoning Rail (Untrusted)
LLM runtime, agent loops, “AI plug-in workloads.”
Can be treated as fully compromised and still not reach vault material.
May receive only bounded artifacts (sanitized prompts, orientation scars, coarse signals), not raw secrets.


Why radial matters

A layered stack (“apps on OS on drivers on kernel”) invites bleed-through: a bug down low leaks everything up high. Radial rails invert that. You don’t harden one tower; you separate rooms and control the doors.

The three states: -1 / 0 / +1

The rails describe who runs where. The states describe how data is allowed to move.

State -1 (Vault): trusted authority, long-lived identity, invariant root, audit root.

State 0 (Airlock): neutral gate / transformer. bounded, stateless, deterministic.

State +1 (Untrusted): everything else—apps, media, drivers, model runtimes.


If you remember only one line: State +1 can be owned. State -1 must remain sovereign. State 0 is the only bridge.

Bay-0 (Ghost Hub / Control Plane)

Bay-0 is a deliberate “non-performing observer/orchestrator” role:

no heavy compute

no learning

no memory writes (except audit counters/telemetry in controlled buffers)

deterministic scheduling and budgeting

state transition enforcement

restart / revocation authority routing


Bay-0 exists to prevent runaway depth, endless context accumulation, resource starvation, and covert persistence. If the reasoning rail tries to become “a little OS,” Bay-0 kills it.

Bay-0 is the difference between “we isolated it” and “it slowly grew around the isolation.”

The Hinge: execution discontinuity by design

The Hinge is the mechanism that replaces “cleanup, garbage collection, and long-lived context” with periodic hard discontinuities.

A hinge event may do some or all of:

revoke or rotate capabilities

flush volatile rail memory

reset model KV / caches

restart protection domains

clear registers, wipe scratch buffers

re-establish only the minimal continuity artifact (“scar”)


The hinge is not a crash. It is a scheduled death-and-rebirth cycle that prevents:

slow privilege creep

hidden persistence

“boiling the frog” drift across long context windows

accumulation of attacker footholds


Continuity without accumulation: the Scar

If the reasoning rail needs “orientation” across hinge cycles, it does not get full text history. It gets a bounded, fixed-size artifact like:

a short hash (or hash chain) of last exchange

a pointer into a vault-owned index (not raw content)

a small set of verified flags (topic/goal markers)


The scar is designed so the system can “wake up facing the same direction” without carrying the entire past forward.

Airlocks: the only allowed bridges

An Airlock is a State-0 component that enforces a strict contract:

bounded input

bounded output

deterministic runtime

no hidden state across calls

explicit result codes for allow/redact/violation

explicit write locations (shared buffers) with defined lifetime


Airlocks exist so you never have “a convenient direct path” that becomes the real system over time.

Two canonical airlocks

Sensor Airlock: controls how sensor data is packaged, timestamped, downgraded, rate-limited, or blocked before any other rail can consume it.

Media Airlock: controls frame/audio pipes, including zero-copy mappings where allowed, and blocks any attempt to use media paths as covert exfiltration.


A third Airlock often exists for reasoning:

Reasoning Airlock: scrubs outbound prompts/responses, enforces invariant checks, produces the scar, and triggers crowbar on violation.


The Crowbar: physical or simulated kill-switch

When a rail violates policy (capability violation, unauthorized mapping attempt, forbidden pattern crossing the airlock), the response is not “log and continue.”

The response is:

immediate revocation

restart or power-domain cut of the offending rail

rollback to last verified checkpoint where applicable

audit entry in the hub


In early prototype work (QEMU or dev boards), “physical cut” is simulated by protection domain restart and memory wipe. The semantics are the same: the compromised domain loses continuity.

Capability model: authority is explicit and scarce

The system avoids permission soup. A rail cannot “guess” or “name” a vault object. If it doesn’t hold a capability, the object might as well not exist.

Practical meaning:

you don’t grant “network permission”

you grant a capability to a specific channel, for a specific duration, under a specific airlock policy

capabilities are revocable and time-bounded


This is what keeps the architecture from turning into a bridge to everywhere. Every bridge is explicit.

Threat model (kernel lane)

Assume:

untrusted rails can be compromised (drivers, media stacks, model runtime, user apps)

prompt injection and malicious documents exist

supply chain threats are real

local physical attackers exist with time and tools


Goal:

prevent compromise of untrusted rails from reaching vault identity, invariant set, or authority minting

prevent covert persistence across hinge cycles

prevent silent cross-rail data flow outside airlocks

preserve auditability and deterministic recovery


Non-goals:

perfect resistance to lab-grade side channels without additional physical hardening

solving all manufacturing Trojan classes with software alone


What “breaks the bottleneck” here (and what doesn’t)

This repo is careful about claims.

This architecture does not magically remove physics. What it does is replace the legacy “one bus, one kernel, one contention domain” with:

spatial separation: rails don’t fight for the same cycles at the same microsecond

zero-copy handoff: data stays in place; only authority to view/use it moves (capabilities)

near-domain compute: reasoning runs where its weights/caches live, not by dragging everything through the hub

temporal determinism: hinge resets replace long-lived cleanup behaviors


The payoff is “calm” under load: less jitter, less contention, less accidental coupling, and less persistent attack surface.

Repository layout (intent)

This repo is organized to keep kernel enforcement separate from workloads.

Typical directories you will see (names may evolve):

sel4/
Microkit system definitions, protection domains, build scripts, QEMU targets.

airlock/
State-0 gate code (Rust/C), contracts, tests.

vault_proxy/
Narrow, capability-checked interface into vault-owned services.

audit_sink/
Append-only audit path components (kernel-visible records).

sim/
Attack simulations and harnesses (cap violation, indirect injection, needle leak tests).

docs/
Threat model, capability model, airlock spec, audit schema.


How to reason about “AI plug-ins”

LLMs are treated as untrusted workloads running on Rail 3. They do not become part of the trusted base.

If you attach an LLM:

it consumes sanitized inputs through the reasoning airlock

it emits outputs through the same airlock

it never receives vault keys or invariant roots

it can be restarted at any time without “bricking” the device identity


This keeps the kernel lane clean and prevents the repo from becoming an AI app repo.

Build philosophy: prove topology before performance

The right order is:

1. Boot a minimal system with one PD.


2. Add the monitor/crowbar PD and verify restart semantics.


3. Add the airlock PD and verify strict contracts.


4. Add a “dummy reasoning rail” PD that attempts forbidden accesses and confirm crowbar triggers.


5. Add media rail zero-copy mappings and prove the CPU does not touch pixels.


6. Only then add real workloads.



If you start with AI performance first, you lose the kernel shape.

Milestones

Milestone 0: Skeleton boots

minimal Microkit system map

one PD runs and logs

reproducible build


Milestone 1: Crowbar semantics

violation triggers restart of untrusted rail

memory wiped, continuity broken

audit entry recorded


Milestone 2: Airlock contract locked

allow/redact/violation return codes

bounded runtime and bounded memory

orientation scar produced


Milestone 3: Zero-copy media passthrough

mapped frame buffer capability-only

no memcpy path

stable frame pacing under rail-3 load


Milestone 4: Hinge cycles

periodic reset policy

scar continuity

deterministic recovery of rail states


Contribution rules (important)

This repo is kernel-first. Contributions that blur the trust boundary will be rejected.

Hard rules:

the trusted base stays small

airlocks must be stateless and bounded

capabilities must be explicit

“convenience bridges” are not allowed

AI logic stays on untrusted rails


If you want to add a workload, it goes in a rail folder with clear “untrusted” labeling.

Status

This is an active prototype/spec repo. Expect breaking changes in structure while the topology is proven.


Glossary (short)

Rail: an isolated execution + I/O domain (sensor/media/reasoning)

Hub/Vault: trusted authority and identity root

Airlock: state-0 deterministic gate between domains

Bay-0: control plane/orchestrator, not a workload

Hinge: scheduled discontinuity/reset semantics

Crowbar: kill/restart mechanism for violations

Scar: bounded continuity artifact across hinge cycles

