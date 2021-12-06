/*
 * Copyright (c) 2010 Telechips, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <platform/iomap.h>
#include <dev/fbcon.h>
#include <plat/cpu.h>

#include <lcd.h>
#include <i2c.h>
#include <dev/gpio.h>

#include <platform/reg_physical.h>
#include <platform/irqs.h>
#include <platform/tcc_ckc.h>
#include <platform/gpio.h>

#include <tcc_lcd.h>
#include <tca_ddic_interface.h>

#include "vioc/vioc_wmix.h"
#include "vioc/vioc_rdma.h"
#include "vioc/vioc_wdma.h"
#include "vioc/vioc_outcfg.h"
#include "vioc/vioc_disp.h"
#include "vioc/vioc_scaler.h"
#include "vioc/vioc_config.h"

//#define _LCD_32BPP_           //LCD output format setting
#if defined(_LCD_32BPP_)
#define LCDC_FB_BPP		32
#else
#define LCDC_FB_BPP		16
#endif

#if defined(DEFAULT_DISPLAY_HDMI) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
#if defined(TCC_HDMI_DRIVER_V1_3)
#include <regs-hdmi-v1-3.h>
#include <hdmi_v1_3_hdmi.h>
#include <hdmi_v1_3_audio.h>
#elif defined(TCC_HDMI_DRIVER_V1_4)
#include <regs-hdmi-v1-4.h>
#include <hdmi_v1_4_hdmi.h>
#include <hdmi_v1_4_audio.h>
#endif
#endif


#if defined(DISPLAY_SPLASH_SCREEN_DIRECT)
// Define boot image infomation 
#ifdef _LCD_32BPP_
 #include <lcd/logo_24bit.h>
#else
 #include <lcd/logo.h>
#endif
#include <lcd/low.h>

#if defined(TARGET_BOARD_STB)
	#ifdef _LCD_32BPP_
	#define LCDC_FB_WIDTH	480
	#define LCDC_FB_HEIGHT	272
	#else
	#define LCDC_FB_WIDTH	640
	#define LCDC_FB_HEIGHT	480
	#endif
#else
	#define LCDC_FB_WIDTH	480
	#define LCDC_FB_HEIGHT	272
#endif

static struct fbcon_config fb_lb = {
	.height		= 275,
	.width		= 481,
	.format		= FB_FORMAT_RGB565,
	.bpp		= LCDC_FB_BPP,
	.update_start	= NULL,
	.update_done	= NULL,
	.base = lowbattery_logo
};

static struct fbcon_config fb_cfg = {
	.height		= LCDC_FB_HEIGHT,
	.width		= LCDC_FB_WIDTH,
	.format		= FB_FORMAT_RGB565,
	.bpp		= LCDC_FB_BPP,
	.update_start	= NULL,
	.update_done	= NULL,
	.base = telechips_logo
};

static struct fbcon_config fb_fr = {
        .height         = LCDC_FB_HEIGHT,
        .width          = LCDC_FB_WIDTH,
        .format         = FB_FORMAT_RGB565,
        .bpp            = LCDC_FB_BPP,
        .update_start   = NULL,
        .update_done    = NULL,
        .base = factory_reset_msg
};
#else
static struct fbcon_config fb_cfg;
static struct fbcon_config fb_fr;
#endif//


// Boot loader image momory location.
#define BOOT_LOGO_MEMORY 		((VIDEO_BASE+VIDEO_SIZE) - ((fb_cfg.width * fb_cfg.height) *4))

#define MAX_SPLASH_IMAGE_WIDTH		1280
#define MAX_SPLASH_IMAGE_HEIGHT		720

static struct fbcon_config fb0_cfg_splash;
static struct fbcon_config fb1_cfg_splash;

enum
{
	LCDC_COMPOSITE_NTSC,
	LCDC_COMPOSITE_PAL,
	LCDC_COMPOSITE_MAX
};

enum
{
	LCDC_COMPONENT_480I_NTSC,
	LCDC_COMPONENT_576I_PAL,
	LCDC_COMPONENT_720P,
	LCDC_COMPONENT_1080I,
	LCDC_COMPONENT_MAX
};

static char defalut_composite_resolution = LCDC_COMPOSITE_NTSC;
static char defalut_component_resolution = LCDC_COMPONENT_720P;

//#define LCDC0_USE
#ifdef LCDC0_USE
#define LCD_LCDC_NUM 	0
#else
#define LCD_LCDC_NUM 	1
#endif//

#if defined(TARGET_TCC893X_EVM)
//  U1_1:LCD_PWRCTL 	C29:LCD_DISP  A4 : LCD_BL_EN, U3_12: LCD_RESET
	#if (HW_REV == 0x1000)
	#define GPIO_LCD_ON			( GPIO_EXT1|1)
	#define GPIO_LCD_BL			TCC_GPF(16)//( GPIO_PORTD|10)
	#define GPIO_LCD_DISPLAY	TCC_GPB(7)
	#define GPIO_LCD_RESET		( TCC_GPD(20) )
	#elif (HW_REV ==0x2000 || HW_REV == 0x3000)
	#define GPIO_LCD_ON			( GPIO_EXT1|1)
	#define GPIO_LCD_BL			TCC_GPG(5)
	#define GPIO_LCD_DISPLAY	TCC_GPB(28)
	#define GPIO_LCD_RESET		( GPIO_NC )
	#else
	#define GPIO_LCD_ON			( GPIO_EXT1|1)
	#define GPIO_LCD_BL			TCC_GPF(16)//( GPIO_PORTD|10)
	#define GPIO_LCD_DISPLAY	TCC_GPB(7)
	#define GPIO_LCD_RESET		( GPIO_NC )	
	#endif//
#elif defined(TARGET_M805_893X_EVM)
	#if (HW_REV == 0x5000 || HW_REV == 0x5001 || HW_REV == 0x5002 || HW_REV == 0x5003)
	#define GPIO_LCD_ON 		TCC_GPE(24)
	#define GPIO_LCD_BL 		TCC_GPC(0) 
	#define GPIO_LCD_DISPLAY	( GPIO_NC )
	#define GPIO_LCD_RESET		TCC_GPB(29)
	#else
		#err Not defined HW_REV
	#endif
#elif defined(TARGET_TCC8920ST_EVM) || defined(TARGET_TCC8930ST_EVM)
	#define GPIO_LCD_ON 		( GPIO_NC )
	#define GPIO_LCD_BL 		( GPIO_NC ) 
	#define GPIO_LCD_DISPLAY	( GPIO_NC )
	#define GPIO_LCD_RESET		( GPIO_NC )
#else
	#error code unknow-device
#endif//

#define HwVIOC_CONFIG_MISC1		(0x7200A084)
#define HwVIOC_CONFIG_DEV_SEL	(0x7200A0BC)

struct atag_tcc_entry
{
	char output;
	char resolution;
	char hdmi_resolution;
	char composite_resolution;
	char component_resolution;
	char hdmi_mode;
};

extern struct atag_tcc_entry tcc_param;

static void lcdc_set_logo(unsigned char lcdc_num, unsigned lcd_wd, unsigned lcd_ht, struct fbcon_config *fb_con)
{
	struct tcc_lcdc_image_update Image_info;

	dprintf(INFO, "%s fb_cfg base:%p xres:%d yres:%d bpp:%d \n", __func__, fb_con->base, fb_con->width, fb_con->height, fb_con->bpp);

#if defined(TARGET_BOARD_STB)
	if(fb_con->base == NULL)
	{
		int buffer_size = lcd_wd * lcd_ht * (LCDC_FB_BPP/8);

		Image_info.addr0 = dma_alloc(4096, buffer_size);

		if (Image_info.addr0 == NULL)		{
			//printf("framebuffer allocation is failed!, size=0x%08x\n", buffer_size);
		}
		else {
			//printf("framebuffer allocation is succeed!, size=0x%08x, addr=0x%08x\n", buffer_size, Image_info.addr0);
			memset(Image_info.addr0, 0x00, buffer_size);
		}

		Image_info.Lcdc_layer = 0;
		Image_info.enable = 1;

		Image_info.Frame_width = lcd_wd;
		Image_info.Frame_height = lcd_ht;

		Image_info.Image_width = lcd_wd;
		Image_info.Image_height = lcd_ht;

		Image_info.offset_x =  0;
		Image_info.offset_y =  0;
	}
	else
#endif
	{
		Image_info.addr0 = (unsigned int)fb_con->base;
		Image_info.Lcdc_layer = 0;
		Image_info.enable = 1;

		Image_info.Frame_width = lcd_wd;
		Image_info.Frame_height = lcd_ht;

		Image_info.Image_width = fb_con->width;
		Image_info.Image_height = fb_con->height;

		#if defined(DISPLAY_SCALER_USE)
			Image_info.offset_x =  0;
			Image_info.offset_y =  0;
		#else
			if(Image_info.Image_width > lcd_wd)
				Image_info.Image_width = lcd_wd;

			if(lcd_wd > Image_info.Image_width)
				Image_info.offset_x = (lcd_wd - Image_info.Image_width)/2;
			else
				Image_info.offset_x =  0;

			if(Image_info.Image_height > lcd_ht)
				Image_info.Image_height = lcd_ht;
				
			if(lcd_ht > Image_info.Image_height)
				Image_info.offset_y = (lcd_ht - Image_info.Image_height)/2;
			else
				Image_info.offset_y = 0;
		#endif
	}	

	#ifdef _LCD_32BPP_
	Image_info.fmt = TCC_LCDC_IMG_FMT_RGB888;
	#else
	Image_info.fmt = TCC_LCDC_IMG_FMT_RGB565;
	#endif

	tcclcd_image_ch_set(lcdc_num, &Image_info);

	mdelay(1);
}

#if defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
static int hdmi_detect(void)
{
	int ret = true;

	gpio_config(TCC_GPHDMI(1), GPIO_FN0 | GPIO_INPUT); /* HDMI_DET */
	ret = gpio_get(TCC_GPHDMI(1))? true : false;

	return ret;
}

