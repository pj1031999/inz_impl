#include <cons.h>
#include <gfx.h>
#include <font.h>
#include <klibc.h>

struct cons_dev {
  window_t window;
  unsigned width, height;
  struct {
    unsigned x, y;
  } cursor;
  font_t *font;
};

static void gfx_cons_init(cons_dev_t *dev) {
  dev->width = dev->window.width >> 3;   /* / font->width; */
  dev->height = dev->window.height >> 4; /* / font->height; */
  dev->cursor.x = 0;
  dev->cursor.y = 0;
}

static void scroll_up(cons_dev_t *dev) {
  unsigned w = dev->window.width;
  unsigned h = dev->window.height;
  unsigned fh = dev->font->height;
  window_t *win = &dev->window;

  unsigned src_x = dev->window.x;
  unsigned src_y = dev->window.y + fh;

  unsigned dst_x = dev->window.x;
  unsigned dst_y = dev->window.y;

  dev->cursor.y--;
  gfx_rect_move(win, w, h - fh, src_x, src_y, dst_x, dst_y);
  gfx_rect_draw(win, 0, h - fh, w, fh, win->bg_col);
}

static void next_line(cons_dev_t *dev) {
  dev->cursor.x = 0;
  dev->cursor.y++;

  if (dev->cursor.y >= dev->height)
    scroll_up(dev);
}

static void next_char(cons_dev_t *dev) {
  dev->cursor.x++;
  if (dev->cursor.x >= dev->width)
    next_line(dev);
}

static void gfx_cons_putc(cons_dev_t *dev, int c) {
  switch (c) {
    case '\r':
      break;
    case '\n':
      next_line(dev);
      break;
    default:
      if (c < 32)
        return;
      gfx_put_char(&dev->window, dev->font, dev->cursor.x * dev->font->width,
                   dev->cursor.y * dev->font->height, c);
      next_char(dev);
      break;
  }
}

static int gfx_cons_getc(cons_dev_t *dev __unused) {
  extern cons_t uart0_cons;
  return uart0_cons.getc(dev);
  // return 0;
}

static void gfx_cons_flush(cons_dev_t *dev) {
  unsigned fw = dev->font->width;
  unsigned fh = dev->font->height;
  unsigned x = dev->cursor.x * fw;
  unsigned y = dev->cursor.y * fh;
  window_t *win = &dev->window;
  gfx_rect_draw(win, x, y, fw, fh, win->fg_col);
}

extern font_t Terminus16;
/* #include <aarch64/cpu.h> */
/* static cons_dev_t devs[4]; */
/* static cons_t conss[4]; */

cons_t *make_gfx_cons(window_t *win, font_t *font) {
  // cons_dev_t *dev = (void*)&devs[arm_cpu_id()];
  cons_dev_t *dev = sbrk(sizeof(cons_dev_t));
  dev->window = *win;
  dev->font = font ? font : &Terminus16;

  // cons_t *cons = (void*)&conss[arm_cpu_id()];
  cons_t *cons = sbrk(sizeof(cons_t));
  cons->dev = dev;
  cons->init = gfx_cons_init;
  cons->getc = gfx_cons_getc;
  cons->putc = gfx_cons_putc;
  cons->flush = gfx_cons_flush;

  return cons;
}
