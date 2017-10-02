#include <klibc.h>
#include <cons.h>

int getchar(void) {
  return cons_getc();
}