static int composite_detect(void)
{
	int ret = true;

	#if defined(DISPLAY_STB_AUTO_DETECT)
		#if defined(BOARD_TCC880X_STB_DEMO)
			gpio_config(GPIO_PORTF|27, GPIO_FN0 | GPIO_INPUT); /* CVBS_DET */
			ret = gpio_get(GPIO_PORTF|27)? false : true;
		#elif defined(TARGET_TCC8920ST_EVM) || defined(TARGET_TCC8930ST_EVM)
			gpio_config(GPIO_PORTF|1, GPIO_FN0 | GPIO_INPUT); /* CVBS_DET */
			ret = gpio_get(GPIO_PORTF|1)? false : true;
		#endif
	#endif

	return ret;
}

static int component_detect(void)
{
	int ret = true;

	#if defined(DISPLAY_STB_AUTO_DETECT)
		#if defined(BOARD_TCC880X_STB_DEMO)
			gpio_config(GPIO_PORTF|26, GPIO_FN0 | GPIO_INPUT); /* VE_DET */
			ret = gpio_get(GPIO_PORTF|26)? false : true;
		#elif defined(TARGET_TCC8920ST_EVM) || defined(TARGET_TCC8930ST_EVM)
			gpio_config(GPIO_PORTB|29, GPIO_FN0 | GPIO_INPUT); /* VE_DET */
			ret = gpio_get(GPIO_PORTB|29)? false : true;
		#endif
	#endif

	return ret;
}
#endif

#if defined(DEFAULT_DISPLAY_COMPOSITE) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
static void composite_get_spec(unsigned char type, stCOMPOSITE_SPEC *spec)
{
	switch(type)
	{
		case LCDC_COMPOSITE_NTSC:
			spec->composite_clock = 27000000;
			spec->composite_divider = 1;
			spec->composite_bus_width = 8;
			spec->composite_width = 720;
			spec->composite_height = 480;

			#if 0 // COMPOSITE_CCIR656
			spec->composite_LPW = 224 - 1; 				// line pulse width
			spec->composite_LPC = 720 * 2 - 1; 			// line pulse count (active horizontal pixel - 1)
			spec->composite_LSWC = 20 - 1;				// line start wait clock (the number of dummy pixel clock - 1)
			spec->composite_LEWC = 32 - 1;				// line end wait clock (the number of dummy pixel clock - 1)
			#else
			spec->composite_LPW = 212 - 1; 				// line pulse width
			spec->composite_LPC = 720 * 2 - 1; 			// line pulse count (active horizontal pixel - 1)
			spec->composite_LSWC = 32 - 1;				// line start wait clock (the number of dummy pixel clock - 1)
			spec->composite_LEWC = 32 - 1;				// line end wait clock (the number of dummy pixel clock - 1)
			#endif

			spec->composite_VDB = 0; 					// Back porch Vsync delay
			spec->composite_VDF = 0; 					// front porch of Vsync delay
			spec->composite_FPW1 = 1 - 1;				// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->composite_FLC1 = 480 - 1;				// frmae line count is the number of lines in each frmae on the screen
			spec->composite_FSWC1 = 37 - 1;				// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->composite_FEWC1 = 7 - 1;				// frame start wait cycle is the number of lines to insert at the begining each frame
			spec->composite_FPW2 = 1 - 1;				// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->composite_FLC2 = 480 - 1;				// frmae line count is the number of lines in each frmae on the screen
			spec->composite_FSWC2 = 38 - 1;				// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->composite_FEWC2 = 6 - 1; 				// frame start wait cycle is the number of lines to insert at the begining each frame
			break;

		case LCDC_COMPOSITE_PAL:		
			spec->composite_clock = 27000000;
			spec->composite_divider = 1;
			spec->composite_bus_width = 8;
			spec->composite_width = 720;
			spec->composite_height = 576;

			spec->composite_LPW = 128 - 1; 				// line pulse width
			spec->composite_LPC = 720 * 2 - 1; 			// line pulse count (active horizontal pixel - 1)
			spec->composite_LSWC = 138 - 1;				// line start wait clock (the number of dummy pixel clock - 1)
			spec->composite_LEWC = 22 - 1;				// line end wait clock (the number of dummy pixel clock - 1)

			spec->composite_VDB = 0; 					// Back porch Vsync delay
			spec->composite_VDF = 0; 					// front porch of Vsync delay
			spec->composite_FPW1 = 1 - 1;				// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->composite_FLC1 = 576 - 1;				// frmae line count is the number of lines in each frmae on the screen
			spec->composite_FSWC1 = 43-1;				// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->composite_FEWC1 = 5-1;				// frame start wait cycle is the number of lines to insert at the begining each frame
			spec->composite_FPW2 = 1 - 1;				// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->composite_FLC2 = 576 - 1;				// frmae line count is the number of lines in each frmae on the screen
			spec->composite_FSWC2 = 44-1;//4 				// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->composite_FEWC2 = 4-1; 				// frame start wait cycle is the number of lines to insert at the begining each frame
			break;
	}
}
#endif

