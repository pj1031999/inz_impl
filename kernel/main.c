/* main.c - the entry point for the kernel */

#include <stdint.h>
#include <stdnoreturn.h>

#include <cdefs.h>
#include <klibc.h>
#include <cons.h>
#include <gfx.h>
#include <clock.h>
#include <armmmu.h>

extern pde_t _kernel_pde[4096];

extern cons_t uart0_cons;

void kernel_entry(uint32_t r0 __unused, uint32_t r1 __unused,
                  uint32_t atags __unused)
{

  /* Apparently graphics has to be setup before lower memory is detached. */
  uint32_t width = 1366;
  uint32_t height = 768;
  uint32_t *framebuffer = set_gfx_mode(width, height, 32);

  /* Disable mapping for lower 2GiB */
  for (int i = 0; i < 2048; i++)
    _kernel_pde[i].raw = PDE_TYPE_FAULT;
  /* TODO: TLB flush needed here */

  cons_init(&uart0_cons);

  uint32_t *pos = framebuffer;
  puts("Hello world!");

  printf("Framebuffer address: %p\n", framebuffer);

  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++, pos++) {
      *pos = (h >> 2 << 8) | (w >> 2);
    }
  }
  printf("Config Register: %08x\n", armreg_sctlr_read());

  clock_init();

  puts("Type letter 'q' to halt machine!");
  while (getchar() != 'q')
    ;

  puts("*** system halting ***\n");
}

noreturn void kernel_exit() {
  for (;;) {
    __asm__ volatile("wfe");
  }
}
