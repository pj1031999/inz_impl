/* main.c - the entry point for the kernel */

#include <types.h>
#include <stdnoreturn.h>
#include <klibc.h>
#include <cons.h>
#include <gfx.h>
#include <gfx_cons.h>
#include <clock.h>
#include <arm/mmu.h>
#include <pcpu.h>
#include <smp.h>

extern pde_t _kernel_pde[4096];

static window_t *screen;

void cons_bootstrap(unsigned cpu) {
  unsigned w2 = screen->width / 2;
  unsigned h2 = screen->height / 2;

  window_t win;
  gfx_window(screen, &win, (cpu & 1) ? w2 : 0, (cpu & 2) ? h2 : 0, w2, h2);
  gfx_set_bg_col(&win, (cpu ^ (cpu >> 1)) & 1 ? color(16,16,16) : color(32,32,32));
  gfx_rect_draw(&win, 0, 0, w2, h2, win.bg_col);
  cons_init(make_gfx_cons(&win, NULL));
}

void va_bootstrap(void) {
  /* Disable mapping for lower 2GiB */
  for (int i = 0; i < 2048; i++)
    _kernel_pde[i].raw = PDE_TYPE_FAULT;
  /* TODO: TLB flush needed here */
}

void kernel_entry(uint32_t r0 __unused, uint32_t r1 __unused,
                  uint32_t atags __unused)
{
  screen = gfx_set_videomode(1280, 800);

  pcpu_init();
  cons_bootstrap(0);

  puts("CPU#0 started!");

  smp_bootstrap();
  va_bootstrap();

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
