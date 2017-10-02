#include <cons.h>

static cons_t *console;

void cons_init(cons_t *cn) {
  console = cn;
  console->init(console->dev);
}

void cons_putc(int c) {
  console->putc(console->dev, c);
}

int cons_getc() {
  return console->getc(console->dev);
}

void cons_flush() {
  console->flush(console->dev);
}
