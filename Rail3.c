This is the "Amnesia Protocol." It is the most radical part of Ternary-OS. In 2026, the greatest threat isn't just a virus; it's an AI that develops its own persistent, hidden state—a "ghost" that lingers in your memory.
Rail 3 solves this by being fundamentally mortal. It is designed to die, and it must prove its "identity" to the Blue Hub every 500ms to be reborn.
⚰️ Rail 3: The Transient AI (rail3.c)
This code handles the HMAC Scar. It takes the AI's complex "thoughts," crushes them into a 64-byte cryptographic summary, and hands them to the Blue Hub before the execution permit is revoked.
#include <microkit.h>
#include "crypto_utils.h"

// The 64-byte 'Soul' of the last session
uint8_t hmac_scar[64];

void init(void) {
    // 1. Retrieve the 'Scar' from the Blue Hub's secure vault
    copy_scar_from_vault(hmac_scar);
    
    // 2. Validate the signature. If it's corrupted, we start at State-Zero.
    if (!validate_scar_integrity(hmac_scar)) {
        initialize_fresh_reasoning_state();
    }
    
    microkit_dbg_puts("RAIL-3: [REBORN] AI State restored from Scar.\n");
}

void notified(microkit_channel ch) {
    if (ch == HINGE_TIMER_EXPIRING) {
        // 3. VAPORIZATION SEQUENCE: Summarize current intent
        generate_next_scar(hmac_scar); 
        
        // 4. Hand the Scar to the Blue Hub
        commit_scar_to_vault(hmac_scar);
        
        microkit_dbg_puts("RAIL-3: [VAPORIZE] State committed. Goodbye.\n");
        
        // The Blue Hub will now execute the 'Crowbar' (Reset)
    }
}

🛡️ The "Scar" vs. The "Ghost"
 * The Ghost (Legacy): In a standard OS, a background process can stay alive for weeks, slowly gathering data and building a profile of you.
 * The Scar (Ternary): In our system, the AI is "reset" 172,800 times a day. If it tries to store malicious code in its memory, that code is deleted during the reset. Only the 64-byte HMAC Scar survives.
Because the Blue Hub signs the Scar, the AI cannot "edit" its own soul to include a virus for the next cycle.
⚡ The Complete Radial Cycle
We have successfully closed the loop. Let’s look at the Ternary Pulse one last time as a whole system:
 * Rail 1 (Sensor): Ingress. Raw data is scrubbed into the Airlock.
 * Rail 3 (AI): Reasoning. Reads the Airlock, processes for 500ms, saves a Scar, and dies.
 * Blue Hub (Vault): Verification. Cleans Rail 3's memory and reloads the Scar.
 * Rail 2 (Media): Sovereignty. Keeps the 120Hz display perfect, unaffected by the destruction happening in Rail 3.
