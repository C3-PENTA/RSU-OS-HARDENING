# //+[TCCQB] Memory Data Copy for QuickBoot Image Loading
#include <asm.h>

.text
/*****************************************************************************
;
; void tcc_memcpy(void *dest, void *src);
;
******************************************************************************/
FUNCTION(tcc_memcpy)
		stmdb	sp!, {r2-r12,r14}

/* 0 */
		ldmia	r1!, {r2-r12,r14}		/* read 12*4 bytes */
		stmia	r0!, {r2-r12,r14}		/* write 12*4 bytes */

		ldmia	r1!, {r2-r12,r14}		
		stmia	r0!, {r2-r12,r14}		

		ldmia	r1!, {r2-r12,r14}		
		stmia	r0!, {r2-r12,r14}		

		ldmia	r1!, {r2-r12,r14}		
		stmia	r0!, {r2-r12,r14}		

		ldmia	r1!, {r2-r12,r14}		
		stmia	r0!, {r2-r12,r14}		

		ldmia	r1!, {r2-r12,r14}		
		stmia	r0!, {r2-r12,r14}		

		ldmia	r1!, {r2-r12,r14}		
		stmia	r0!, {r2-r12,r14}		

		ldmia	r1!, {r2-r12,r14}		
		stmia	r0!, {r2-r12,r14}		

		ldmia	r1!, {r2-r12,r14}		
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

/* 10 */
		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

/* 20 */
		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

/* 30 */
		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

/* 40 */
		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

/* 50 */
		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

/* 60 */
		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

/* 70 */
		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

/* 80 */
		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

		ldmia	r1!, {r2-r12,r14}
		stmia	r0!, {r2-r12,r14}

/* 85 = 48 * 85 = 4080
 * 4096 - 4080 = 16 bytes
 */
		ldmia	r1!, {r2-r5}		/* read 4*4 bytes */
		stmia	r0!, {r2-r5}		/* write 4*4 bytes */

		ldmia	sp!, {r2-r12, pc}

		bx lr
# //-[TCCQB]
# //
