#include <bitstring.h>
#include <queue.h>
#include <klibc.h>
#include <pmman.h>

typedef struct pm_segment {
  TAILQ_ENTRY(pm_segment) link;
  paddr_t start;
  paddr_t end;
  unsigned npages;
  bitstr_t bitmap[0];
} pm_segment_t;

static TAILQ_HEAD(, pm_segment) seglist;

void pm_init(void) {
  TAILQ_INIT(&seglist);
}

void pm_add_segment(paddr_t start, paddr_t end) {
  start = rounddown2(start, PAGESIZE);
  end = roundup2(end, PAGESIZE);

  assert(start < end);

  unsigned npages = (end - start) / PAGESIZE;
  unsigned veclen = bitstr_size(npages);

  pm_segment_t *seg = sbrk(sizeof(pm_segment_t) + veclen);

  seg->start = start;
  seg->end = end;
  seg->npages = npages;
  bzero(seg->bitmap, veclen);

  TAILQ_INSERT_TAIL(&seglist, seg, link);
}

static bool bit_all_zero(bitstr_t *bitstr, unsigned first, unsigned last) {
  for (unsigned bit = first; bit <= last; bit++)
    if (bit_test(bitstr, bit))
      return false;
  return true;
}

static bool pm_reserve_from(pm_segment_t *seg, paddr_t start, paddr_t end) {
  if (start < seg->start)
    return false;
  if (end > seg->end)
    return false;

  assert(start < end);

  unsigned first = (start - seg->start) / PAGESIZE;
  unsigned last = (end - seg->start) / PAGESIZE - 1;

  if (!bit_all_zero(seg->bitmap, first, last))
    return false;

  bit_nset(seg->bitmap, first, last);
  return true;
}

void pm_reserve(paddr_t start, paddr_t end) {
  pm_segment_t *seg;
  TAILQ_FOREACH (seg, &seglist, link) {
    if (pm_reserve_from(seg, start, end))
      return;
  }

  panic("%s: no segment found!", __func__);
}

static int bit_find_nclear(bitstr_t *bitstr, unsigned nbits, unsigned n) {
  unsigned count = 0;
  unsigned bit = 0;

  while (bit < nbits && count < n) {
    if (bit_test(bitstr, bit)) {
      count = 0;
    } else {
      count++;
    }
    bit++;
  }

  if (count < n)
    return -1;

  return bit - count;
}

paddr_t pm_alloc(unsigned size) {
  unsigned npages = roundup2(size, PAGESIZE) / PAGESIZE;

  pm_segment_t *seg;
  TAILQ_FOREACH (seg, &seglist, link) {
    int idx = bit_find_nclear(seg->bitmap, seg->npages, npages);
    if (idx >= 0) {
      bit_nset(seg->bitmap, idx, idx + npages);
      return seg->start + idx * PAGESIZE;
    }
  }

  return -1;
}
