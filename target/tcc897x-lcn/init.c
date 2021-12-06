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

#ifdef USE_CM4_EARLY_CAM
#include <dev/camera/camera.h>
#endif

#define LINUX_MACHTYPE  5014

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
	i2c_init(I2C_CH_MASTER0, 11);
	i2c_init(I2C_CH_MASTER1, 18);
	i2c_init(I2C_CH_MASTER2, 23);
	i2c_init(I2C_CH_MASTER3, 12);
	i2c_init(I2C_CH_SMU, 0);
}

void target_init(void)
{
	uint32_t base_addr;
	uint8_t slot;

	dprintf(INFO, "target_init()\n");

#ifdef TCC_PCA950X_USE
	/*
	GPIO EXPANDER U36
	BANK0			BANK1    	BANK2		BANK3			BANK4
	0 - USB20H0VBUS_FAULT   0 -		0 -		0 - CODEC_GPIO0		0 - V_5P0_EN
	1 - OTGVBUS_FAULT	1 -		1 -		1 - CODEC_GPIO1		1 - OTG_EN
	2 - COMPASS_IRQ		2 -		2 -		2 - CODEC_GPIO2		2 - U20H0_EN
	3 - SDWF_WP		3 -		3 - 		3 - CODEC_GPIO3		3 - HDMI_EN
	4 - SD2_WP		4 -		4 -		4 - CODEC_GPIO4		4 - LVDS_EN 
	5 - SD2_CD		5 -		5 -		5 - CODEC_GPIO5		5 - PWR_CTL0
	6 - SMART_DET		6 -		6 -		6 - x			6 - PWR_CTL1
	7 - x 			7 -		7 -		7 - CAS_PWR_SEL		7 - x
	*/
	
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 0, 0xFF, 0xFF); // bank0 // direction IO0-0 ~ IO0-7 
//	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 1, 0xFF, 0x00); // bank1 // direction IO1-0 ~ IO1-7 
//	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 2, 0xFF, 0x00); // bank2 // direction IO2-0 ~ IO2-7 
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 3, 0x00, 0xFF); // bank3 // direction IO3-0 ~ IO3-7 
	pca950x_init(GPIO_PORTEXT1, I2C_CH_MASTER0, 0, 4, 0x00, 0xFF); // bank4 // direction IO4-0 ~ IO4-7 

	/*
	GPIO EXPANDER U37
	BANK0			BANK1    		BANK2			BANK3			BANK4
	0 - TVOUT_ON  		0 - GPS_PWREN		0 - CAM0_ON		0 - CODEC_ON		0 - x
	1 - LCD_ON		1 - GPS_PWDN		1 - P-CAM0_PWR_ON	1 - EXT_CODEC_ON	1 - x
	2 - LCD_DISP		2 - BT_ON		2 - CAM4_ON		2 - MUTE_CTL		2 - x
	3 - TV_SLEEP#		3 - BT_RST#		3 - CAM4_PWDN 		3 - DXB_ON		3 - OTGVBUS_EN
	4 - CAS_ON		4 - BT_WAKE		4 - CAM4_RST#		4 - DXB0_RST#		4 - USB20H0VBUS_EN
	5 - CAS_GP		5 - SDWF_RST#		5 - CAM4_STDBY		5 - DXB1_RST#		5 - EDI_RST#
	6 - CAS_RST#		6 - IPOD_ON		6 - P-CAM4_PWR_ON	6 - DXB0_PD		6 - EDI_ON
	7 - SD2_ON 		7 - AUTH_RST#		7 - COMPASS_RST		7 - DXB1_PD		7 - x
	*/	

	pca950x_init(GPIO_PORTEXT2, I2C_CH_MASTER0, 1, 0, 0x00, 0xFF); // bank0 // direction IO0-0 ~ IO0-7 
	pca950x_init(GPIO_PORTEXT2, I2C_CH_MASTER0, 1, 1, 0x00, 0xFF); // bank1 // direction IO1-0 ~ IO0-7 
	pca950x_init(GPIO_PORTEXT2, I2C_CH_MASTER0, 1, 2, 0x00, 0xFF); // bank2 // direction IO2-0 ~ IO0-7 
	pca950x_init(GPIO_PORTEXT2, I2C_CH_MASTER0, 1, 3, 0x00, 0xFF); // bank3 // direction IO3-0 ~ IO0-7 
	pca950x_init(GPIO_PORTEXT2, I2C_CH_MASTER0, 1, 4, 0x00, 0xFF); // bank4 // direction IO4-0 ~ IO0-7 	
#endif

#if DISPLAY_SPLASH_SCREEN
	dprintf(INFO, "Display Init: Start\n");
	display_init();
	dprintf(INFO, "Display Init: Done\n");
#endif

#ifdef USE_CM4_EARLY_CAM
	startEarlyCamera();
#endif

	if (target_is_emmc_boot()) {
		emmc_boot_main();       // emmc boot
		if (check_fwdn_mode())
			goto fwdn_start;
	}else {
		edi_init();
		if (check_fwdn_mode())
			goto fwdn_start;

		flash_boot_main();              // nand boot
	}

#if defined (CONFIG_TCC_PARKING_GUIDE_LINE)
	parking_guide_clear();
	load_parking_guide();
#endif

	return;

fwdn_start:
#if defined (CONFIG_TCC_PARKING_GUIDE_LINE)
	parking_guide_clear();
	load_parking_guide();
#endif
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

void target_cmdline_kpanic(char *cmdline)
{
	char s[128];
	unsigned long kpanic_base = 0;
	unsigned long kpanic_size = 0;

	if (get_partition_info("kpanic", &kpanic_base, &kpanic_size) )
	{
		printf("%s : get kpanic info failed...\n", __func__);
		return -1;
	}

	printf("%s : kpanic_base(%d), kpanic_size(%d)\n", __func__, kpanic_base, kpanic_size);

	sprintf(s, " tcc_kpanic_base=%d tcc_kpanic_size=%d", kpanic_base, kpanic_size);
	strcat(cmdline, s);
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
	sprintf(s, " androidboot.hardware=tcc897x vmalloc=%s", "480M");
	strcat(cmdline,s);
}
