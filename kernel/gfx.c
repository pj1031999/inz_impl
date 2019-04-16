#include <rpi/vc_mbox.h>
#include <gfx.h>
#include <font.h>
#include <klibc.h>

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t buffer_width;
  uint32_t buffer_height;
  uint32_t pitch;
  uint32_t bit_depth;
  uint32_t x;
  uint32_t y;
  void *buffer;
  uint32_t buffer_size;
} GFX_INIT_REQUEST __aligned(16);

static GFX_INIT_REQUEST init_request;

window_t screen;
static volatile unsigned int mailbuffer[256] __attribute__((aligned (16)));


window_t *gfx_set_videomode(unsigned w, unsigned h) {
  init_request.width = init_request.buffer_width = w;
  init_request.height = init_request.buffer_height = h;
  init_request.buffer = 0;
  init_request.pitch = init_request.x = init_request.y =
      init_request.buffer_size = 0;
  init_request.bit_depth = 32;

#ifdef AARCH64
  /* Initialise the framebuffer */
  unsigned int var;
  unsigned int count;
  //unsigned int physical_screenbase;

  /* Storage space for the buffer used to pass information between the
   * CPU and VideoCore
   * Needs to be aligned to 16 bytes as the bottom 4 bits of the address
   * passed to VideoCore are used for the mailbox number
   */

  /* Physical memory address of the mailbuffer, for passing to VC */
  unsigned long physical_mb = (unsigned long)mailbuffer;

  /* /\* Get the display size *\/ */
  /* mailbuffer[0] = 8 * 4;		// Total size */
  /* mailbuffer[1] = 0;		// Request */
  /* mailbuffer[2] = 0x40003;	// Display size */
  /* mailbuffer[3] = 8;		// Buffer size */
  /* mailbuffer[4] = 0;		// Request size */
  /* mailbuffer[5] = 0;		// Space for horizontal resolution */
  /* mailbuffer[6] = 0;		// Space for vertical resolution */
  /* mailbuffer[7] = 0;		// End tag */

  /* writemailbox(8, physical_mb); */

  /* var = readmailbox(8); */

  /* /\* Valid response in data structure *\/ */
  /* if(mailbuffer[1] != 0x80000000) */
  /* 	fb_fail(FBFAIL_GET_RESOLUTION);	 */

  /* fb_x = mailbuffer[5]; */
  /* fb_y = mailbuffer[6]; */

  /* /\* If both fb_x and fb_y are both zero, assume we're running on the */
  /*  * qemu Raspberry Pi emulation (which doesn't return a screen size */
  /*  * at this point), and request a 640x480 screen */
  /*  *\/ */
  /* if(fb_x==0 && fb_y==0) */
  /* { */
  /* 	fb_x = 640; */
  /* 	fb_y = 480; */
  /* } */

  /* if(fb_x==0 || fb_y==0) */
  /* 	fb_fail(FBFAIL_GOT_INVALID_RESOLUTION); */


  /* Set up screen */

  unsigned int c = 1;
  mailbuffer[c++] = 0;		// Request

  mailbuffer[c++] = 0x00048003;	// Tag id (set physical size)
  mailbuffer[c++] = 8;		// Value buffer size (bytes)
  mailbuffer[c++] = 8;		// Req. + value length (bytes)
  mailbuffer[c++] = w;		// Horizontal resolution
  mailbuffer[c++] = h;		// Vertical resolution

  mailbuffer[c++] = 0x00048004;	// Tag id (set virtual size)
  mailbuffer[c++] = 8;		// Value buffer size (bytes)
  mailbuffer[c++] = 8;		// Req. + value length (bytes)
  mailbuffer[c++] = w;		// Horizontal resolution
  mailbuffer[c++] = h;		// Vertical resolution

  mailbuffer[c++] = 0x00048005;	// Tag id (set depth)
  mailbuffer[c++] = 4;		// Value buffer size (bytes)
  mailbuffer[c++] = 4;		// Req. + value length (bytes)
  mailbuffer[c++] = 32;		// 16 bpp

  mailbuffer[c++] = 0x00040001;	// Tag id (allocate framebuffer)
  mailbuffer[c++] = 8;		// Value buffer size (bytes)
  mailbuffer[c++] = 4;		// Req. + value length (bytes)
  mailbuffer[c++] = 16;		// Alignment = 16
  mailbuffer[c++] = 0;		// Space for response

  mailbuffer[c++] = 0;		// Terminating tag

  mailbuffer[0] = c*4;		// Buffer size

  //writemailbox(8, physical_mb);
  //var = readmailbox(8);

  vc_mbox_send(physical_mb, 8);
  var = vc_mbox_recv(8);

	

  /* Valid response in data structure */
  if(mailbuffer[1] != 0x80000000)
    return 0; //fb_fail(FBFAIL_SETUP_FRAMEBUFFER);	

  count=2;	/* First tag */
  while((var = mailbuffer[count]))
    {
      if(var == 0x40001)
	break;

      /* Skip to next tag
       * Advance count by 1 (tag) + 2 (buffer size/value size)
       *                          + specified buffer size
       */
      count += 3+(mailbuffer[count+1]>>2);

      if(count>c)
	return 0;//fb_fail(FBFAIL_INVALID_TAGS);
    }

  /* 8 bytes, plus MSB set to indicate a response */
  if(mailbuffer[count+2] != 0x80000008)
    return 0;//fb_fail(FBFAIL_INVALID_TAG_RESPONSE);

  /* Framebuffer address/size in response */
  //physical_screenbase = mailbuffer[count+3];
  //screensize = mailbuffer[count+4];

  //if(physical_screenbase == 0 || screensize == 0)
  //  return 0;//fb_fail(FBFAIL_INVALID_TAG_DATA);

  /* physical_screenbase is the address of the screen in RAM
   * screenbase needs to be the screen address in virtual memory
   */
  //screenbase=mem_p2v(physical_screenbase);

  /* /\* Get the framebuffer pitch (bytes per line) *\/ */
  /* mailbuffer[0] = 7 * 4;		// Total size */
  /* mailbuffer[1] = 0;		// Request */
  /* mailbuffer[2] = 0x40008;	// Display size */
  /* mailbuffer[3] = 4;		// Buffer size */
  /* mailbuffer[4] = 0;		// Request size */
  /* mailbuffer[5] = 0;		// Space for pitch */
  /* mailbuffer[6] = 0;		// End tag */

  /* writemailbox(8, physical_mb); */

  /* var = readmailbox(8); */

  /* /\* 4 bytes, plus MSB set to indicate a response *\/ */
  /* if(mailbuffer[4] != 0x80000004) */
  /*   fb_fail(FBFAIL_INVALID_PITCH_RESPONSE); */

  /* pitch = mailbuffer[5]; */
  /* if(pitch == 0) */
  /*   fb_fail(FBFAIL_INVALID_PITCH_DATA); */

  /* /\* Need to set up max_x/max_y before using console_write *\/ */
  /* max_x = fb_x / CHARSIZE_X; */
  /* max_y = fb_y / CHARSIZE_Y; */

  /* console_write(COLOUR_PUSH BG_BLUE BG_HALF FG_CYAN */
  /* 		"Framebuffer initialised. Address = 0x"); */
  /* console_write(tohex(physical_screenbase, sizeof(physical_screenbase))); */
  /* console_write(" (physical), 0x"); */
  /* console_write(tohex(screenbase, sizeof(screenbase))); */
  /* console_write(" (virtual), size = 0x"); */
  /* console_write(tohex(screensize, sizeof(screensize))); */
  /* console_write(", resolution = "); */
  /* console_write(todec(fb_x, 0)); */
  /* console_write("x"); */
  /* console_write(todec(fb_y, 0)); */
  /* console_write(COLOUR_POP "\n"); */
  
#else
  vc_mbox_send((unsigned long)&init_request, VC_CHAN_FB);
  uint32_t reply = vc_mbox_recv(VC_CHAN_FB);

  if (reply != 0)
    return 0;
#endif
  
  screen = (window_t){
    .x = 0,
    .y = 0,
    .width = w,
    .height = h,
    .stride = w,
    .fg_col = color(255,255,255),
    .bg_col = color(0,0,0),

#ifdef AARCH64
    .pixels = (color_t*)(long)(mailbuffer[count+3])
#else
    .pixels = init_request.buffer
#endif

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
