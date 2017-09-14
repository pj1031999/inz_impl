/* mmio.h - access to MMIO registers */

#ifndef MMIO_H
#define MMIO_H

#include <stdint.h>

enum {
  MMIO_BASE = 0x3F000000,

  // The GPIO registers base address.
  GPIO_BASE = (MMIO_BASE + 0x200000),

  // The offsets for reach register.

  // Controls actuation of pull up/down to ALL GPIO pins.
  GPPUD = (GPIO_BASE + 0x94),

  // Controls actuation of pull up/down for specific GPIO pin.
  GPPUDCLK0 = (GPIO_BASE + 0x98),
};

// write to MMIO register
static inline void mmio_write(uint32_t reg, uint32_t data) {
  uint32_t *ptr = (uint32_t *)reg;
  __asm__ volatile("str %[data], [%[reg]]"
                   :
                   : [reg] "r"(ptr), [data] "r"(data));
}

// read from MMIO register
static inline uint32_t mmio_read(uint32_t reg) {
  uint32_t *ptr = (uint32_t *)reg;
  uint32_t data;
  __asm__ volatile("ldr %[data], [%[reg]]"
                   : [data] "=r"(data)
                   : [reg] "r"(ptr));
  return data;
}

#endif // #ifndef MMIO_H
