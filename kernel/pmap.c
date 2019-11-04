#include <pmap.h>
#include <pmman.h>
#include <klibc.h>
#include <aarch64/cpureg.h>
#include <aarch64/aarch64reg.h>
#include <aarch64/pte.h>
#include <aarch64/mmu.h>
#include <aarch64/cpu.h>

#define IS_VALID(a) ((a) >= 0)
#define IS_LAST_LVL(a) ((a) == 3 || (a) == -3)

static pt_lvl_t get_pte(vaddr_t va, pt_entry_t **res_entry) {
  uint64_t kernel = (uint64_t)_kernel;

  // virtual adresses for kernel starts witf 0xfff..
  bool kspace = !!(va & kernel);

  // clear upper bits
  va = va & ~kernel;

  uint32_t l1_idx = pmap_l1_index(va);
  uint32_t l2_idx = pmap_l2_index(va);
  uint32_t l3_idx = pmap_l3_index(va);

  pt_entry_t *l1, *l2, *l3, desc;

  // read first level page table entry from register ttbr0 or ttbr1 according to
  // upper va bits
  l1 = &((pt_entry_t *)(kspace ? kernel | reg_ttbr1_el1_read()
                               : reg_ttbr0_el1_read()))[l1_idx];
  desc = *l1 & ATTR_DESCR_MASK;

  // if entry describes block then return
  if (desc == L1_BLOCK) {
    *res_entry = l1;
    return 1;
  }

  // if entry does not describe block nor table then return error
  if (desc != L1_TABLE) {
    *res_entry = NULL;
    return -1; // error at level 1
  }

  l2 = &((pt_entry_t *)((*l1 & ~ATTR_MASK) | (kspace ? kernel : 0)))[l2_idx];
  desc = *l2 & ATTR_DESCR_MASK;

  if (desc == L2_BLOCK) {
    *res_entry = l2;
    return 2;
  }

  if (desc != L2_TABLE) {
    *res_entry = NULL;
    return -2; // error at level 2
  }

  l3 = &((pt_entry_t *)((*l2 & ~ATTR_MASK) | (kspace ? kernel : 0)))[l3_idx];
  desc = *l3 & ATTR_DESCR_MASK;

  if (desc != L3_PAGE) {
    //*res_entry = NULL;
    *res_entry = l3;
    return -3; // error at level 3
  }

  *res_entry = l3;
  return 3;
}

static inline void table_invalidate_entry(vaddr_t va) {
  __asm__ __volatile__("LSR %[vaddr], %[vaddr], #12 \n"
                       "TLBI vmalle1 \n"
                       "TLBI VAAE1, %[vaddr] \n"
                       "ISB \n"
                       :
                       : [ vaddr ] "r"(va)
                       : "memory");
}

void pmap_kenter(vaddr_t va, paddr_t pa, flags_t flags) {

  assert(pa % PAGESIZE == 0);
  assert(va % PAGESIZE == 0);

  pt_entry_t *entry = NULL;
  pt_lvl_t res = get_pte(va, &entry);

  if (IS_VALID(res) || IS_LAST_LVL(res)) {
    *entry =
      // clear entry access permission and execute never bits
      //(*entry & ~L3_PAGE_OA & ~ATTR_AP_MASK & ~ATTR_XN)
      (flags & ~L3_PAGE_OA) | (pa & L3_PAGE_OA);
  }

  table_invalidate_entry(va);
}

void pmap_kremove(vaddr_t va, size_t size) {
  assert(size % PAGESIZE == 0);
  int pages = size / PAGESIZE;

  pt_entry_t *entry = NULL;
  while (pages--) {
    if (IS_VALID(get_pte(va, &entry)))
      *entry = 0x0;

    table_invalidate_entry(va);
    va += PAGESIZE;
  }
}

bool pmap_kextract(vaddr_t va, paddr_t *pa_p) {
  pt_entry_t *entry = NULL;
  pt_lvl_t lvl = get_pte(va, &entry);

  if (!IS_VALID(lvl))
    return false;

  /* entry is valid */
  switch (lvl) {
    case 1:
      assert((*entry & ATTR_DESCR_MASK) == L1_BLOCK);
      *pa_p = (paddr_t)((*entry & L1_BLOCK_OA) | (va & L1_OFFSET));
      return true;

    case 2:
      assert((*entry & ATTR_DESCR_MASK) == L2_BLOCK);
      *pa_p = (paddr_t)((*entry & L2_BLOCK_OA) | (va & L2_OFFSET));
      return true;

    case 3:
      assert((*entry & ATTR_DESCR_MASK) == L3_PAGE);
      *pa_p = (paddr_t)((*entry & L3_PAGE_OA) | (va & L3_OFFSET));
      return true;

    default:
      return false;
  }
}

bool pmap_is_referenced(vaddr_t va) {
  pt_entry_t *entry = NULL;
  if (IS_VALID(get_pte(va, &entry)))
    return *entry & ATTR_AF;

  return false;
}

bool pmap_is_modified(vaddr_t va) {
  pt_entry_t *entry = NULL;
  if (IS_VALID(get_pte(va, &entry)))
    return *entry & ATTR_DBM;

  return false;
}

void pmap_clear_referenced(vaddr_t va) {
  pt_entry_t *entry = NULL;
  if (IS_VALID(get_pte(va, &entry)))
    *entry &= ~ATTR_AF;

  table_invalidate_entry(va);
}

void pmap_clear_modified(vaddr_t va) {
  pt_entry_t *entry = NULL;
  if (IS_VALID(get_pte(va, &entry)))
    *entry &= ~ATTR_DBM;

  table_invalidate_entry(va);
}

void pmap_data_abort_access_fault(vaddr_t va) {
  pt_entry_t *entry = NULL;
  if (IS_VALID(get_pte(va, &entry)))
    *entry = *entry | ATTR_AF;

  table_invalidate_entry(va);
}

void pmap_data_abort_modify_fault(vaddr_t va) {
  pt_entry_t *entry = NULL;
  if (IS_VALID(get_pte(va, &entry))) {
    if (*entry & ATTR_SW_RW)
      *entry = *entry & ~FLAG_MEM_RO;

    *entry = *entry | ATTR_DBM;
  }

  table_invalidate_entry(va);
}

extern pte_t *_level1_pagetable;
extern vaddr_t *_brk_limit;

void pmap_setup_kernel_space() {
  kernel_space.pt_root = _level1_pagetable;
  kernel_space.start = (vaddr_t)_kernel;
  kernel_space.end = (vaddr_t)&_brk_limit;
  kernel_space.asid = 0;
}

bool load_word(vaddr_t addr, uint64_t *res) {
  *res = *(uint64_t *)addr;
  return true;
}

bool store_word(vaddr_t addr, uint64_t val) {
  *(uint64_t *)addr = val;
  return true;
}
