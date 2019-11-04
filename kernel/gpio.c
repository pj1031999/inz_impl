#include <mmio.h>
#include <dev/bcm2836reg.h>
#include <rpi/gpio.h>
#include <klibc.h>

/* The GPIO registers base address. */
#define GPIO_BASE BCM2835_PERIPHERALS_BUS_TO_PHYS(BCM2835_GPIO_BASE)

/* The function select registers are used to define the operation of the
 * general-purpose I/O pins. */
#define GPFSEL(x) (GPIO_BASE + (x) * sizeof(uint32_t))

#define GPFSEL_PINS_PER_REGISTER 10
#define GPFSEL_BITS_PER_PIN 3

/* The output set registers are used to set a GPIO pin. */
#define GPSET(x) (GPIO_BASE + 0x1C + (x) * sizeof(uint32_t))

/* The output clear registers are used to clear a GPIO pin. */
#define GPCLR(x) (GPIO_BASE + 0x28 + (x) * sizeof(uint32_t))

/* The pin level registers return the actual value of the pin. */
#define GPLEV(x) (GPIO_BASE + 0x34 + (x) * sizeof(uint32_t))

/* The event detect status registers are used to record level and edge events
 * on the GPIO pins. */
#define GPEDS(x) (GPIO_BASE + 0x40 + (x) * sizeof(uint32_t))

/* The rising edge detect enable registers define the pins for which a rising
 * edge transition sets a bit in the event detect status registers GPEDS. */
#define GPREN(x) (GPIO_BASE + 0x4C + (x) * sizeof(uint32_t))

/* The falling edge detect enable registers define the pins for which a
 * falling edge transition sets a bit in the event detect status registers
 * GPEDS. */
#define GPFEN(x) (GPIO_BASE + 0x58 + (x) * sizeof(uint32_t))

/* The high level detect enable registers define the pins for which a high
 * level sets a bit in the event detect status register GPEDS. */
#define GPHEN(x) (GPIO_BASE + 0x64 + (x) * sizeof(uint32_t))

/* The low level detect enable registers define the pins for which a low level
 * sets a bit in the event detect status register GPEDS. */
#define GPLEN(x) (GPIO_BASE + 0x70 + (x) * sizeof(uint32_t))

/* The asynchronous rising edge detect enable registers define the pins for
 * which a asynchronous rising edge transition sets a bit in the event detect
 * status registers GPEDS. */
#define GPAREN(x) (GPIO_BASE + 0x7C + (x) * sizeof(uint32_t))

/* The asynchronous falling edge detect enable registers define the pins for
 * which a asynchronous falling edge transition sets a bit in the event detect
 * status registers GPEDS. */
#define GPAFEN(x) (GPIO_BASE + 0x88),

/* Controls actuation of pull up/down to ALL GPIO pins. */
#define GPPUD (GPIO_BASE + 0x94)

#define GPPUD_PINS_PER_REGISTER 32

/* Controls actuation of pull up/down for specific GPIO pin. */
#define GPPUDCLK(x) (GPIO_BASE + 0x98 + (x) * sizeof(uint32_t))

/*
 * \brief delay function
 * \param delay number of cycles to delay
 *
 * This just loops <delay> times in a way that the compiler
 * won't optimize away.
 */
void delay(int64_t count) {
  __asm__ volatile("1: subs %[count], %[count], #1; bne 1b"
                   : [ count ] "+r"(count));
}

void gpio_function_select(unsigned pin, unsigned func) {
  assert(pin < GPIO_PINS);
  assert(func <= GPIO_ALT3);

  unsigned mask = __BIT(GPFSEL_BITS_PER_PIN) - 1;
  unsigned reg = pin / GPFSEL_PINS_PER_REGISTER;
  unsigned shift = (pin % GPFSEL_PINS_PER_REGISTER) * GPFSEL_BITS_PER_PIN;

  unsigned val = mmio_read(GPFSEL(reg));
  val &= ~(mask << shift);
  val |= (func << shift);
  mmio_write(GPFSEL(reg), val);
}

void gpio_set_pull(unsigned pin, unsigned pud) {
  assert(pin < GPIO_PINS);
  assert(pud <= GPPUD_PULLUP);

  unsigned mask = __BIT(pin % GPPUD_PINS_PER_REGISTER);
  unsigned reg = pin / GPPUD_PINS_PER_REGISTER;

  mmio_write(GPPUD, pud);
  delay(150);
  mmio_write(GPPUDCLK(reg), mask);
  delay(150);
  mmio_write(GPPUD, GPPUD_PULLOFF);
  mmio_write(GPPUDCLK(reg), 0);
}