#if defined(DEFAULT_DISPLAY_COMPONENT) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
static void component_get_spec(unsigned char type, stCOMPONENT_SPEC *spec)
{
	switch(type)
	{
		case LCDC_COMPONENT_480I_NTSC:
			spec->component_clock = 270000;
			spec->component_divider = 1;
			spec->component_bus_width = 8;
			spec->component_width = 720;
			spec->component_height = 480;
			spec->component_LPW = 128 - 1; 					// line pulse width
			spec->component_LPC = 720 * 2 - 1; 				// line pulse count (active horizontal pixel - 1)
			spec->component_LSWC = 116 - 1;					// line start wait clock (the number of dummy pixel clock - 1)
			spec->component_LEWC = 32 - 1;					// line end wait clock (the number of dummy pixel clock - 1)

			spec->component_VDB = 0; 						// Back porch Vsync delay
			spec->component_VDF = 0; 						// front porch of Vsync delay

			spec->component_FPW1 = 6 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->component_FLC1 = 480 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->component_FSWC1 = 30 - 1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->component_FEWC1 = 9 - 1;					// frame start wait cycle is the number of lines to insert at the begining each frame
			spec->component_FPW2 = 6 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->component_FLC2 = 480 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->component_FSWC2 = 31 - 1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->component_FEWC2 = 8 - 1; 					// frame start wait cycle is the number of lines to insert at the begining each frame
			break;
				
		case LCDC_COMPONENT_576I_PAL:
			spec->component_clock = 270000;
			spec->component_divider = 1;
			spec->component_bus_width = 8;
			spec->component_width = 720;
			spec->component_height = 576;
			spec->component_LPW = 128 - 1; 					// line pulse width
			spec->component_LPC = 720 * 2 - 1; 				// line pulse count (active horizontal pixel - 1)
			spec->component_LSWC = 136 - 1;					// line start wait clock (the number of dummy pixel clock - 1)
			spec->component_LEWC = 24 - 1;					// line end wait clock (the number of dummy pixel clock - 1)

			spec->component_VDB = 0; 						// Back porch Vsync delay
			spec->component_VDF = 0; 						// front porch of Vsync delay
				
			spec->component_FPW1 = 5 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->component_FLC1 = 576 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->component_FSWC1 = 39 - 1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->component_FEWC1 = 5 - 1;					// frame start wait cycle is the number of lines to insert at the begining each frame
			spec->component_FPW2 = 5 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->component_FLC2 = 576 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->component_FSWC2 = 40 - 1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->component_FEWC2 = 4 - 1; 					// frame start wait cycle is the number of lines to insert at the begining each frame
			break;

		case LCDC_COMPONENT_720P:
			spec->component_clock = 742500;
			spec->component_divider = 1;
			spec->component_bus_width = 24;
			spec->component_width = 1280;
			spec->component_height = 720;

			spec->component_LPW = 9 - 1; 					// line pulse width
			spec->component_LPC = 1280 - 1; 				// line pulse count (active horizontal pixel - 1)
			spec->component_LSWC = 349 - 1;					// line start wait clock (the number of dummy pixel clock - 1)
			spec->component_LEWC = 12 - 1;					// line end wait clock (the number of dummy pixel clock - 1)

			spec->component_VDB = 0; 						// Back porch Vsync delay
			spec->component_VDF = 0; 						// front porch of Vsync delay
				
			spec->component_FPW1 = 3 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->component_FLC1 = 720 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->component_FSWC1 = 26 - 1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->component_FEWC1 = 1 - 1;					// frame start wait cycle is the number of lines to insert at the begining each frame
			spec->component_FPW2 = 3 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->component_FLC2 = 720 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->component_FSWC2 = 26 - 1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->component_FEWC2 = 1 - 1; 					// frame start wait cycle is the number of lines to insert at the begining each frame
			break;

		case LCDC_COMPONENT_1080I:
			spec->component_clock = 742500;
			spec->component_divider = 1;
			spec->component_bus_width = 24;
			spec->component_width = 1920;
			spec->component_height = 1080;

			spec->component_LPW = 24 - 1; 					// line pulse width
			spec->component_LPC = 1920 - 1; 				// line pulse count (active horizontal pixel - 1)
			spec->component_LSWC = 228 - 1;					// line start wait clock (the number of dummy pixel clock - 1)
			spec->component_LEWC = 28 - 1;					// line end wait clock (the number of dummy pixel clock - 1)

			spec->component_VDB = 0; 						// Back porch Vsync delay
			spec->component_VDF = 0; 						// front porch of Vsync delay
				
			spec->component_FPW1 = 5*2 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->component_FLC1 = 540*2 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->component_FSWC1 = 15*2 - 1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->component_FEWC1 = 2.5*2 - 1;					// frame start wait cycle is the number of lines to insert at the begining each frame
			spec->component_FPW2 = 5*2 - 1;					// TFT/TV : Frame pulse width is the pulse width of frmae clock
			spec->component_FLC2 = 540*2 - 1;					// frmae line count is the number of lines in each frmae on the screen
			spec->component_FSWC2 = 15.5*2 - 1;					// frmae start wait cycle is the number of lines to insert at the end each frame
			spec->component_FEWC2 = 2*2 - 1; 					// frame start wait cycle is the number of lines to insert at the begining each frame
			break;
	}
}
#endif

