/* main.c - the entry point for the kernel */

#include <stdint.h>
#include <stdnoreturn.h>

#include <cdefs.h>
#include <bcm2836reg.h>
#include <klibc.h>
#include <cons.h>
#include <gfx.h>
#include <gfx_cons.h>
#include <clock.h>
#include <armmmu.h>

extern pde_t _kernel_pde[4096];

static cons_t *gfx_cons[BCM2836_NCPUS];

static void cons_bootstrap(void) {
  window_t *screen = gfx_set_videomode(1280, 800);

  unsigned w2 = screen->width / 2;
  unsigned h2 = screen->height / 2;

  for (int cpu = 0; cpu < BCM2836_NCPUS; cpu++) {
    window_t win;
    gfx_window(screen, &win, (cpu & 1) ? w2 : 0, (cpu & 2) ? h2 : 0, w2, h2);
    gfx_set_bg_col(&win, (cpu ^ (cpu >> 1)) & 1 ? color(16,16,16) : color(32,32,32));
    gfx_rect_draw(&win, 0, 0, w2, h2, win.bg_col);
    gfx_cons[cpu] = make_gfx_cons(&win, NULL);
  }
}

void kernel_entry(uint32_t r0 __unused, uint32_t r1 __unused,
                  uint32_t atags __unused)
{

  cons_bootstrap();
  cons_init(gfx_cons[0]);

  /* Disable mapping for lower 2GiB */
  for (int i = 0; i < 2048; i++)
    _kernel_pde[i].raw = PDE_TYPE_FAULT;
  /* TODO: TLB flush needed here */

  puts("Hello world!");


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
