#include "aarch64/aarch64reg.h"
#include "aarch64/pte.h"
#include "dev/bcm2836reg.h"

#define STACK_START_SIZE 0x38

.section ".init"

.global _start
_start:
    /* --- save atags register
     * x27 is callee-saved */
    MOV  x27, x0
    /* --- get CPU number */
    MRS  x3, MPIDR_EL1
    AND  x3, x3, #3
    
    /* --- let's add 1 to the CPU number */
    ADD  x3, x3, #0x1
    /* we can use free space between 0x0 and 0x100 as temporary stack
     * 0x100 is used for atags
     * and SP must be aligned to 8
     * so now we have 56 bytes for stack for each CPU
     * 0x38 for CPU0
     * 0x70 for CPU1
     * 0xa8 for CPU2
     * 0xe0 for CPU3 */
    MOV  x4, STACK_START_SIZE
    MUL  x4, x3, x4
    
    /* --- but we need additional bytes for CPU0
     * let's move all the stacks by 24 bytes 
     * 0x50 for CPU0
     * 0x88 for CPU1
     * 0xc0 for CPU2
     * 0xf8 for CPU3 */
    ADD  x4, x4, 0x18
    MOV  SP, x4

    BL   arm64_init

    BL   platform_stack
    MOV  SP, x0
    /* --- now we have stack */

    /* --- restore atags register and go out of assembler's hell */
    MOV  x0, x27
    BL   platform_init 
