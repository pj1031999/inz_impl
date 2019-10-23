#include <cons.h>
#include <pcpu.h>
#include <aarch64/cpu.h>

void cons_init(cons_t *cn) {
  pcpu()->cons = cn;
  cn->init(cn->dev);
}

void cons_putc(int c) {
  arm_irq_disable();

  cons_t *cn = pcpu()->cons;
  cn->putc(cn->dev, c);

  arm_irq_enable();
}

int cons_getc() {
  cons_t *cn = pcpu()->cons;
  return cn->getc(cn->dev);
}

void cons_flush() {
  cons_t *cn = pcpu()->cons;
  cn->flush(cn->dev);
}
