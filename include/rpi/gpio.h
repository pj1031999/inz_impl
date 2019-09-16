#ifndef RPI_GPIO_H
#define RPI_GPIO_H

#include <types.h>

#define MMIO_BASE       0xFFFFFFFF3F000000

#define GPFSEL0         ((volatile uint32_t*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile uint32_t*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile uint32_t*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile uint32_t*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile uint32_t*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile uint32_t*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile uint32_t*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile uint32_t*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile uint32_t*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile uint32_t*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile uint32_t*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile uint32_t*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile uint32_t*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile uint32_t*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile uint32_t*)(MMIO_BASE+0x00200068))

#define GPPUDCLK0       ((volatile uint32_t*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile uint32_t*)(MMIO_BASE+0x0020009C))

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
