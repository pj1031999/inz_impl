/* clock.c - System timer */

#include <aarch64/cpu.h>
#include <klibc.h>
#include <rpi/irq.h>
#include <pmap.h>
#include <pmman.h>

#define CLK_FREQ 192000000

static uint32_t ticks = 0;
static uint64_t clk_freq = 0;

extern uint64_t *_kernel;

#define TEST 1
static void pmap_test(){
#if TEST == 0
  paddr_t alloc_p = pm_alloc(PAGESIZE);
  vaddr_t *alloc_v = (vaddr_t*)((uint64_t)&_kernel | (uint64_t)alloc_p);
  *alloc_v = 0xcafebabe; 
  printf("\t alloc_v = %p \t *alloc_v = %x \n", alloc_v, *alloc_v);
  pmap_kremove((vaddr_t)alloc_v, PAGESIZE);
  printf("\t alloc_v = %p \t *alloc_v = %x \t", alloc_v, *alloc_v);

#elif TEST == 1
  paddr_t alloc1_p = pm_alloc(PAGESIZE);
  vaddr_t *alloc1_v = (vaddr_t*)((uint64_t)&_kernel | (uint64_t)alloc1_p);
  *alloc1_v = 0x1; 

  paddr_t alloc2_p = pm_alloc(PAGESIZE);
  vaddr_t *alloc2_v = (vaddr_t*)((uint64_t)&_kernel | (uint64_t)alloc2_p);
  *alloc2_v = 0x2; 

  paddr_t pa1 = 0, pa2 = 0;
  pmap_kextract((vaddr_t)alloc1_v, &pa1);
  pmap_kextract((vaddr_t)alloc2_v, &pa2);
  printf("\t alloc1_v = %p \t *alloc1_v = %x \t phys = %p \n", alloc1_v, *alloc1_v, pa1);
  printf("\t alloc2_v = %p \t *alloc2_v = %x \t phys = %p \n", alloc2_v, *alloc2_v, pa2);

  pmap_kenter((vaddr_t)alloc1_v, alloc2_p, (flags_t)NULL);
  //pmap_kenter((vaddr_t)alloc2_v, alloc1_p, (flags_t)NULL);
  
  pmap_kextract((vaddr_t)alloc1_v, &pa1);
  pmap_kextract((vaddr_t)alloc2_v, &pa2);
  printf("\t alloc1_v = %p \t *alloc1_v = %x \t phys = %p \n", alloc1_v, *alloc1_v, pa1);
  printf("\t alloc2_v = %p \t *alloc2_v = %x \t phys = %p \n", alloc2_v, *alloc2_v, pa2);

  
#elif TEST == 2
  vaddr_t va = (ticks-1) * 0x000010000 + 0xffffFFFF00054321;
  paddr_t pa = 0;

  if(ticks % 2 == 0)
    pa = *((uint64_t*)va);

  if(ticks % 8 == 0){
    pmap_clear_referenced(va);
    pmap_clear_modified(va);
  }

  //pmap_kremove(va, PAGESIZE);

  pmap_kextract(va, &pa);
  printf("\t %.16p -> %.8p af=%d db=%d \t", va, pa, pmap_is_referenced(va), pmap_is_modified(va));

#endif
}

static void clock_irq(unsigned irq __unused) {
  uint64_t val = reg_cntp_cval_el0_read();
  reg_cntp_cval_el0_write(val + clk_freq);
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

  arm_isb();
  ticks++;

  pmap_test();
  printf("tick %d!\n", ticks);
}

void clock_init(void) {

  //reg_cntvct_el0_write(CLK_FREQ);
  clk_freq = reg_cntfrq_el0_read();
  reg_cntp_cval_el0_write(clk_freq );
  reg_cntp_ctl_el0_write(CNTCTL_ENABLE);

  arm_isb();

  /* Enable CP0 physical timer interrupt. */
  bcm2836_local_irq_register(BCM2836_INT_CNTPSIRQ, clock_irq);
  bcm2836_local_irq_enable(BCM2836_INT_CNTPSIRQ);

  bcm2836_local_irq_register(BCM2836_INT_CNTPNSIRQ, clock_irq);
  bcm2836_local_irq_enable(BCM2836_INT_CNTPNSIRQ);

}


