#include <klibc.h>

void __assert(const char *expr, const char *file, unsigned line) {
  kprintf("Assertion \"%s\" at [%s:%d] failed!\n", expr, file, line);

  for(;;);
}
