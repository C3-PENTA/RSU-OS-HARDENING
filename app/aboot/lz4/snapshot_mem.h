//+[TCCQB] QB memory map
/*=========================================================
 *
 * SNAPSHOT BOOT MEMORY 
 *
 * =======================================================*/

//==========================================
//
// TCC893X Definitions
//
//==========================================
void *TCC893X_CORE_MMU_DATA_ADDR	= (void *)0xF000F000;
int TCC893X_CORE_MMU_DATA_SIZE		= 0x00000100;
void *TCC893X_SNAPSHOT_BOOT_ADDR	= (void *)0xF000F100;
int TCC893X_SNAPSHOT_BOOT_SIZE		= 0x00000400;
void *TCC893X_STACK_ADDR			= (void *)0xF000FFFC;

//==========================================
//
// TCC896X Definitions
//
//==========================================
void *TCC896X_CORE_MMU_DATA_ADDR	= (void *)0xF000A000;
int TCC896X_CORE_MMU_DATA_SIZE		= 0x00000100;
void *TCC896X_SNAPSHOT_BOOT_ADDR	= (void *)0xF000A100;
int TCC896X_SNAPSHOT_BOOT_SIZE		= 0x00000400;
void *TCC896X_STACK_ADDR			= (void *)0xF000FFFC;

//==========================================
//
// TCC897X Definitions
//
//==========================================
void *TCC897X_CORE_MMU_DATA_ADDR	= (void *)0x1000A000;
int TCC897X_CORE_MMU_DATA_SIZE		= 0x00000100;
void *TCC897X_SNAPSHOT_BOOT_ADDR	= (void *)0x1000A100;
int TCC897X_SNAPSHOT_BOOT_SIZE		= 0x00000400;
void *TCC897X_STACK_ADDR			= (void *)0xF000FFFC;

/*	 Use DRAM_MMU_SWITCH_ADDR Area defined in kernel/arch/arm/mach-tcc897x/include/mach/sram_map.h	*/
#if (TCC_MEM_SIZE > 1024)	// 2GB Memory
void *TCC897X_DRAM_KERNEL_JUMP_CODE	= (void *)0xEFFFFFC0;	// Using DRAM_MMU_SWITCH_ADDR Area
int TCC897X_DRAM_KERNEL_JUMP_SIZE	= 0x00000040;
#else	// 1GB Memory
void *TCC897X_DRAM_KERNEL_JUMP_CODE	= (void *)0xAFFFFFC0;	// Using DRAM_MMU_SWITCH_ADDR Area
int TCC897X_DRAM_KERNEL_JUMP_SIZE	= 0x00000040;
#endif
//-[TCCQB]
//
