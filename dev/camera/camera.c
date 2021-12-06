/****************************************************************************
  Copyright (C) 2014 Telechips, Inc.
****************************************************************************/
/****************************************************************************
   Written by S.W.Hwang (Audio and Display Team, Telechips Inc.)
****************************************************************************/
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <reg.h>
#include <platform/iomap.h>
#include <dev/fbcon.h>
#include <plat/cpu.h>

#include <tcc_pmap_avn.h>

#include <platform/reg_physical.h>
#include <platform/structures_cm4.h>
#include <platform/irqs.h>
#include <platform/tcc_ckc.h>
#include <platform/CM4_earlycamera.h>

#include <i2c.h>
#include <dev/gpio.h>
#include <platform/gpio.h>
#include <platform/vioc_cam_plugin.h>
#include <tcc_lcd.h>
#include <dev/camera/camera.h>
#include <dev/camera/sensor_if.h>
#include <splash/splashimg.h>

extern struct tcc_cif_parameters parameters_data;

void tcc_cif_delay(int ms)
{
	unsigned int msec;

	msec = ms / 10; //10msec unit

	if(!msec)	mdelay(1);
	else		mdelay(msec);
}

void tcc_mbox_flush_buffers(void)
{
	volatile MAILBOX * pMbox;
	// flush mbox0
	pMbox = (volatile MAILBOX *)HwCORTEXM4_MAILBOX0_BASE;
	memset((void *)&pMbox->uMBOX_TX0, 0, sizeof(unsigned int) * 8);
	// flush mbox0
	pMbox = (volatile MAILBOX *)HwCORTEXM4_MAILBOX1_BASE;
	memset((void *)&pMbox->uMBOX_TX0, 0, sizeof(unsigned int) * 8);
}


// Mailbox control Setting, Cortex A15 - M4
static void CM_MailBox_Configure(void)
{
	volatile PMAILBOX pMailBoxMain	= (volatile PMAILBOX)HwCORTEXM4_MAILBOX0_BASE;
	volatile PMAILBOX pMailBoxSub	= (volatile PMAILBOX)HwCORTEXM4_MAILBOX1_BASE;
	volatile PCM_TSD_CFG pTSDCfg	= (volatile PCM_TSD_CFG)HwCORTEXM4_TSD_CFG_BASE;

	tcc_mbox_flush_buffers();

	BITSET(pMailBoxMain->uMBOX_CTL_016.nREG, Hw0|Hw1|Hw4|Hw5|Hw6);
	BITSET(pMailBoxSub->uMBOX_CTL_016.nREG, Hw0|Hw1|Hw4|Hw5|Hw6);
	BITSET(pTSDCfg->IRQ_MASK_POL.nREG, Hw16|Hw22); 
}

static void CM_MailBox_WaitRxPolling(unsigned int request_rx)
{
	volatile unsigned int mailbox_data;
	volatile PMAILBOX pMailBox = (volatile PMAILBOX)HwCORTEXM4_MAILBOX0_BASE;

	do {
		mailbox_data = pMailBox->uMBOX_RX0.nREG;
		tcc_cif_delay(200);		
		dprintf(INFO,"[WaitRxPolling] Master RX0 DATA = 0x%08lx\n",mailbox_data);		
	} while(mailbox_data  != request_rx);	
	
}

// for Cortex M4 binary(firmware)
static void CM_UnloadBinary(void)
{
	volatile PCM_TSD_CFG pTSDCfg = (volatile PCM_TSD_CFG)HwCORTEXM4_TSD_CFG_BASE;
	BITSET(pTSDCfg->CM_RESET.nREG, Hw1|Hw2); //m3 no reset
	dprintf(INFO,"%s:%d\n",__func__, __LINE__);
}


