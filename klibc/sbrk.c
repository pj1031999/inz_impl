#include <klibc.h>

/* The end of the kernel's .bss section. Provided by the linker. */
extern uint8_t _bss_end[];
/* Limit for the end of kernel's bss. Provided by the linker. */
extern uint8_t _brk_limit[];

static struct {
  uint8_t *ptr;
  uint8_t *end;
} state = {_bss_end, _brk_limit};

void *sbrk(size_t size) {
  uint8_t *ptr = state.ptr;
  size = roundup(size, sizeof(uint64_t));
  assert(ptr + size <= state.end);
  state.ptr += size;
  bzero(ptr, size);
  return ptr;
}
