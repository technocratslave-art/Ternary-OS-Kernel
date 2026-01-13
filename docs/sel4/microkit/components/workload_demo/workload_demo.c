#include <stdint.h>
#include <string.h>
#include "microkit.h"

#define CH_WORKLOAD 1
static volatile uint8_t *BUF = (volatile uint8_t *)0x70000000;

static void send_msg(const char *msg) {
  uint32_t len = (uint32_t)strlen(msg);
  *((volatile uint32_t *)(BUF + 0)) = len;
  memcpy((void *)(BUF + 4), msg, len);
  microkit_notify(CH_WORKLOAD);
}

void init(void) {
  // Normal message
  send_msg("hello from workload");

  // Simulated “needle leak attempt” (will trigger violation)
  send_msg("... NEEDLE_HASH_EXAMPLE_1 ...");
}

void notified(microkit_channel ch) {
  (void)ch;
}
