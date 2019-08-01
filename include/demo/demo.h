#ifndef DEMO_H
#define DEMO_H

#include <klibc.h>
#include <pmman.h>
#include <pmap.h>
#include <pcpu.h>

void demo_clock();
void demo_pmap();
void demo_uart();
void demo_led();
void demo_gpio();

extern vaddr_t _brk_limit;

static vaddr_t
vm_alloc(size_t size){
  static vaddr_t next_addr = (vaddr_t)&_brk_limit;
  next_addr += size;
  return next_addr;
}

static vaddr_t __unused
pages_alloc(size_t pages, flags_t flags){
  vaddr_t vaddr = vm_alloc(pages*PAGESIZE);
  paddr_t paddr = pm_alloc(pages*PAGESIZE);
  pmap_kenter(vaddr, paddr, flags);

  return vaddr;
}


#endif /*DEMO_H*/
