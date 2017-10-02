#include <klibc.h>
#include <cons.h>

void puts(const char *str) {
  while (*str)
    cons_putc(*str++);
  cons_putc('\n');
  cons_flush();
}
