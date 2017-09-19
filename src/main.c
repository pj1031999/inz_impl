/* main.c - the entry point for the kernel */

#include <stdint.h>
#include <stdnoreturn.h>

#include <cdefs.h>
#include <klibc.h>
#include <uart.h>
#include <gfx.h>

extern uint32_t _kernel_pde[4096];

void kernel_main(uint32_t r0 __unused, uint32_t r1 __unused,
                 uint32_t atags __unused)
{
  uart_init();
  uart_puts("Hello world!\n");

  /* Apparently graphics has to be setup before lower memory is detached. */
  uint32_t width = 1366;
  uint32_t height = 768;
  uint32_t *framebuffer = set_gfx_mode(width, height, 32);

  /* Disable mapping for lower 2GiB */
  for (int i = 0; i < 2048; i++)
    _kernel_pde[i] = 0;
  /* TODO: TLB flush needed here */

  {
    uint32_t cr;
    __asm__ volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r" (cr));
    kprintf("Config Register: %08x\n", cr);
  }

  uint32_t *pos = framebuffer;

  kprintf("Framebuffer address: %p\n", framebuffer);

  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++, pos++) {
      *pos = (h >> 2 << 8) | (w >> 2);
    }
  }

  uart_puts("Type letter 'q' to halt machine!\n");
  while (uart_getc() != 'q')
    ;

  uart_puts("*** system halting ***\n");
}

noreturn void kernel_exit() {
  for (;;) {
    __asm__ volatile("wfe");
  }
}
