/* uart.c - UART initialization & communication */
/* PrimeCell® UART (PL011) */

#include <types.h>
#include <dev/bcm2836reg.h>
#include <dev/plcomreg.h>
#include <mmio.h>
#include <rpi/gpio.h>
#include <rpi/irq.h>
#include <cons.h>
#include <klibc.h>
#include <rpi/vc_mbox.h>
#include <rpi/vc_prop.h>

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
};

static void pl011_irq(unsigned irq __unused);

static void pl011_init(cons_dev_t *dev __unused) {
  /* Disable UART0. */
  mmio_write(UART0_CR, 0);
  /* Clear pending interrupts. */
  mmio_write(UART0_ICR, PL011_INT_ALLMASK);

  extern unsigned int _mail_buffer[256];
  /* static volatile unsigned int mailbuffer[256] __attribute__((aligned (16)));
   */
  unsigned long physical_mb =
    (unsigned long)_mail_buffer; // will cut to 32 bit value at vc_mbox_send
  unsigned int res;

  // set up clock for consistent divisor values
  _mail_buffer[0] = 8 * 4; // buffer size
  _mail_buffer[1] = VCPROPTAG_REQUEST;
  _mail_buffer[2] = VCPROPTAG_SET_CLOCKRATE;
  _mail_buffer[3] = 12;
  _mail_buffer[4] = 8;
  _mail_buffer[5] = VCPROP_CLK_UART;
  _mail_buffer[6] = 4000000; // 4Mhz
  _mail_buffer[7] = VCPROPTAG_END;

  vc_mbox_send(physical_mb, 8);
  res = vc_mbox_recv(8);

  /* enable UART0 on pins 14 & 15 */
  gpio_function_select(14, GPIO_ALT0);
  gpio_function_select(15, GPIO_ALT0);
  gpio_set_pull(14, GPPUD_PULLOFF);
  gpio_set_pull(15, GPPUD_PULLOFF);

  /*
   * Set integer & fractional part of baud rate.
   * Divider = UART_CLOCK/(16 * Baud)
   * Fraction part register = (Fractional part * 64) + 0.5
   * UART_CLOCK = 3000000; Baud = 115200.
   *
   * Divider = 4000000/(16 * 115200) = 2.17 = ~2.
   * Fractional part register = (.17 * 64) + 0.5 = 11.3 = ~11 = 0xb.
   */
  mmio_write(UART0_IBRD, 2);
  mmio_write(UART0_FBRD, 0xb);

  /* Enable FIFO & 8 bit data transmission (1 stop bit, no parity). */
  mmio_write(UART0_LCRH, PL01X_LCR_FEN | PL01X_LCR_8BITS);

  /* Mask all interrupts. */
  mmio_write(UART0_IMSC, PL011_INT_ALLMASK);

  /* Enable UART0, receive & transfer part of UART. */
  mmio_write(UART0_CR, PL01X_CR_UARTEN | PL011_CR_TXE | PL011_CR_RXE);

  /* Enable receive interrupt. */
  mmio_write(UART0_IMSC, PL011_INT_RX);
  bcm2835_irq_register(BCM2835_INT_UART0, pl011_irq);
  bcm2835_irq_enable(BCM2835_INT_UART0);
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
  int c = mmio_read(UART0_DR);
  // pl011_putc(NULL, c);
  return c;
}

static void pl011_flush(cons_dev_t *dev __unused) {
}

static void pl011_irq(unsigned irq __unused) {
  printf("UART0: received '%c'!\n", pl011_getc(NULL));
}

struct cons_dev {};

cons_t uart0_cons = {
  .init = pl011_init,
  .getc = pl011_getc,
  .putc = pl011_putc,
  .flush = pl011_flush,
};
