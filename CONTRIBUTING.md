📜 CONTRIBUTING.md: The Laws of the Rails
Welcome to the Ternary-OS project. By contributing, you are committing to the elimination of "Electrical Violence." We do not "fix" bugs here; we enforce Geometry.
1. The Rule of Isolation
Code written for one Rail must never include headers or references to another Rail.
 * Rail 1 (Sensor) only talks to the Blue Hub.
 * Rail 3 (AI) is blind to the Media Buffer.
 * If your code requires data from another rail, you must request an Airlock Token from the Blue Hub.
2. Memory is a Fortress
We do not use malloc() or dynamic heap allocation in the core kernel. All memory must be statically mapped in the linker.ld file.
 * Every byte must have a physical address.
 * If it isn't in the memory map, it doesn't exist.
3. The 500ms Duty Cycle
Any logic submitted for Rail 3 (Reasoning) must be stateless. You must assume your code will be vaporized every 500ms.
 * To persist data, you must serialize it into the 64-byte HMAC Scar.
 * If your payload exceeds 64 bytes, it will be truncated by the Crowbar.
4. Zero-Copy or Nothing
If you are writing drivers for Rail 2 (Media), you must use DMA Passthrough. We do not "copy" pixels; we move the hardware's attention. If your PR includes a memcpy() of a frame buffer, it will be closed immediately.
🏛️ The "Architectural Integrity" Check
Every Pull Request must pass the Thermal & Jitter Simulation. We run your code on a virtual seL4 node and measure the "Waveform."
 * Green Light: The wave remains a perfect 120Hz square.
 * Red Light: Your code caused a "Micro-Stutter" or a thermal spike. Optimization required.
The Forge is Open.