#if defined(DEFAULT_DISPLAY_HDMI) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
extern void hdmi_start(void);
extern void hdmi_ddi_config_init (void);
extern int hdmi_set_hdmi_mode(int mode);
extern int hdmi_set_video_mode(const struct HDMIVideoParameter * const hdmi_video_mode);
extern int hdmi_set_audio_mode(const struct HDMIAudioParameter * const hdmi_audio_mode);
static void lcdc_io_init_hdmi(unsigned char lcdc_num)
{
	uint width, height;
	unsigned int factory_reset = 0;
	VIOC_DISP *pDISP;
	VIOC_WMIX *pWIMX;
	VIOC_RDMA *pRDMA;

	stLTIMING HDMI_TIMEp;
	stLCDCTR pCtrlParam;
	volatile PDDICONFIG pDDI_Config = (PDDICONFIG)HwDDI_CONFIG_BASE;
	
	struct fbcon_config *fb_con, *fb_con_splash;
	struct lcd_panel *panel_info;
	
	const struct HDMIVideoParameter video = {
	#if (HDMI_MODE_TYPE == 1)
	/*	video.mode 				=*/	HDMI,
	#else
	/*	video.mode 				=*/	DVI,
	#endif
	/*	video.resolution 		=*/	gRefHdmiVideoModeList[HDMI_VIDEO_MODE_TYPE].vfmt_val,
	/*	video.colorSpace		=*/	HDMI_CS_RGB,
	/*	video.colorDepth		=*/	HDMI_CD_24,
	/*	video.colorimetry		=*/	HDMI_COLORIMETRY_NO_DATA,
	/*	video.pixelAspectRatio	=*/	gRefHdmiVideoModeList[HDMI_VIDEO_MODE_TYPE].ratio,
	#if defined(TCC_HDMI_DRIVER_V1_4)
	/*  video.videoSrc          =*/ HDMI_SOURCE_EXTERNAL,
	/*  video.Video Structure   =*/ HDMI_2D_VIDEO_FORMAT
	#endif
	};

	const struct HDMIAudioParameter audio = {
	/*	audio.inputPort 		=*/	I2S_PORT,
	/*	audio.outPacket 		=*/	HDMI_ASP,
	/*	audio.formatCode		=*/	LPCM_FORMAT,
	/*	audio.channelNum		=*/	CH_2,
	/*	audio.sampleFreq		=*/	SF_44KHZ,
	/*	audio.wordLength		=*/	WORD_16,

	/*	audio.i2sParam.bpc	=*/	I2S_BPC_16,
	/*	audio.i2sParam.format	=*/	I2S_BASIC,
	/*	audio.i2sParam.clk		=*/	I2S_64FS
	};

	dprintf(INFO, "%s LCDC NUM:%d \n", __func__, lcdc_num);
		
	fb_con = &fb_cfg;
			
	#if defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
		#if defined(TCC_FACTORY_RESET_SUPPORT)
			#if defined(TARGET_TCC8935_STICK)
				#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
					if(!check_fwdn_mode()) {
						if(gpio_get(GPIO_PORTE|20) == false)
						{
							fb_con = &fb_fr;
							factory_reset = 1;
						}
					}
				#else
					if(!check_fwdn_mode()) {
						if(gpio_get(GPIO_PORTE|16) == false)
						{
							fb_con = &fb_fr;
							factory_reset = 1;
						}
					}
				#endif
			#endif
		#endif
	#endif

	if(lcdc_num)	
	{
		pDISP = (VIOC_DISP *)HwVIOC_DISP1;
		pWIMX = (VIOC_WMIX *)HwVIOC_WMIX1; 
		pRDMA = (VIOC_RDMA *)HwVIOC_RDMA04; 
		tcc_set_peri(PERI_LCD1, ENABLE, XTIN_CLK_RATE);
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_HDMI, VIOC_OUTCFG_DISP1);
	}
	else
	{
		pDISP = (VIOC_DISP *)HwVIOC_DISP0;
		pWIMX = (VIOC_WMIX *)HwVIOC_WMIX0; 
		pRDMA = (VIOC_RDMA *)HwVIOC_RDMA00; 
		tcc_set_peri(PERI_LCD0, ENABLE, XTIN_CLK_RATE);
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_HDMI, VIOC_OUTCFG_DISP0);
	}

	tcc_set_ipisol_pwdn(PMU_ISOL_HDMI, 0);
 	tcc_set_peri(PERI_HDMI, ENABLE, 1000000);
 	
	width = gRefHdmiVideoModeList[HDMI_VIDEO_MODE_TYPE].width;
	height = gRefHdmiVideoModeList[HDMI_VIDEO_MODE_TYPE].height;

	HDMI_TIMEp.lpw= LCDCTimimgParams[video.resolution].lpw;
	HDMI_TIMEp.lpc= LCDCTimimgParams[video.resolution].lpc + 1;
	HDMI_TIMEp.lswc= LCDCTimimgParams[video.resolution].lswc+ 1;
	HDMI_TIMEp.lewc= LCDCTimimgParams[video.resolution].lewc+ 1;
	
	HDMI_TIMEp.vdb = LCDCTimimgParams[video.resolution].vdb;
	HDMI_TIMEp.vdf = LCDCTimimgParams[video.resolution].vdf;
	HDMI_TIMEp.fpw = LCDCTimimgParams[video.resolution].fpw;
	HDMI_TIMEp.flc = LCDCTimimgParams[video.resolution].flc;
	HDMI_TIMEp.fswc = LCDCTimimgParams[video.resolution].fswc;
	HDMI_TIMEp.fewc = LCDCTimimgParams[video.resolution].fewc;
	HDMI_TIMEp.fpw2 = LCDCTimimgParams[video.resolution].fpw2;
	HDMI_TIMEp.flc2 = LCDCTimimgParams[video.resolution].flc2;
	HDMI_TIMEp.fswc2 = LCDCTimimgParams[video.resolution].fswc2;
	HDMI_TIMEp.fewc2 = LCDCTimimgParams[video.resolution].fewc2;
	
	VIOC_DISP_SetTimingParam(pDISP, &HDMI_TIMEp);
 
	memset(&pCtrlParam, 0x00, sizeof(pCtrlParam));
	pCtrlParam.id= LCDCTimimgParams[video.resolution].id;
	pCtrlParam.iv= LCDCTimimgParams[video.resolution].iv;
	pCtrlParam.ih= LCDCTimimgParams[video.resolution].ih;
	pCtrlParam.ip= LCDCTimimgParams[video.resolution].ip;
	pCtrlParam.clen = 0;
	if(video.colorSpace == HDMI_CS_RGB)	{
		pCtrlParam.r2y = 0;
		pCtrlParam.pxdw = 12; //RGB888
	}
	else {
		pCtrlParam.r2y = 1;
		pCtrlParam.pxdw = 8; //RGB888
	}
	pCtrlParam.dp = LCDCTimimgParams[video.resolution].dp;
	pCtrlParam.ni = LCDCTimimgParams[video.resolution].ni;
	pCtrlParam.tv = LCDCTimimgParams[video.resolution].tv;
	pCtrlParam.opt = 0;
	pCtrlParam.stn = 0;
	pCtrlParam.evsel = 0;
	pCtrlParam.ovp = 0;

	VIOC_DISP_TurnOff(pDISP);
	VIOC_RDMA_SetImageDisable(pRDMA);

	VIOC_DISP_SetControlConfigure(pDISP, &pCtrlParam);

	VIOC_WMIX_SetOverlayPriority(pWIMX, 24);
	VIOC_WMIX_SetBGColor(pWIMX, 0x00, 0x00, 0x00, 0xff);
	VIOC_WMIX_SetSize(pWIMX, width, height);
	VIOC_WMIX_SetChromaKey(pWIMX, 0, 0, 0, 0, 0, 0xF8, 0xFC, 0xF8);
	VIOC_WMIX_SetUpdate(pWIMX);
	
	BITSET(pDDI_Config->PWDN.nREG, Hw2);
	BITCLR(pDDI_Config->SWRESET.nREG, Hw2);
	BITSET(pDDI_Config->SWRESET.nREG, Hw2);
	
	hdmi_ddi_config_init();
	hdmi_set_hdmi_mode(video.mode);
	hdmi_set_video_mode(&video);

	hdmi_check_phy_ready();
	
	VIOC_DISP_SetSize (pDISP, width, height);
	VIOC_DISP_SetBGColor(pDISP, 0, 0 , 0);
	VIOC_DISP_TurnOn(pDISP);

	if (video.mode == HDMI)
		hdmi_set_audio_mode(&audio);

	hdmi_start();

	if(factory_reset)
	{
 		lcdc_set_logo(lcdc_num, width, height, fb_con);
	}
	else
	{
	#if defined(DISPLAY_SPLASH_SCREEN_DIRECT)	
 		lcdc_set_logo(lcdc_num, width, height, fb_con);
	#else
		if(lcdc_num)
			fb_con_splash = &fb1_cfg_splash;
		else
			fb_con_splash = &fb0_cfg_splash;
			
		fb_con_splash->width = MAX_SPLASH_IMAGE_WIDTH;
		fb_con_splash->height = MAX_SPLASH_IMAGE_HEIGHT;
		fb_con_splash->stride = fb_con_splash->width;
		fb_con_splash->bpp = LCDC_FB_BPP;
		fb_con_splash->format = FB_FORMAT_RGB565;

		fb_con_splash->base = dma_alloc(4096, fb_con_splash->width * fb_con_splash->height * (fb_con_splash->bpp/8));
		memset(fb_con_splash->base, 0x00, fb_con_splash->width * fb_con_splash->height * (fb_con_splash->bpp/8));
	      
		if (fb_con_splash->base == NULL)
			dprintf(INFO, "%s, lcdc: framebuffer alloc failed!\n", __func__);
		else
		{
			dprintf(INFO, "%s, splash image: width=%d, height=%d\n", __func__, width, height);
			if(splash_image_load("logo_hdmi", fb_con_splash) < 0)
			{
		 		lcdc_set_logo(lcdc_num, width, height, fb_con);
	 		}
	 		else
	 		{
		 		lcdc_set_logo(lcdc_num, width, height, fb_con_splash);
	 		}
 		}
	#endif
	}
}
#endif //#if defined(DEFAULT_DISPLAY_HDMI) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)

