#ifndef MMIO_H
#define MMIO_H

#include <types.h>

extern uint8_t _kernel[];

static __attribute__((noinline)) void mmio_write(uint32_t reg, uint32_t data) {
  uint32_t *ptr = (void *)(reg + _kernel);
  __asm__ __volatile__("str %w[data], [%[reg]]"
                   :
                   : [reg] "r"(ptr), [data] "r"(data));
}

static __attribute__((noinline)) uint32_t mmio_read(uint32_t reg) {
  uint32_t *ptr = (void *)(reg + _kernel);
  uint32_t data;
  __asm__ __volatile__("ldr %[data], [%[reg]]"
                   : [data] "=r"(data)
                   : [reg] "r"(ptr));
  return data;
}

#endif /* MMIO_H */
