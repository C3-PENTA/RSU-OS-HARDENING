#include <asm.h>

.section .rawtext
	.byte 0x1 
	.byte 0x1 
	.byte 0x1 
	.byte 0x1 
.global __uboot_start
__uboot_start:
	.incbin __uboot_file_path__
.global __uboot_end
__uboot_end:
	.byte 0x0 
	.byte 0x1 
	.byte 0x1 
	.byte 0x1 
	.byte 0x1 
