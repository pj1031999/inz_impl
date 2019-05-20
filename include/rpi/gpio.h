#ifndef RPI_GPIO_H
#define RPI_GPIO_H

#include <types.h>

#define GPIO_PINS 54

/* \brief Pin is input / output or may be assigned alternate function. */
enum {
  GPIO_IN   = 0, 
  GPIO_OUT  = 1, 
  GPIO_ALT5 = 2, 
  GPIO_ALT4 = 3, 
  GPIO_ALT0 = 4, 
  GPIO_ALT1 = 5, 
  GPIO_ALT2 = 6,
  GPIO_ALT3 = 7,
};

/* 
 * \brief Set alternate pin function.
 * \see BCM2835 ARM Peripherals documentation (§6.2)
 */
void gpio_function_select(unsigned pin, unsigned func);

enum {
  GPPUD_PULLOFF  = 0,
  GPPUD_PULLDOWN = 1,
  GPPUD_PULLUP   = 2,
};

/* \brief Configures input pin as pull-up or pull-down. */
void gpio_set_pull(unsigned pin, unsigned pud);

/*
 * \brief delay function
 * \param delay number of cycles to delay
 *
 * This just loops <delay> times in a way that the compiler
 * won't optimize away.
 */
void delay(int64_t count);

#endif /* !RPI_GPIO_H */
