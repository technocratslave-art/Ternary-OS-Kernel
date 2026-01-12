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
