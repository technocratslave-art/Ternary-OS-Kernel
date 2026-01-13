#include <stdint.h>
#include "microkit.h"

#define CH_AUDIT 3
static volatile uint32_t *AUD = (volatile uint32_t *)0x70002000;

void init(void) {}

void notified(microkit_channel ch) {
  if (ch == CH_AUDIT) {
    // MVP: could print via serial if enabled; for now we just “consume”.
    volatile uint32_t code = AUD[0];
    volatile uint32_t arg  = AUD[1];
    (void)code; (void)arg;
  }
}
