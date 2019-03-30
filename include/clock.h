#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>

void clock_init(void);
bool clock_irq(void);

extern void gen_timer_init();
extern void gen_timer_reset();

extern void enable_irq( void );
extern void disable_irq( void );
extern void put32 ( unsigned long, unsigned int );
extern unsigned int get32 ( unsigned long );

#endif // #ifndef CLOCK_H