#if defined(DEFAULT_DISPLAY_COMPOSITE) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
static void lcdc_io_init_composite(unsigned char lcdc_num, unsigned char type)
{
#if defined(DISPLAY_SUPPORT_COMPOSITE)
	unsigned int width, height;
	stCOMPOSITE_SPEC spec;

	stLTIMING				CompositeTiming;
	stLCDCTR				LcdCtrlParam;
	PVIOC_DISP				pDISPBase;
	PVIOC_WMIX				pWIXBase;
	PDDICONFIG 				pDDICfg = (PDDICONFIG)HwDDI_CONFIG_BASE;
	PNTSCPAL 				pTVE = (PNTSCPAL)HwNTSCPAL_BASE;
	PNTSCPAL_ENCODER_CTRL 	pTVE_VEN = (PNTSCPAL_ENCODER_CTRL)HwNTSCPAL_ENC_CTRL_BASE;

	struct fbcon_config *fb_con, *fb_con_splash;
	struct lcd_panel *panel_info;
	
	dprintf(INFO, "%s, lcdc_num=%d, type=%d\n", __func__, lcdc_num, type);
		
	if(type >= LCDC_COMPOSITE_MAX)
		type = defalut_composite_resolution;

	composite_get_spec(type, &spec);

	fb_con = &fb_cfg;

	BITSET(pDDICfg->PWDN.nREG, Hw1);		// PWDN - TVE
	BITCLR(pDDICfg->SWRESET.nREG, Hw1);		// SWRESET - TVE
	BITSET(pDDICfg->SWRESET.nREG, Hw1);		// SWRESET - TVE	
	BITSET(pDDICfg->NTSCPAL_EN.nREG, Hw0);	// NTSCPAL_EN	
	
	if(lcdc_num)	
	{
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP1;
		pWIXBase =(VIOC_WMIX *)HwVIOC_WMIX1; 
		tcc_set_peri(PERI_LCD1, ENABLE, spec.composite_clock*spec.composite_divider);
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_SDVENC, VIOC_OUTCFG_DISP1);
	}
	else
	{
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP0;
		pWIXBase =(VIOC_WMIX *)HwVIOC_WMIX0; 
		tcc_set_peri(PERI_LCD0, ENABLE, spec.composite_clock*spec.composite_divider);
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_SDVENC, VIOC_OUTCFG_DISP0);
	}

	dprintf(INFO, "lcdc_num = %d, LCDC0 clk:%u Hz, LCDC1 clk:%u Hz, divide:%d\n", lcdc_num, tcc_get_peri(PERI_LCD0), tcc_get_peri(PERI_LCD1), spec.composite_divider);
	
	//LCDC_IO_Set(lcdc_num, spec.composite_bus_width);
	
	// hdmi power wake up
	tcc_set_ipisol_pwdn(PMU_ISOL_VDAC, 0);
 	//tcc_set_peri(PERI_HDMI, ENABLE, 1000000);
 	
	width = spec.composite_width;
	height = spec.composite_height;

	#if defined(DISPLAY_SPLASH_SCREEN_DIRECT)	
 		lcdc_set_logo(lcdc_num, width, height, fb_con);
	#else
		if(lcdc_num)
			fb_con_splash = &fb1_cfg_splash;
		else
			fb_con_splash = &fb0_cfg_splash;
			
		fb_con_splash->width = MAX_SPLASH_IMAGE_WIDTH;
		fb_con_splash->height = MAX_SPLASH_IMAGE_HEIGHT;
		fb_con_splash->stride = fb_con_splash->width;
		fb_con_splash->bpp = LCDC_FB_BPP;
		fb_con_splash->format = FB_FORMAT_RGB565;

		fb_con_splash->base = dma_alloc(4096, fb_con_splash->width * fb_con_splash->height * (fb_con_splash->bpp/8));
		memset(fb_con_splash->base, 0x00, fb_con_splash->width * fb_con_splash->height * (fb_con_splash->bpp/8));
	      
		if (fb_con_splash->base == NULL)
			dprintf(INFO, "%s, lcdc: framebuffer alloc failed!\n", __func__);
		else
		{
			dprintf(INFO, "%s, splash image: width=%d, height=%d\n", __func__, width, height);
			if(splash_image_load("logo_composite", fb_con_splash) < 0)
			{
		 		lcdc_set_logo(lcdc_num, width, height, fb_con);
	 		}
	 		else
	 		{
		 		lcdc_set_logo(lcdc_num, width, height, fb_con_splash);
	 		}
 		}
	#endif

	CompositeTiming.lpw = spec.composite_LPW;
	CompositeTiming.lpc = spec.composite_LPC + 1;
	CompositeTiming.lswc = spec.composite_LSWC + 1;
	CompositeTiming.lewc = spec.composite_LEWC + 1;
	
	CompositeTiming.vdb = spec.composite_VDB;
	CompositeTiming.vdf = spec.composite_VDF;
	CompositeTiming.fpw = spec.composite_FPW1;
	CompositeTiming.flc = spec.composite_FLC1;
	CompositeTiming.fswc = spec.composite_FSWC1;
	CompositeTiming.fewc = spec.composite_FEWC1;
	CompositeTiming.fpw2 = spec.composite_FPW2;
	CompositeTiming.flc2 = spec.composite_FLC2;
	CompositeTiming.fswc2 = spec.composite_FSWC2;
	CompositeTiming.fewc2 = spec.composite_FEWC2;
	
	VIOC_DISP_SetTimingParam(pDISPBase, &CompositeTiming);
 
	memset(&LcdCtrlParam, 0x00, sizeof(LcdCtrlParam));
	LcdCtrlParam.r2ymd = 3;
	LcdCtrlParam.ckg = 1;
	//LcdCtrlParam.id= 0;
	LcdCtrlParam.iv = 1;
	LcdCtrlParam.ih = 1;
	LcdCtrlParam.ip = 1;
	LcdCtrlParam.clen = 1;
	LcdCtrlParam.r2y = 1;
	LcdCtrlParam.pxdw = 6;
	//LcdCtrlParam.dp = 0;
	//LcdCtrlParam.ni = 0;
	LcdCtrlParam.tv = 1;
	//LcdCtrlParam.opt = 0;
	//LcdCtrlParam.stn = 0;
	//LcdCtrlParam.evsel = 0;
	//LcdCtrlParam.ovp = 0;

	VIOC_DISP_SetControlConfigure(pDISPBase, &LcdCtrlParam);

	VIOC_DISP_SetSize(pDISPBase, width, height);
	VIOC_DISP_SetBGColor(pDISPBase, 0, 0 , 0);

	//VIOC_DISP_TurnOn(pDISPBase);

	VIOC_WMIX_SetOverlayPriority(pWIXBase, 24);
	VIOC_WMIX_SetBGColor(pWIXBase, 0x00, 0x00, 0x00, 0xff);
	VIOC_WMIX_SetSize(pWIXBase, width, height);
	VIOC_WMIX_SetUpdate(pWIXBase);

	//Disconnect LCDC with NTSC/PAL encoder
	BITCLR(pTVE_VEN->VENCON.nREG, HwTVEVENCON_EN_EN);
		
	//Set ECMDA Register
	if(type == LCDC_COMPOSITE_NTSC)
	{
		pTVE->ECMDA.nREG  = 
			HwTVECMDA_PWDENC_PD 			|	// [7]	 Power down mode for entire digital logic of TV encoder
			HwTVECMDA_FDRST_1				|	// [6]	 Chroma is free running as compared to H-sync
			HwTVECMDA_FSCSEL_NTSC			|	// [5:4] Color subcarrier frequency is 3.57954545 MHz for NTSC
			HwTVECMDA_PEDESTAL				|	// [3]	 Video Output has a pedestal (0 is NTSC-J)
			HwTVECMDA_PIXEL_601 			|	// [2]	 Input data is at 601 rates.
			HwTVECMDA_IFMT_525				|	// [1]	 Output data has 525 lines
			HwTVECMDA_PHALT_NTSC			|	// [0]	 NTSC encoded chroma signal output
			0;
	}
	else
	{
		pTVE->ECMDA.nREG  = 
			HwTVECMDA_FDRST_1				|	// [6]	 Chroma is free running as compared to H-sync
			HwTVECMDA_FSCSEL_PALX			|	// [5:4] Color subcarrier frequency is 4.43361875 MHz for PAL-B,D,G,H,I,N
			HwTVECMDA_PIXEL_601 			|	// [2]	 Input data is at 601 rates.
			HwTVECMDA_IFMT_625				|	// [1]	 Output data has 625 lines
			HwTVECMDA_PHALT_PAL 			|	// [0]	 PAL encoded chroma signal output
			0;
	}
	
	//Set DACSEL Register
	BITSET(pTVE->DACSEL.nREG, HwTVEDACSEL_DACSEL_CVBS);
	//Set DACPD Register
	#if defined(TCC892X) || defined(TCC893X)
		BITCLR(pTVE->DACPD.nREG, HwTVEDACPD_PD_EN);
	#else
		BITSET(pTVE->DACPD.nREG, HwTVEDACPD_PD_EN);
	#endif

	BITSET(pTVE->ICNTL.nREG, HwTVEICNTL_VSIP_HIGH);
	BITSET(pTVE->ICNTL.nREG, HwTVEICNTL_HSVSP_RISING);
	#if 0 // COMPOSITE_CCIR656
	BITCSET(pTVE->ICNTL.nREG, HwTVEICNTL_ISYNC_MASK, HwTVEICNTL_ISYNC_ESAV_F);
	#else
	BITCSET(pTVE->ICNTL.nREG, HwTVEICNTL_ISYNC_MASK, HwTVEICNTL_ISYNC_HVSI);
	#endif
		
	//Set the Vertical Offset
	BITCSET(pTVE->HVOFFST.nREG, 0x07, ((0 & 0x700)>>8));
	pTVE->HOFFST.nREG = (0 & 0xFF);
			
	//Set the Horizontal Offset
	BITCSET(pTVE->HVOFFST.nREG, 0x08, ((1 & 0x100)>>5));
	pTVE->VOFFST.nREG = (1 & 0xFF);
			
	//Set the Digital Output Format
	BITCSET(pTVE->HVOFFST.nREG, HwTVEHVOFFST_INSEL_MASK, HwTVEHVOFFST_INSEL(2));
			
	//Set HSVSO Register
	BITCSET(pTVE->HSVSO.nREG, 0x07, ((0 & 0x700)>>8));
	pTVE->HSOE.nREG = (0 & 0xFF);
	BITCSET(pTVE->HSVSO.nREG, 0x38, ((0 & 0x700)>>5));
	pTVE->HSOB.nREG = (0 & 0xFF);
	BITCSET(pTVE->HSVSO.nREG, 0x40, ((0 & 0x100)>>2));
	pTVE->VSOB.nREG = (0 & 0xFF);

	//Set VSOE Register
	BITCSET(pTVE->VSOE.nREG, 0x1F, (0 & 0x1F));
	BITCSET(pTVE->VSOE.nREG, 0xC0, (0 & 0x03)<<6);
	BITCSET(pTVE->VSOE.nREG, 0x20, (0 & 0x01)<<5);
			
	//Set the Connection Type
	BITSET(pTVE_VEN->VENCIF.nREG, HwTVEVENCIF_FMT_1);

	BITSET(pTVE_VEN->VENCON.nREG, HwTVEVENCON_EN_EN);
	#if defined(TCC892X) || defined(TCC893X)
		BITSET(pTVE->DACPD.nREG, HwTVEDACPD_PD_EN);
	#else
		BITCLR(pTVE->DACPD.nREG, HwTVEDACPD_PD_EN);
	#endif
	BITCLR(pTVE->ECMDA.nREG, HwTVECMDA_PWDENC_PD);

	VIOC_DISP_TurnOn(pDISPBase);
