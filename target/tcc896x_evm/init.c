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

#define LINUX_MACHTYPE  5012

extern  bool target_use_signed_kernel(void);
extern unsigned boot_into_chrome;
static void set_sdc_power_ctrl();

static unsigned int target_id;
static uint32_t pmic_ver;

extern unsigned boot_into_qb_prebuilt;

extern int check_fwdn_mode(void);
extern void fwdn_start(void);
extern void edi_init(void);

void target_early_init(void)
{
	i2c_init(I2C_CH_MASTER0, 8);
	i2c_init(I2C_CH_MASTER1, 21);
	i2c_init(I2C_CH_MASTER2, 15);
	i2c_init(I2C_CH_SMU, 0);
}

void target_init(void)
{
	uint32_t base_addr;
	uint8_t slot;

	dprintf(INFO, "target_init()\n");

#if (HW_REV == 0x1001)
	/*
	GPIO EXPANDER U3
	BANK0			BANK1    		BANK2			BANK3			BANK4
	0 - LCD_ON		0 - P-CAM4_PWR_ON	0 - CAS_RST#		0 - USB30_EN		0 - PWR_CTL0
	1 - LCD_RST#		1 - P-CAM0_PWR_ON	1 - CAS_VCC_SEL		1 - U20H0_EN		1 - PWR_CTL1
	2 - TVOUT_ON		2 - COMPASS_RST		2 - MUTE_CTL		2 - U20H1_EN		2 - x
	3 - GPS_PWREN		3 - IPOD_ON		3 - SDWF_GP		3 - USB30VBUS_EN	3 - x
	4 - CODEC_ON		4 - AUTH_RST#		4 - MAC_RXRSTN		4 - USB20H0VBUS_EN	4 - x
	5 - EXT_CODEC_ON	5 - TV_SLEEP#		5 - DRAMPWR0_ON		5 - USB20H1VBUS_EN	5 - x
	6 - BTWF_ON		6 - CAS_ON		6 - DRAMPWR1_ON		6 - HDMI_EN		6 - x
	7 - BT_WAKE		7 - CAS_GP		7 - V_5P0_EN		7 - LVDS_EN		7 - x
	*/

	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 0, 0x00, 0x03); // bank0 // direction IO0-0 ~ IO0-7
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 1, 0x00, 0x20); // bank1 // direction IO1-0 ~ IO1-7
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 2, 0x00, 0xE0); // bank2 // direction IO2-0 ~ IO2-7
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 3, 0x00, 0x89); // bank3 // direction IO3-0 ~ IO3-7
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 4, 0xFC, 0x00); // bank4 // direction IO4-0 ~ IO4-7
#elif (HW_REV == 0x1100)
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 0, 0x00, 0xFF); // bank0 // direction IO0-0 ~ IO0-7
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 1, 0x00, 0xFF); // bank1 // direction IO1-0 ~ IO1-7
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 2, 0x20, 0xBF); // bank2 // direction IO2-0 ~ IO2-7
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 3, 0x24, 0xDB); // bank3 // direction IO3-0 ~ IO3-7
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 4, 0xFE, 0x00); // bank4 // direction IO4-0 ~ IO4-7
#else
	/* LCD_ON, TVOUT_ON, GPS_PWREN, EXT_CODEC_ON, BT_ON, BT_WAKE, TSPR_ON, P-CAM_PWR_ON
	 * COMPASS_RST, IPOD_EN, AUTH_RST#, TV_SLEEP#, CAS_ON, CAS_GP, SMART_AUX1, SMART_AUX2 */

	//pca953x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 0x00003FFF, 0x00000000);
	pca953x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 0x00003FFF, 0x00000000);

	/* V_5P0_EN, USB30_EN, U20H0_EN, U20H1_EN, HDMI_EN, LVDS_EN, PWR_CTL0, PWR_CTL1
	 * CODEC_ON, ... */

	pca953x_init(GPIO_PORTEXT2, I2C_CH_MASTER0, 3, 0x0000FFFF, 0x00000003);
#endif

	keys_init();
	keypad_init();

	if (target_is_emmc_boot()) {
		emmc_boot_main();       // emmc boot
		if (check_fwdn_mode() || keys_get_state(KEY_MENU) != 0)
			goto fwdn_start;
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
	sprintf(s, " androidboot.hardware=tcc896x vmalloc=%s", "480M");
	strcat(cmdline,s);
}
