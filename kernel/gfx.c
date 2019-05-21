#include <rpi/vc_mbox.h>
#include <gfx.h>
#include <mmu.h>
#include <font.h>
#include <klibc.h>

window_t screen;

window_t *gfx_set_videomode(unsigned w, unsigned h) {

  /* Initialise the framebuffer */
  /* Set up screen */
  const uint32_t c = set_mailbufer((unsigned int*)_mail_buffer, w, h);
  vc_mbox_send((uint64_t)_mail_buffer, VC_CHAN_ARM2VC);
  vc_mbox_recv(VC_CHAN_ARM2VC);

  /* Valid response in data structure */
  if(!vcprop_buffer_success_p((struct vcprop_buffer_hdr*)_mail_buffer))
    return 0; 

  struct vcprop_tag_allocbuf* alloc_buff
    = get_response_tag_allocbuf((const unsigned int*)_mail_buffer, c);
  
  /* 8 bytes, plus MSB set to indicate a response */
  if(alloc_buff->tag.vpt_rcode != 0x80000008)
    return 0;

  /*
   * Framebuffer address/size in response 
   * physical_screenbase is the address of the screen in RAM
   * screenbase needs to be the screen address in virtual memory
   */
  unsigned long long  physical_screenbase = alloc_buff->address;
  physical_screenbase &= ~0xc0000000;
  unsigned long long screenbase = mem_p2v(physical_screenbase);
  
  screen = (window_t){
    .x = 0,
    .y = 0,
    .width = w,
    .height = h,
    .stride = w,
    .fg_col = color(255,255,255),
    .bg_col = color(0,0,0),

    .pixels = (color_t*)(screenbase)
  };

  return &screen;
}

bool gfx_window(window_t *main, window_t *win,
                unsigned x, unsigned y, unsigned width, unsigned height) {
  *win = *main;

  win->x += x;
  win->y += y;
  win->width = width;
  win->height = height;

  if (win->x > main->x + main->width) {
    win->width = 0;
    return false;
  }

  if (win->y > main->y + main->height) {
    win->height = 0;
    return false;
  }

  if (win->x + win->width > main->x + main->width)
    win->width = main->x + main->width - win->x;

  if (win->y + win->height > main->y + main->height)
    win->height = main->x + main->height - win->x;

  return true;
}

void gfx_put_pixel(window_t *win, unsigned x, unsigned y, color_t color) {
  if (x >= win->width)
    return;
  if (y >= win->height)
    return;
  x += win->x;
  y += win->y;
  win->pixels[win->stride * y + x] = color;
}

uint32_t gfx_get_pixel(window_t *win, unsigned x, unsigned y) {
  if (x >= win->width)
    return 0;
  if (y >= win->height)
    return 0;
  x += win->x;
  y += win->y;
  return win->pixels[win->stride * y + x];
}

void gfx_rect_draw(window_t *win, unsigned x, unsigned y,
                   unsigned w, unsigned h, color_t color) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      gfx_put_pixel(win, x + i, y + j, color);
    }
  }
}

void gfx_rect_move(window_t *win, unsigned w, unsigned h,
                   unsigned sx, unsigned sy, unsigned dx, unsigned dy) {
  /* TODO clipping */
  if (sy > dy) {
    for (int y = 0; y < h; y++) {
      uint32_t *src = win->pixels + win->stride * (sy + y) + sx;
      uint32_t *dst = win->pixels + win->stride * (dy + y) + dx;
      memmove(dst, src, w * sizeof(uint32_t));
    }
  }
}

void gfx_put_char(window_t *win, font_t *font, unsigned x, unsigned y, int c) {
  font_char_t *fc = font->map;

  while (fc->code != FONT_CODE_LAST && fc->code != c)
    fc++;

  uint8_t *data = fc->data;
  for (int j = 0; j < font->height; j++) {
    uint8_t row = data[j];
    for (int i = 0; i < 8; i++) {
      uint32_t color = (row & __BIT(7 - i)) ? win->fg_col : win->bg_col;
      gfx_put_pixel(win, x + i, y + j, color);
    }
  }
}

struct vcprop_tag_allocbuf*
get_response_tag_allocbuf(const unsigned int mailbuffer[], const uint32_t c)
{

  unsigned int var;
  struct vcprop_tag* mb_tag = (struct vcprop_tag*)&mailbuffer[2]; /* First tag */
  
  while((var = mb_tag->vpt_tag)){
    if(var == VCPROPTAG_ALLOCATE_BUFFER)
      break;
    
    /* Skip to next tag*/
    {
      void* p = mb_tag;
      p = (uint8_t*)p + (sizeof(struct vcprop_tag) + vcprop_tag_resplen(mb_tag));
      mb_tag = p;
    }
    
    if(mb_tag >= (struct vcprop_tag*)&mailbuffer[c])
      return 0;
  }
  return (struct vcprop_tag_allocbuf*)mb_tag;
}


fill(vcprop_tag_fbdepth);
fill(vcprop_tag_fbres);
fill(vcprop_tag_allocbuf);
fill(vcprop_tag_request);
fill(vcprop_tag_terminate);

uint32_t
set_mailbufer( unsigned int mailbuffer[], unsigned w, unsigned h)
{
  
#define UNPACK(...) __VA_ARGS__
#define fill_with(vcprop_tag_name, tag_val)		\
  {							\
    struct vcprop_tag_name val = UNPACK tag_val;	\
    c += fill_##vcprop_tag_name(mailbuffer, c, val);	\
  }
  
  unsigned int c = 1;
  fill_with (vcprop_tag_request,   ( {VCPROPTAG_REQUEST}) );
  fill_with (vcprop_tag_fbres,	   ({{VCPROPTAG_SET_FB_RES,   8, 0}, w, h}) );
  fill_with (vcprop_tag_fbres,	   ({{VCPROPTAG_SET_FB_VRES,  8, 0}, w, h}) );
  fill_with (vcprop_tag_fbdepth,   ({{VCPROPTAG_SET_FB_DEPTH, 4, 0}, 32}) );
  fill_with (vcprop_tag_allocbuf,  ({{VCPROPTAG_ALLOCATE_BUFFER, 8, 0}, 16, 0}) );
  fill_with (vcprop_tag_terminate, ( {VCPROPTAG_END}) );
  
  mailbuffer[0] = c * sizeof(mailbuffer[0]);     	// Buffer size

  return c;
}

