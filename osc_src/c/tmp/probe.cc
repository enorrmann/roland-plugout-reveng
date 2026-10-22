#include <stdint.h>
#define OSC_DAT(a) (*(volatile uint32_t *)(uintptr_t)(a))
static const uint8_t D[] __attribute__((section(".osc_dr"))) = "\x01\x02\x03";
void f(void) {
  OSC_DAT(0x600003e0) = 5;
  OSC_DAT(0x600003d8) = OSC_DAT(0x600003e8) + 3 - 1;
  if ((OSC_DAT(0x600003dc) & 0x70000) != 0x50000) {}
}
