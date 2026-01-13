#include <stdint.h>
#include "microkit.h"

#define CH_VAULT 2

void init(void) {}

void notified(microkit_channel ch) {
  if (ch == CH_VAULT) {
    // In MVP, we do nothing. Real vault would serve read-only queries
    // via caps, never accepting writes from +1.
  }
}