#endif //DISPLAY_SUPPORT_COMPOSITE
}
#endif //defined(DEFAULT_DISPLAY_COMPOSITE) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)

#if defined(DEFAULT_DISPLAY_COMPONENT) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
static void lcdc_io_init_component(unsigned char lcdc_num, unsigned char type)
{
#if defined(DISPLAY_SUPPORT_COMPONENT)
	unsigned int width, height;
	stCOMPONENT_SPEC spec;
	stLTIMING		ComponentTiming;
	stLCDCTR		LcdCtrlParam;
	PVIOC_DISP		pDISPBase;
	PVIOC_WMIX		pWIXBase;
	struct fbcon_config *fb_con, *fb_con_splash;
	struct lcd_panel *panel_info;

	dprintf(INFO, "%s, lcdc_num=%d, type=%d\n", __func__, lcdc_num, type);
		
	if(type >= LCDC_COMPONENT_MAX)
		type = defalut_component_resolution;

	#if defined(COMPONENT_CHIP_THS8200)
		#if defined(TARGET_BOARD_STB)
			/* THS8200 Power Control - GPIO_F16 */
			//gpio_set(TCC_GPF(16), 1);
		#else
		#endif
	#endif

	component_get_spec(type, &spec);
	
	#if defined(TARGET_TCC8930ST_EVM)
		#if defined(TARGET_TCC8930_STB1)
		LCDC_IO_Set(lcdc_num, lcdc_num, spec.component_bus_width);
		#else
		LCDC_IO_Set(1, 1, spec.component_bus_width);
		#endif
	#else
		LCDC_IO_Set(lcdc_num, lcdc_num, spec.component_bus_width);
	#endif

	fb_con = &fb_cfg;
			
	if(lcdc_num)	
	{
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP1;
		pWIXBase =(VIOC_WMIX *)HwVIOC_WMIX1; 
		tcc_set_peri(PERI_LCD1, ENABLE, spec.component_clock*spec.component_divider);
		//VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_MRGB, VIOC_OUTCFG_DISP1);
	}
	else
	{
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP0;
		pWIXBase =(VIOC_WMIX *)HwVIOC_WMIX0; 
		tcc_set_peri(PERI_LCD0, ENABLE, spec.component_clock*spec.component_divider);
		//VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_MRGB, VIOC_OUTCFG_DISP0);
	}

	dprintf(INFO, "LCDC0 clk:%u Hz, LCDC1 clk:%u Hz, divide:%d\n", tcc_get_peri(PERI_LCD0), tcc_get_peri(PERI_LCD1), spec.component_divider);
	
	width = spec.component_width;
	height = spec.component_height;

	#if defined(DISPLAY_SPLASH_SCREEN_DIRECT)	
 		lcdc_set_logo(lcdc_num, width, height, fb_con);
	#else
		if(lcdc_num)
			fb_con_splash = &fb1_cfg_splash;
		else
			fb_con_splash = &fb0_cfg_splash;
			
		fb_con_splash->width = MAX_SPLASH_IMAGE_WIDTH;
		fb_con_splash->height = MAX_SPLASH_IMAGE_HEIGHT;
		fb_con_splash->stride = fb_con_splash->width;
		fb_con_splash->bpp = LCDC_FB_BPP;
		fb_con_splash->format = FB_FORMAT_RGB565;

		fb_con_splash->base = dma_alloc(4096, fb_con_splash->width * fb_con_splash->height * (fb_con_splash->bpp/8)); //mem size setting by lcd size
		memset(fb_con_splash->base, 0x00, fb_con_splash->width * fb_con_splash->height * (fb_con_splash->bpp/8));
	      
		if (fb_con_splash->base == NULL)
			dprintf(INFO, "%s, lcdc: framebuffer alloc failed!\n", __func__);
		else
		{
			dprintf(INFO, "%s, splash image: width=%d, height=%d\n", __func__, width, height);
			if(splash_image_load("logo_component", fb_con_splash) < 0)
			{
		 		lcdc_set_logo(lcdc_num, width, height, fb_con);
	 		}
	 		else
	 		{
		 		lcdc_set_logo(lcdc_num, width, height, fb_con_splash);
	 		}
 		}
	#endif

	ComponentTiming.lpw = spec.component_LPW;
	ComponentTiming.lpc = spec.component_LPC + 1;
	ComponentTiming.lswc = spec.component_LSWC + 1;
	ComponentTiming.lewc = spec.component_LEWC + 1;
	
	ComponentTiming.vdb = spec.component_VDB;
	ComponentTiming.vdf = spec.component_VDF;
	ComponentTiming.fpw = spec.component_FPW1;
	ComponentTiming.flc = spec.component_FLC1;
	ComponentTiming.fswc = spec.component_FSWC1;
	ComponentTiming.fewc = spec.component_FEWC1;
	ComponentTiming.fpw2 = spec.component_FPW2;
	ComponentTiming.flc2 = spec.component_FLC2;
	ComponentTiming.fswc2 = spec.component_FSWC2;
	ComponentTiming.fewc2 = spec.component_FEWC2;

	VIOC_DISP_SetTimingParam(pDISPBase, &ComponentTiming);
 
	memset(&LcdCtrlParam, 0x00, sizeof(LcdCtrlParam));

	switch(type)
	{
		case LCDC_COMPONENT_480I_NTSC:
		case LCDC_COMPONENT_576I_PAL:
			break;

		case LCDC_COMPONENT_720P:
			LcdCtrlParam.r2ymd = 3;
			LcdCtrlParam.ckg = 1;
			LcdCtrlParam.id= 0;
			LcdCtrlParam.iv = 1;
			LcdCtrlParam.ih = 1;
			LcdCtrlParam.ip = 0;
			LcdCtrlParam.pxdw = 12;
			LcdCtrlParam.ni = 1;
			break;

		case LCDC_COMPONENT_1080I:
			LcdCtrlParam.r2ymd = 3;
			LcdCtrlParam.ckg = 1;
			LcdCtrlParam.id= 1;
			LcdCtrlParam.iv = 1;
			LcdCtrlParam.ih = 0;
			LcdCtrlParam.ip = 1;
			LcdCtrlParam.pxdw = 12;
			LcdCtrlParam.ni = 0;
			LcdCtrlParam.tv = 1;
			break;

		default:
			break;
	}
	
	VIOC_DISP_SetControlConfigure(pDISPBase, &LcdCtrlParam);

	VIOC_DISP_SetSize(pDISPBase, width, height);
	VIOC_DISP_SetBGColor(pDISPBase, 0, 0 , 0);

	VIOC_WMIX_SetOverlayPriority(pWIXBase, 0);
	VIOC_WMIX_SetBGColor(pWIXBase, 0x00, 0x00, 0x00, 0xff);
	VIOC_WMIX_SetSize(pWIXBase, width, height);
	VIOC_WMIX_SetUpdate(pWIXBase);

	#if defined(TARGET_BOARD_STB)
		/* VE_FIELD: GPIO_E27 */
		gpio_config(TCC_GPE(27), GPIO_FN0|GPIO_OUTPUT|GPIO_HIGH);
	#endif
	
	/* Enable Component Chip */
	#if defined(COMPONENT_CHIP_CS4954)
		if(type == LCDC_COMPONENT_480I_NTSC)
			cs4954_enable(COMPONENT_MODE_NTSC_M); // NTSC_M
		else
			cs4954_enable(COMPONENT_MODE_PAL_B); // PAL_B
	#elif defined(COMPONENT_CHIP_THS8200)
		if(type == LCDC_COMPONENT_720P)
			ths8200_enable(COMPONENT_MODE_720P); // 720P
		else
			ths8200_enable(COMPONENT_MODE_1080I); // 1080I
	#endif

	VIOC_DISP_TurnOn(pDISPBase);
#endif //DISPLAY_SUPPORT_COMPONENT
}
#endif //defined(DEFAULT_DISPLAY_COMPONENT) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)

