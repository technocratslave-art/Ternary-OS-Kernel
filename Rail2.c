This is the "Showpiece." Rail 2 is the sovereign domain—the part of the system the user actually touches. In Ternary-OS, this rail is designed to be a "Passive Conduit." It doesn't process data; it just allows it to flow at high velocity.
By implementing the 120Hz Heartbeat here, we prove that the Blue Hub can keep the user experience perfectly smooth, even while the AI on Rail 3 is undergoing its "500ms Vaporization" cycle.
🏎️ Rail 2: The Sovereignty Loop (rail2.c)
This code manages the Zero-Copy Passthrough. Notice there is no "drawing" logic here; we are simply synchronizing the hardware's clock to the display's refresh rate.
#include <microkit.h>
#include "ternary_api.h"

// The Direct-Mapped Frame Buffer (from system.xml)
volatile uint32_t *display_buffer = (uint32_t *)0x41000000;

void init(void) {
    microkit_dbg_puts("RAIL-2: [INIT] Media Sovereignty Established.\n");
    microkit_dbg_puts("RAIL-2: [INFO] 120Hz Bay-Zero Pulse active.\n");
}

void notified(microkit_channel ch) {
    switch (ch) {
        case V_SYNC_PULSE:
            // This is the Zero-Copy moment. 
            // We simply 'flip' the hardware pointer. No copying.
            flip_display_buffer(display_buffer);
            
            // Signal the Hub that the Operator is calm and clear.
            microkit_notify(BLUE_HUB_HB);
            break;

        case AIRLOCK_DATA_READY:
            // Optional: Overlay AI 'thoughts' onto the screen
            // without letting the AI touch the buffer itself.
            render_secure_overlay();
            break;
    }
}

💓 The Bay-Zero Pulse vs. The Hinge
The magic of this architecture happens in the Timing.
 * The Pulse (120Hz): Rail 2 ticks every 8.33 milliseconds.
 * The Hinge (2Hz): Rail 3 is vaporized every 500 milliseconds.
Because Rail 2 is isolated, the "Vaporization" of the AI is completely invisible to the display loop. On a legacy phone, the AI restarting would cause the screen to "hiccup" or drop a frame. On Ternary-OS, the square wave of Rail 2 never wavers.
🛡️ Preventing the "Ghost"
Because of the system.xml we wrote earlier, Rail 2 has RW (Read/Write) access to the pixels, but Rail 3 has ZERO access.
Even if the AI becomes "sentient" and tries to see what you are typing on your screen, it will find that its memory map for 0x41000000 is a void. It is physically blind to your sovereignty.