static void CM_LoadBinary(unsigned char *fw_data, unsigned int fw_size)
{
	volatile unsigned int * pCodeMem = (volatile unsigned int *)HwCORTEXM4_CODE_MEM_BASE;
	volatile PCM_TSD_CFG pTSDCfg = (volatile PCM_TSD_CFG)HwCORTEXM4_TSD_CFG_BASE;

	CM_UnloadBinary();

	dprintf(INFO,"%s:%d\n",__func__, __LINE__);

	if(fw_data && fw_size > 0)
	{
		memcpy((void *)pCodeMem, (void *)fw_data, fw_size);
	}
	else
		dprintf(INFO,"Using previous loading the firmware\n");

	BITCLR(pTSDCfg->CM_RESET.nREG, Hw1|Hw2);
}

// for sync parameters, Cortex-A15 - M4
void tcc_sync_parameters(void)
{
	memset((void*)SYNC_BASE_ADDR,0,SYNC_BASE_SIZE);
	memcpy((void*)SYNC_BASE_ADDR,&parameters_data,sizeof(struct tcc_cif_parameters));
	dprintf(INFO, "preview addr : 0x%x \n", parameters_data.Lcdc_address0);
}

static void parking_guide_clear(void)
{
	unsigned int parking_guide = PARKING_GUIDE_BASE;
	memset((void *)parking_guide, 0xff, (parameters_data.PGL_width * parameters_data.PGL_height * 4));
}

void startCM4_preview(void)
{
	unsigned int splash_width, splash_height;
	tcc_set_clkctrl(FBUS_CMBUS, ENABLE, 100000000);             // enable Cortex-M4.
//	tcc_cif_delay(200);
	thread_sleep(200);

#if defined(CONFIG_TCC_PARKING_GUIDE_LINE)
	parameters_data.PGL_use = 1;

	parking_guide_clear();

	get_splash_image_early_camera_V2(CONFIG_TCC_PARKING_GUIDE_LINE_NAME, PARKING_GUIDE_BASE, &splash_width, &splash_height);
	printf("CONFIG_TCC_PARKING_GUIDE_LINE_NAME = %s, PARKING_GUIDE_BASE = 0x%x, splah_width = %d, splah_height = %d\n",
			CONFIG_TCC_PARKING_GUIDE_LINE_NAME, PARKING_GUIDE_BASE, splash_width, splash_height);

	if(parameters_data.PGL_width != splash_width || parameters_data.PGL_height != splash_height) {
		printf(INFO, "PGL size doesn't match!!!! \n");
		parameters_data.PGL_use = 0;
	}

#endif
	tcc_sync_parameters();                                      // synchronized parameter.

//if you want to use deintl_s on chip tcc897x, you should set deintl_s register like below line.(TIMS : ID043A-51)
#if defined(TCC897X)
	*((volatile unsigned int *)0x72003800) = 0x3;
#endif

	CM_MailBox_Configure(); // Mailbox setting. 
	CM_LoadBinary(CM4_earlycamera, sizeof(CM4_earlycamera)); // load Cortex-M4 binary.
	CM_MailBox_WaitRxPolling(0x0831);                           // Check to run CM4 with mailbox command.
}
void tcc_cif_open(void)
{
	dprintf(INFO,"EarlyCamera - Start. \n");

	sensor_if_open();

	startCM4_preview();	                                          //CM4 preview start
}

int stopEarlyCamera(void) {
	volatile PMAILBOX pMailBox = (volatile PMAILBOX)HwCORTEXM4_MAILBOX0_BASE;
	volatile unsigned long val = 0x51;
	volatile unsigned long ack = (val | 0x10000000);

	pMailBox->uMBOX_TX0.nREG = val;
	CM_MailBox_WaitRxPolling(ack);
	CM_UnloadBinary();
//	tcc_set_clkctrl(FBUS_CMBUS, DISABLE, 1000000);			// disable Cortex-M4.

	return 0;
}

int tcc_cif_close(void) {
	dprintf(INFO, "EarlyCamera - Finish\n");

	stopEarlyCamera();
//	sensor_if_close();
	
	return 0;
}