static struct lcd_panel * lcdc_io_init(unsigned char lcdc_num)
{
	struct lcd_panel *panel;
	unsigned int 	lclk;

#if 0
// reset VIOD
	BITSET(pDDICfg->SWRESET, Hw3);
	BITSET(pDDICfg->SWRESET, Hw2);

	BITCLR(pDDICfg->SWRESET, Hw3);
	BITCLR(pDDICfg->SWRESET, Hw2);	
#endif//

	panel = tccfb_get_panel();
	panel->dev.power_on = GPIO_LCD_ON;
	panel->dev.display_on = GPIO_LCD_DISPLAY;
	panel->dev.bl_on = GPIO_LCD_BL;
	panel->dev.reset = GPIO_LCD_RESET;
	panel->dev.lcdc_num = lcdc_num;
	panel->init(panel);

	if(lcdc_num)
	{
		tcc_set_peri(PERI_LCD1,ENABLE, panel->clk_freq * panel->clk_div);
		lclk  = tcc_get_peri(PERI_LCD1);
	}
	else
	{
		tcc_set_peri(PERI_LCD0,ENABLE, panel->clk_freq * panel->clk_div);
		lclk  = tcc_get_peri(PERI_LCD0);
	}
	printf("telechips %s lcdc:%d clk:%u Hz set clk:%u Hz \n", __func__, lcdc_num, panel->clk_freq, lclk);

	panel->set_power(panel, 1);

	printf("%s end\n", __func__);

	return panel;
}

#if defined(DEFAULT_DISPLAY_LCD)
struct fbcon_config *lcdc_init(void)
{
    
	unsigned LCDC_NUM;

	if(cpu_is_tcc8930)
		LCDC_NUM = 1;
	else
		LCDC_NUM = 0;		

	struct lcd_panel *panel_info;
	struct fbcon_config *fbcon_cfg;
	struct tcc_lcdc_image_update Image_info;

	unsigned int 	lclk;

	panel_info = tccfb_get_panel();
	panel_info->dev.power_on = GPIO_LCD_ON;
	panel_info->dev.display_on = GPIO_LCD_DISPLAY;
	panel_info->dev.bl_on = GPIO_LCD_BL;
	panel_info->dev.reset = GPIO_LCD_RESET;

	panel_info->dev.lcdc_num = LCDC_NUM;

	panel_info->init(panel_info);

	if(panel_info->dev.lcdc_num)
	{
		tcc_set_peri(PERI_LCD1,ENABLE, panel_info->clk_freq * panel_info->clk_div);
		lclk  = tcc_get_peri(PERI_LCD1);
	}
	else
	{
		tcc_set_peri(PERI_LCD0,ENABLE, panel_info->clk_freq * panel_info->clk_div);
		lclk  = tcc_get_peri(PERI_LCD0);
	}
	printf("telechips %s lcdc:%u Hz clk:%u Hz set clk:%d \n", __func__, LCDC_NUM, panel_info->clk_freq, lclk);

	
	dprintf(INFO, "lcdc: panel is %d x %d %dbpp\n",	panel_info->xres, panel_info->yres, fb_cfg.bpp);

