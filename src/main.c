/* main.c - the entry point for the kernel */

#include <stdint.h>
#include <stdnoreturn.h>
#include <uart.h>

void *set_gfx_mode(uint32_t w, uint32_t h, uint32_t bpp);

#define UNUSED(x) (void)(x)

static const char *to_hex = "0123456789ABCDEF";

static void print_hex(uint32_t number) {
  for (int i = 7; i >= 0; i--) {
    uart_putc(to_hex[(number >> (i * 4)) & 0xF]);
  }
}

extern uint32_t _kernel_pde[4096];

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
  UNUSED(r0);
  UNUSED(r1);
  UNUSED(atags);

  uart_init();
  uart_puts("Hello world!\n");

  /* Disable mapping for lower 3GiB */
  for (int i = 0; i < 3072; i++)
    _kernel_pde[i] = 0;
  /* TODO: TLB flush needed here */

  {
    uint32_t tmp;
    __asm__ volatile ("mrc p15, 0, %0, c1, c0, 0":"=r"(tmp));
    print_hex((uint32_t)tmp);
    uart_putc('\n');
  }

  uint32_t width = 1366;
  uint32_t height = 768;
  uint32_t *framebuffer = set_gfx_mode(width, height, 32);
  uint32_t *pos = framebuffer;

  uart_puts("Framebuffer address: ");
  print_hex((uint32_t)framebuffer);
  uart_putc('\n');

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
