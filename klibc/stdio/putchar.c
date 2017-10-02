#include <klibc.h>
#include <cons.h>

void putchar(int c) {
  return cons_putc(c);
}
