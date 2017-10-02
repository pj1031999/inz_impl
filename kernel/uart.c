/* uart.c - UART initialization & communication */
/* PrimeCell® UART (PL011) */

#include <cdefs.h>
#include <stdint.h>
#include <dev/bcm2836reg.h>
#include <dev/plcomreg.h>
#include <mmio.h>
#include <irq.h>
#include <cons.h>

enum {
  UART0_BASE = BCM2835_PERIPHERALS_BUS_TO_PHYS(BCM2835_UART0_BASE),

  UART0_DR = (UART0_BASE + PL01XCOM_DR),
  UART0_RSR = (UART0_BASE + PL01XCOM_RSR),
  UART0_ECR = (UART0_BASE + PL01XCOM_ECR),
  UART0_FR = (UART0_BASE + PL01XCOM_FR),
  UART0_ILPR = (UART0_BASE + PL01XCOM_ILPR),
  UART0_IBRD = (UART0_BASE + PL011COM_IBRD),
  UART0_FBRD = (UART0_BASE + PL011COM_FBRD),
  UART0_LCRH = (UART0_BASE + PL011COM_LCRH),
  UART0_CR = (UART0_BASE + PL011COM_CR),
  UART0_IFLS = (UART0_BASE + PL011COM_IFLS),
  UART0_IMSC = (UART0_BASE + PL011COM_IMSC),
  UART0_RIS = (UART0_BASE + PL011COM_RIS),
  UART0_MIS = (UART0_BASE + PL011COM_MIS),
  UART0_ICR = (UART0_BASE + PL011COM_ICR),
  UART0_DMACR = (UART0_BASE + PL011COM_DMACR),
  
  // The GPIO registers base address.
  GPIO_BASE = BCM2835_PERIPHERALS_BUS_TO_PHYS(BCM2835_GPIO_BASE),
  // Controls actuation of pull up/down to ALL GPIO pins.
  GPPUD = (GPIO_BASE + 0x94),
  // Controls actuation of pull up/down for specific GPIO pin.
  GPPUDCLK0 = (GPIO_BASE + 0x98),
};

/*
 * delay function
 * int32_t delay: number of cycles to delay
 *
 * This just loops <delay> times in a way that the compiler
 * wont optimize away.
 */
static void delay(int32_t count) {
  __asm__ volatile("1: subs %[count], %[count], #1; bne 1b"
                   : [count] "+r"(count));
}

static void pl011_init() {
  // Disable UART0.
  mmio_write(UART0_CR, 0);
  // Setup the GPIO pin 14 && 15.

  // Disable pull up/down for all GPIO pins & delay for 150 cycles.
  mmio_write(GPPUD, 0);
  delay(150);

  // Disable pull up/down for pin 14,15 & delay for 150 cycles.
  mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
  delay(150);

  // Write 0 to GPPUDCLK0 to make it take effect.
  mmio_write(GPPUDCLK0, 0);

  // Clear pending interrupts.
  mmio_write(UART0_ICR, PL011_INT_ALLMASK);

  // Set integer & fractional part of baud rate.
  // Divider = UART_CLOCK/(16 * Baud)
  // Fraction part register = (Fractional part * 64) + 0.5
  // UART_CLOCK = 3000000; Baud = 115200.

  // Divider = 3000000/(16 * 115200) = 1.627 = ~1.
  // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
  mmio_write(UART0_IBRD, 1);
  mmio_write(UART0_FBRD, 40);

  // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
  mmio_write(UART0_LCRH, PL01X_LCR_FEN | PL01X_LCR_8BITS);

  // Mask all interrupts.
  mmio_write(UART0_IMSC, PL011_INT_ALLMASK);

  // Enable UART0, receive & transfer part of UART.
  mmio_write(UART0_CR, PL01X_CR_UARTEN | PL011_CR_TXE | PL011_CR_RXE);
}


static void pl011_putc(cons_dev_t *dev __unused, int c) {
  /* wait for UART to become ready to transmit */
  while (mmio_read(UART0_FR) & PL01X_FR_TXFF)
    ;
  mmio_write(UART0_DR, c);
}

static int pl011_getc(cons_dev_t *dev __unused) {
  /* wait for UART to have recieved something */
  while (mmio_read(UART0_FR) & PL01X_FR_RXFE)
    ;
  return mmio_read(UART0_DR);
}

static void pl011_flush(cons_dev_t *dev __unused) {
}

struct cons_dev {};

cons_t uart0_cons = {
  .init = pl011_init,
  .getc = pl011_getc,
  .putc = pl011_putc,
  .flush = pl011_flush,
};
