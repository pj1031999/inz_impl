/* main.c - the entry point for the kernel */

#include <aarch64/cpu.h>
#include <types.h>
#include <klibc.h>
#include <cons.h>
#include <gfx.h>
#include <gfx_cons.h>
#include <clock.h>
#include <rpi/irq.h>
#include <aarch64/mmu.h>
#include <pmman.h>
#include <pcpu.h>
#include <smp.h>
#include <userspace_demo.h>

extern uint8_t _brk_limit;

static window_t *screen;
extern cons_t uart0_cons;

void cons_bootstrap(unsigned cpu) {
  unsigned w2 = screen->width / 2;
  unsigned h2 = screen->height / 2;

  window_t win;
  gfx_window(screen, &win, (cpu & 1) ? w2 : 0, (cpu & 2) ? h2 : 0, w2, h2);
  gfx_set_bg_col(&win, (cpu ^ (cpu >> 1)) & 1 ? color(16,16,16) : color(32,32,32));
  gfx_rect_draw(&win, 0, 0, w2, h2, win.bg_col);
  cons_init(make_gfx_cons(&win, NULL));
}

void kernel_entry(uint32_t r0 __unused, uint32_t r1 __unused,
                  uint32_t atags __unused)
{
  pcpu_init();

  screen = gfx_set_videomode(1280, 800);

  cons_bootstrap(0);
  
  /* bcm2835_irq_init(); */
  /* bcm2836_local_irq_init(); */
  /* arm_irq_enable(); */

  pm_init();
  pm_add_segment(0, BCM2835_PERIPHERALS_BASE);
  pm_reserve(0, mmu_translate((vaddr_t)&_brk_limit));

  puts("CPU#0 started!");

  smp_bootstrap();

  printf("Config Register: %08x\n", reg_sctlr_el1_read());
  printf("Framebuffer address: %p\n", screen->pixels);

  //clock_init();
  //uart0_cons.init(NULL);
  demo_uart();
  demo_clock_switch();
  
  puts("Type letter 'q' to halt machine!");
  uint8_t c;
  while ((c = getchar()) != 'q'){
    printf("%c", c);
  }

    
  //for(;;);
  kernel_exit();
}

noreturn void kernel_exit() {
  arm_irq_disable();
  printf("\n*** CPU#%d halted! ***", arm_cpu_id());
  for (;;);
}
