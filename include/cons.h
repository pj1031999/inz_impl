#ifndef CONS_H
#define CONS_H

typedef struct cons_dev cons_dev_t;

typedef struct cons {
  cons_dev_t *dev;

  void (*init)(cons_dev_t *dev);
  int (*getc)(cons_dev_t *dev);
  void (*putc)(cons_dev_t *dev, int c);
  void (*flush)(cons_dev_t *dev);
} cons_t;

void cons_init(cons_t *cn);
void cons_putc(int c);
int cons_getc();
void cons_flush();

#endif