	#ifdef DISPLAY_SPLASH_SCREEN_DIRECT
	fb_cfg.stride = fb_cfg.width;
	#else
	fb_cfg.width = panel_info->xres;
	fb_cfg.height = panel_info->yres;
	fb_cfg.stride = fb_cfg.width;
	fb_cfg.bpp = LCDC_FB_BPP;
	fb_cfg.format = FB_FORMAT_RGB565;

	fb_cfg.base = dma_alloc(4096, panel_info->xres * panel_info->yres * (fb_cfg.bpp/8)); //mem size setting by lcd size
	memset(fb_cfg.base, 0x00, panel_info->xres * panel_info->yres * (fb_cfg.bpp/8));
      
	if (fb_cfg.base == NULL)
	  dprintf(INFO, "lcdc: framebuffer alloc failed!\n");    
	#endif//

	dprintf(INFO, "fb_cfg base:0x%x xres:%d yres:%d bpp:%d \n",fb_cfg.base, fb_cfg.width, fb_cfg.height, fb_cfg.bpp);

	Image_info.addr0 = (unsigned int)fb_cfg.base;
	Image_info.Lcdc_layer = 0;

      #if defined(DISPLAY_SPLASH_SCREEN_DIRECT)
	Image_info.enable = 1;
      #else
	Image_info.enable = 0;
      #endif

	Image_info.Frame_width = Image_info.Image_width = fb_cfg.width;
	Image_info.Frame_height = Image_info.Image_height = fb_cfg.height;
	
	printf(INFO, "Frame_width:%d Image_width:%d width:%d \n",Image_info.Frame_width, Image_info.Image_width, fb_cfg.width);
	printf(INFO, "Frame_height:%d Image_height:%d width:%d \n",Image_info.Frame_height, Image_info.Image_height, fb_cfg.height);

	if(Image_info.Image_width > panel_info->xres)
		Image_info.Image_width = panel_info->xres;

	if(panel_info->xres > Image_info.Frame_width)
		Image_info.offset_x = (panel_info->xres - Image_info.Frame_width)/2;
	else
		Image_info.offset_x =  0;

	if(panel_info->yres > Image_info.Frame_height)
		Image_info.offset_y = (panel_info->yres - Image_info.Frame_height)/2;
	else
		Image_info.offset_y = 0;
	
	#ifdef _LCD_32BPP_
	Image_info.fmt = TCC_LCDC_IMG_FMT_RGB888;
	#else
	Image_info.fmt = TCC_LCDC_IMG_FMT_RGB565;
	#endif
	
	tcclcd_image_ch_set(panel_info->dev.lcdc_num, &Image_info);

       panel_info->set_power(panel_info, 1);

	mdelay(1);
	panel_info->set_backlight_level(panel_info, DEFAULT_BACKLIGTH);

	fbcon_cfg = &fb_cfg;

	return fbcon_cfg;
}
#else
struct fbcon_config *lcdc_init(void)
{
	struct fbcon_config *fbcon_cfg;
	unsigned char lcdc_1st = 0; //LCD_CONTROLLER_1
	unsigned char lcdc_2nd = 1; //LCD_CONTROLLER_0
	unsigned int *pConfigMisc1 = (unsigned int *)HwVIOC_CONFIG_MISC1;
	unsigned int config_val;

#if defined(DEFAULT_DISPLAY_COMPONENT) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
	if(tcc_param.output > 3)
	{
		tcc_param.composite_resolution = defalut_composite_resolution;		
		tcc_param.component_resolution = defalut_component_resolution;		

		dprintf(INFO, "%s, composite_resolution:%d, component_resolution:%d\n", __func__, 
				defalut_composite_resolution, defalut_component_resolution);
	}
#endif

	#if defined(TARGET_TCC8920ST_EVM)// || defined(TARGET_TCC8930ST_EVM)
		lcdc_1st = 1; //LCD_CONTROLLER_1
		lcdc_2nd = 0; //LCD_CONTROLLER_0
	#endif

	#if defined(TARGET_TCC8930ST_EVM)
	#if(HW_REV == 0x7030)
		config_val = *pConfigMisc1;
		BITCSET(config_val, Hw29|Hw28, Hw29);	//LCD2_SEL
		BITCSET(config_val, Hw27|Hw26, Hw26);  //LCD1_SEL
		BITCSET(config_val, Hw25|Hw24,    0);	//LCD0_SEL
		*pConfigMisc1 = config_val;
		dprintf(INFO, "%s, CFG_MISC1: 0x%08x\n", __func__, *pConfigMisc1);
	#else	
		config_val = *pConfigMisc1;
		BITCSET(config_val, Hw29|Hw28, Hw29);	//LCD2_SEL
		BITCSET(config_val, Hw27|Hw26, 0);		//LCD1_SEL
		BITCSET(config_val, Hw25|Hw24, Hw24);	//LCD0_SEL
		*pConfigMisc1 = config_val;
		dprintf(INFO, "%s, CFG_MISC1: 0x%08x\n", __func__, *pConfigMisc1);
	#endif
	#endif
	
	#if defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
		#if defined(DISPLAY_STB_AUTO_HDMI_CVBS)
			if(hdmi_detect() == true)
				lcdc_io_init_hdmi(lcdc_1st);
			else
				lcdc_io_init_composite(lcdc_1st, tcc_param.composite_resolution);
		#elif defined(DISPLAY_STB_ATTACH_HDMI_CVBS) || defined(DISPLAY_STB_ATTACH_DUAL_AUTO)
			#if defined(DISPLAY_STB_ATTACH_DUAL_AUTO)
				/* 1st Output Setting : HDMI or COMPONENT */
				if(hdmi_detect())
					lcdc_io_init_hdmi(lcdc_1st);
				else
					lcdc_io_init_component(lcdc_1st, LCDC_COMPONENT_720P + tcc_param.component_resolution);
			#else
				/* 1st Output Setting : HDMI or COMPONENT */
				lcdc_io_init_hdmi(lcdc_1st);
			#endif

			/* 2nd Output Setting : COMPOSITE */
			lcdc_io_init_composite(lcdc_2nd, tcc_param.composite_resolution);
		#else
			#if defined(TARGET_TCC8930_EV) || defined(TARGET_TCC8930_STB1)
				/* 1st Output Setting : HDMI or COMPONENT */
				if(hdmi_detect() == true)
					lcdc_io_init_hdmi(lcdc_1st);
				else
					lcdc_io_init_component(lcdc_1st, LCDC_COMPONENT_720P + tcc_param.component_resolution);
				/* 2nd Output Setting : COMPOSITE */
				lcdc_io_init_composite(lcdc_2nd, tcc_param.composite_resolution);
			#elif defined(TARGET_TCC8935_STICK)
				/* 1st Output Setting : HDMI */
				lcdc_io_init_hdmi(lcdc_1st);
			#endif
 		#endif
	#else
		lcdc_io_init_hdmi(lcdc_1st);
	#endif

	fbcon_cfg = &fb_cfg;

	mdelay(50);
	
	return fbcon_cfg;
}
#endif


int display_splash_logo(struct fbcon_config *fb_con)
{
#if !defined(DISPLAY_SPLASH_SCREEN_DIRECT)		
	struct lcd_panel *panel_info;
	panel_info = tccfb_get_panel();
	lcdc_set_logo(panel_info->dev.lcdc_num, panel_info->xres, panel_info->yres, fb_con);
#endif
	return 0;
}

