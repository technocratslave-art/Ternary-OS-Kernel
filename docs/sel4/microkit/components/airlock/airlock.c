#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "microkit.h"

#define CH_WORKLOAD 1
#define CH_VAULT    2
#define CH_AUDIT    3

// The shared airlock buffer is mapped at 0x70000000 via system XML.
static volatile uint8_t *AIRLOCK_BUF = (volatile uint8_t *)0x70000000;

// Rust policy interface (implemented in rust/src/lib.rs)
extern uint32_t airlock_scrub_inplace(uint8_t *buf, uint32_t len);
extern uint32_t airlock_detected_violation(void);

// Simple audit event codes
enum {
  EVT_OK_FORWARD        = 0x1001,
  EVT_REDACTED          = 0x1002,
  EVT_BLOCKED_VIOLATION = 0x1003,
  EVT_CROWBAR_SIM       = 0x2001
};

static void audit_emit(uint32_t code, uint32_t arg) {
  // Minimal: place code/arg into audit shared buffer, notify audit sink.
  volatile uint32_t *AUD = (volatile uint32_t *)0x70002000;
  AUD[0] = code;
  AUD[1] = arg;
  microkit_notify(CH_AUDIT);
}

// “Crowbar” simulation: in real hardware this would close gates / cut power.
// In MVP: we just stop forwarding and keep producing an immutable event.
static void crowbar_sim(uint32_t reason) {
  audit_emit(EVT_CROWBAR_SIM, reason);
  // Optionally: keep workload in “blocked” state by not responding.
}

void init(void) {
  // nothing
}

void notified(microkit_channel channel) {
  if (channel == CH_WORKLOAD) {
    // Convention: first 4 bytes of buffer = length (u32), followed by payload.
    uint32_t len = *((volatile uint32_t *)(AIRLOCK_BUF + 0));
    if (len > 0x1F00) len = 0x1F00; // hard cap

    uint8_t *payload = (uint8_t *)(AIRLOCK_BUF + 4);

    // Run verified-ish policy logic in Rust (in-place scrub)
    uint32_t redactions = airlock_scrub_inplace(payload, len);

    if (airlock_detected_violation()) {
      audit_emit(EVT_BLOCKED_VIOLATION, 0xDEAD0001);
      crowbar_sim(0xDEAD0001);
      return;
    }

    if (redactions > 0) {
      audit_emit(EVT_REDACTED, redactions);
    } else {
      audit_emit(EVT_OK_FORWARD, len);
    }

    // Forward to Vault proxy (still through Airlock)
    microkit_notify(CH_VAULT);
  }
}
