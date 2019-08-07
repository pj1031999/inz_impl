#include <demo/userspace_demo.h>
#include <types.h>


static void write_number(int64_t num){
  __asm__ __volatile__(// write number
		       "mov x0, %[num] \n"
		       "svc #0x2"
		       :: [num] "r" (num) );
}
static int64_t __unused read_number(void){
  int64_t num = 0;
  __asm__ __volatile__( // read number
  		       "svc #0x1 \n"
  		       "mov %[num], x0"
  		       :[num] "=r" (num) );
  return num;
}

static int double_it(int a){
  if(a == 0){ // try access kernelspace address -> panic
    extern uint64_t _level1_pagetable[];
    uint64_t e = _level1_pagetable[0];
    write_number(e);
  }

  int b = a*2;
  return b;
}


static void __attribute__((section(".us_text")))
us_program(){
  
  int64_t res = 2;
  res = read_number();
  res = double_it(res);
  write_number(res);

  return;
}

void __attribute__((section(".us_text")))
us_program_entry(){
  //save link register on stack
  __asm__ __volatile__("str lr, [sp, #-0x10]! \n");
  us_program();

  //read saved link register, then exit to kernel
  __asm__ __volatile__("ldr lr, [sp], #0x10\n"
		       "svc #0x0"); //exit
}
