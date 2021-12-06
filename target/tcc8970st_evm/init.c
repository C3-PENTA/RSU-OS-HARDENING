/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <debug.h>
#include <platform/iomap.h>
#include <platform/gpio.h>
#include <reg.h>
#include <target.h>
#include <platform.h>
#include <mmc.h>
#include <spmi.h>
#include <board.h>
#include <smem.h>
#include <baseband.h>
#include <dev/keys.h>
#include <partition_parser.h>
#include <scm.h>
#include <platform/tcc_ckc.h>
#include <stdlib.h>
#include <target/guid_partition.h>
#include <sdmmc/emmc.h>
#include <i2c.h>
#include <plat/cpu.h>

#if defined(START_FWDN_BY_REMOTE_KEY)
#include <tcc_remocon.h>
#endif

#define LINUX_MACHTYPE  5015

extern  bool target_use_signed_kernel(void);
extern unsigned boot_into_chrome;
static void set_sdc_power_ctrl();

static unsigned int target_id;
static uint32_t pmic_ver;

extern unsigned boot_into_qb_prebuilt;
extern void edi_init( void );

void target_early_init(void)
{
	if (cpu_is_tcc8975()) {
		#if defined(TARGET_TCC8975_EVM)
		i2c_init(I2C_CH_MASTER0, 9);
		#endif
	}
	else {
		i2c_init(I2C_CH_MASTER0, 15);
	}
}

void target_init(void)
{
	uint32_t base_addr;
	uint8_t slot;

	dprintf(INFO, "target_init()\n");

	if (cpu_is_tcc8975()) {
		#if defined(TARGET_TCC8975_EVM)
		pca953x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 0x0000FFFF, 0x0000FFFF);
		pca953x_init(GPIO_PORTEXT2, I2C_CH_MASTER0, 3, 0x0000FFFF, 0x0000FFFF);
		#endif
	}

	keys_init();
	#if defined(TARGET_TCC8975_EVM)
	keypad_init();
	#endif

	if (target_is_emmc_boot()) {
#if _EMMC_BOOT
	    emmc_boot_main();       // emmc boot
	    if (check_fwdn_mode() || keys_get_state(KEY_MENU) != 0)
		goto fwdn_start;
#else
	    dprintf(INFO, "emmc boot not supported\n");
#endif
	}else {
	    edi_init();
	    if (check_fwdn_mode() || keys_get_state(KEY_MENU) != 0)
		goto fwdn_start;

	    flash_boot_main();              // nand boot
	}

#if DISPLAY_SPLASH_SCREEN
	dprintf(INFO, "Display Init: Start\n");
	display_init();
	dprintf(INFO, "Display Init: Done\n");
#endif
#ifdef START_FWDN_BY_REMOTE_KEY
	if(getRemoteKey() == SCAN_NUM_0)
		goto fwdn_start;
#endif
	return;

fwdn_start:
	    fwdn_start();

}

unsigned board_machtype(void)
{
	target_id = LINUX_MACHTYPE;
	return target_id;
}

void reboot_device(unsigned reboot_reason)
{
	        reboot(reboot_reason);
}

static int board_get_serialno(char *serialno)
{
	int n,i;
	char temp[32];
#if _EMMC_BOOT
	if (target_is_emmc_boot())
		n = get_emmc_serial(temp);
	else
#endif
		n = NAND_GetSerialNumber(temp, 32);

	for (i=0; i<4; i++)	// 4 = custon field(2) + product number(2)
		*serialno++ = temp[i];
	for (i=16; i<32; i++)	// 16 = time(12) + serial count(4)
		*serialno++ = temp[i];
	*serialno = '\0';
	return strlen(serialno);
}

static int board_get_wifimac(char *wifimac)
{
	int n,i;
	char temp[32];
#if _EMMC_BOOT
	if (target_is_emmc_boot())
		n = get_emmc_serial(temp);
	else
#endif
		n = NAND_GetSerialNumber(temp, 32);

	if (temp[1] == '1') {
		for (i=0; i<12; i++) {
			*wifimac++ = temp[4+i];
			if (i==11) break;
			if (!((i+1)%2)) *wifimac++ = ':';
		}
	} else if(temp[1] == '2') {
		for (i=0; i<12; i++) {
			*wifimac++ = temp[16+i];
			if (i==11) break;
			if (!((i+1)%2)) *wifimac++ = ':';
		}
	}
	*wifimac = '\0';
	return strlen(wifimac);
}

static int board_get_btaddr(char *btaddr)
{
	int n,i;
	char temp[32];
#if _EMMC_BOOT
	if (target_is_emmc_boot())
		n = get_emmc_serial(temp);
	else
#endif
		n = NAND_GetSerialNumber(temp, 32);

	for (i=4; i<16; i++)	// 12 = bluetooth bd address field(12)
		*btaddr++ = temp[i];
	*btaddr = '\0';
	return strlen(btaddr);
}

void target_cmdline_bootmode(char *cmdline)
{
	extern unsigned boot_into_factory;
	char s[32] = "";
	char s2[32] = "";

#if (0)
	if (target_is_battery_charging())
		strcpy(s, "charger");
	else
#endif
	if (target_is_emmc_boot())
		strcpy(s, "emmc");
	else
		strcpy(s, "nand");

	strcat(cmdline, " androidboot.mode=");
	strcat(cmdline, s);

	if (boot_into_factory ) {
		strcat(s2, "tcc_factory");
	} 
	strcat(cmdline, " androidboot.dignostic=");
	strcat(cmdline, s2);
}

void target_cmdline_serialno(char *cmdline)
{
	char s[128];
	char s2[64];

	board_get_serialno(s2);


#if _EMMC_BOOT
	if(boot_into_chrome)
		sprintf(s, " root=/dev/mmcblk0p3 rw rootfstype=ext2 rootwait noinitrd", s2);
	else
		sprintf(s, " androidboot.serialno=%s", s2);
#else
	sprintf(s, " androidboot.serialno=%s", s2);
#endif

	strcat(cmdline, s);
}

void target_cmdline_wifimac(char *cmdline)
{
	char s[64];
	char s2[32];

	board_get_wifimac(s2);

	sprintf(s, " androidboot.wifimac=%s", s2);
	strcat(cmdline, s);
}

void target_cmdline_btaddr(char *cmdline)
{
	char s[64];
	char s2[32];

	board_get_btaddr(s2);

	sprintf(s, " androidboot.btaddr=%s", s2);
	strcat(cmdline, s);
}

void target_cmdline_memtype(char *cmdline)
{
	char s[64];
	char memtype[2] = {""};
#if defined(DRAM_DDR2)
	char temp = '1';
#elif defined(DRAM_DDR3)
	char temp = '2';
#elif ! defined(TCC88XX)
	char temp = '1';
#else
	char temp = '0';
#endif
	memtype[0] = temp;
	memtype[1] = '\0';

	sprintf(s, " androidboot.memtype=%s", memtype);
	strcat(cmdline, s);
}

void target_cmdline_vmalloc(char *cmdline)
{
	char s[64];
	sprintf(s, " androidboot.hardware=tcc8970st vmalloc=%s", "480M");
	strcat(cmdline,s);
}
