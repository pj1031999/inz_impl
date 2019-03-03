.section ".evec"

.globl _exc_vector

//@ Exception vector.

_exc_vector:
	nop	//@ reset			//@ 0x00
	b	exc_undefined_instruction	//@ 0x04
	b	exc_supervisor_call		//@ 0x08
	b	exc_prefetch_abort		//@ 0x0C
	b	exc_data_abort    		//@ 0x10
	nop	//@ reserved
	b	exc_irq				//@ 0x18
	b	exc_fast_irq			//@ 0x1C


//@ vim: ft=armv5 ts=8 sw=8 noet
