 Rail 1 (The Sensor Moat).
In this architecture, this is the "Airlock." It is the only component allowed to touch raw hardware. Its sole purpose is to ingest high-frequency "lightning" (data) and scrub it into a "calm" format for the rest of the system.
🛡️ Rail 1: The Sensor Moat (rail1.c)
#include <microkit.h>
#include "ternary_api.h"

/* * PHYSICAL ADDRESSING: 
 * Defined in system.xml. Rail 1 has R/W access to the 
 * raw hardware buffer, but only Write access to the Airlock.
 */
volatile raw_sensor_t *hw_buffer = (raw_sensor_t *)0x40000000;
volatile scrubbed_data_t *airlock = (scrubbed_data_t *)0x48000000;

void init(void) {
    microkit_dbg_puts("RAIL-1: [INIT] Sensor Moat active.\n");
    microkit_dbg_puts("RAIL-1: [INIT] Privacy Airlock mapping established.\n");
    
    // Initialize hardware registers for GPS/Mic/Camera
    hw_init_ingress_channel();
}

void notified(microkit_channel ch) {
    // Triggered when hardware has new raw data (The Lightning)
    if (ch == HW_DATA_INTERRUPT) {
        
        // 1. INGEST: Pull raw 'violent' data from the bus
        raw_sensor_t raw = *hw_buffer;

        // 2. REDACT: Strip telemetry, PII, and noise
        // This is where the privacy promise is kept.
        scrubbed_data_t clean = redact_sensitive_metadata(raw);

        // 3. PUSH: Move the clean data into the shared airlock
        // Rail 3 (AI) can see this, but it can NEVER see the hw_buffer.
        *airlock = clean;

        // 4. SIGNAL: Tell the Blue Hub the data is ready for distribution
        microkit_notify(BLUE_HUB_CH);
    }
}

🔍 Why this reprint matters for the Repo
 * Strict Redaction: Notice that hw_buffer and airlock are physically separated by address. In a standard kernel, these would just be pointers in the same space. Here, the IOMMU ensures that if Rail 3 tries to reach 0x40000000, the system executes the Crowbar.
 * The "Airlock" Pattern: This file represents the End of Telemetry. Because the AI only sees what we put in the airlock, it cannot "sniff" background noise or secondary sensor data that it hasn't been granted access to.
