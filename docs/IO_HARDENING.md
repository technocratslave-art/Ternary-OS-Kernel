Port and power hardening for the Ternary rails

Goal: prevent everyday electrical events from turning into state corruption, latch-up, or rail-to-rail leakage. This is about ESD, EFT/burst, hot-plug transients, cable noise, and brownouts. The hinge protects against persistence; the hardware protections protect against physics.

Design rule zero: clamp at the boundary

Every external interface must have protection at the connector. The protector’s return path must be short and low-inductance, and it should dump to the chassis/entry ground, not deep digital ground.

If the clamp is “inside the building,” the surge already ran through the hallway.

Grounding model

Use two references:

1. Chassis / entry ground (connector shells, shield drains, TVS return)


2. Digital ground (logic, SoCs, memory)



Tie them together at one controlled point near the power-entry / PMIC region (single-point bond). This avoids dumping connector energy across the logic ground plane.

Power domains (fits your rails)

V_BLUE (Vault): always-on, most protected

V_MEDIA (Display/continuity): always-on or long-lived

V_RED (Reasoning): expendable, frequently reset or power-cycled

V_SENSOR (Ingress): can be reset/power-cycled depending on build


Power integrity is part of security. A rail that browns out unpredictably is a rail that can behave nondeterministically.

USB-C (power + data)

USB-C is the highest-risk everyday port because it’s a power port and a cable antenna.

Minimum stack:

TVS on VBUS sized for your system voltage (5 V), placed at the connector

Low-cap ESD/TVS array on high-speed data pairs and CC lines (choose parts intended for USB)

Common-mode choke on the SuperSpeed pairs only if signal integrity allows; otherwise keep layout clean and rely on the ESD array

Ferrite bead or small inductor on VBUS feeding your PMIC/charger, plus bulk + ceramic caps at entry

A proper port controller / charger with robust OVP/OCP and dead-battery behavior

If you can, add a power-path limiter (eFuse / load switch) so a fault doesn’t drag other rails down


Why: you want hot-plug spikes and cable noise to be absorbed at the boundary and never propagate into your rail regulators.

GPIO / UART / debug headers

These are the easiest way for “random wires” to inject garbage.

Minimum stack:

Series resistor per signal (typically 22–100 Ω depending on speed) close to the MCU pin

ESD clamp per line at the header (low-cap, appropriate working voltage)

Keep those traces short, route over a solid ground reference

Prefer keyed/shrouded connectors and avoid long flying leads


Why: series resistance limits current into IO protection diodes, and clamps keep the pin voltage inside survivable bounds.

Audio / mic / analog sensor lines

Analog pins are sensitive and tend to be long.

Minimum stack:

ESD clamp at the jack/connector

RC input filtering (small series R + shunt C) tuned to your bandwidth

Separate analog ground island if needed, then stitch carefully back to digital ground


Why: without filtering, you can get audible pops, ADC glitches, or false triggers that look like “state drift.”

Display ribbon / camera ribbon

Ribbons are noisy and mechanically fragile.

Minimum stack:

ESD arrays at the board-side connector

Tight return paths and solid reference planes under the connector breakout

If the link is high-speed, prioritize impedance-controlled routing over “more parts”

Keep shield/grounding strategy consistent: shields to chassis/entry where possible


Why: the goal is to stop connector-side events from punching into your SoC IO ring.

Battery and main power entry

This is where you prevent brownouts and latch-up loops from becoming “mystery instability.”

Minimum stack:

Reverse protection (ideal diode controller or appropriate MOSFET arrangement)

Input transient clamp appropriate to your battery/charger voltage domain

Polyfuse or current limit where it makes sense for prototypes

A real reset supervisor / brownout monitor for V_BLUE and any always-on controller

Rail sequencing: V_BLUE should come up cleanly, stay up cleanly, and never sag because V_RED misbehaved

Decoupling done right: bulk near entry, ceramics near loads, and no skinny shared return paths


Why: the hinge only helps if the system can reliably cut and restore rails. Power integrity is the “hinge’s muscle.”

Latch-up and “hard reset means power removal”

If you want resets to actually clear fault states, you need true power-domain control on the rails you intend to cleanse. A “soft reset” that leaves the domain biased can fail to clear latch-up or stuck peripherals.

Practical rule:

V_RED should have a load switch / regulator enable that the vault/control plane can drop fully.

V_BLUE should be isolated so V_RED collapse can’t pull it down (separate regulator, careful ground return, controlled inrush).


Validation checklist (what to test before claiming anything)

ESD gun testing at connectors if you can access a lab; otherwise at least perform controlled ESD contact tests with proper precautions

Hot-plug abuse: repeated plug/unplug under load, cheap cables, noisy chargers

Brownout: sweep input voltage down/up and verify deterministic behavior

Rail fault injection: deliberately short V_RED briefly and confirm V_BLUE stays stable

Logic analyzer check: ensure rail kill/rebirth doesn’t corrupt the Airlock mailbox state machine


How it maps to the Ternary model

Airlock integrity depends on clean power and clean IO boundaries. Garbage in should get clamped and filtered before it becomes “meaning.”

Bay-0 enforcement depends on reliable reset and revocation. If power is sloppy, enforcement becomes probabilistic.

The hinge is not a substitute for protection. Protection keeps the system alive; the hinge keeps it clean.
