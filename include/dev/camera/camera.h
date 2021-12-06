/****************************************************************************
 *   FileName    : camera.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, 
including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, 
copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information's accuracy, 
completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, 
out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/

#ifndef __CAMERA_H__
#define	__CAMERA_H__

#include <viocmg.h>

#define ON		1
#define OFF		0

#define ENABLE		1
#define DISABLE		0

#define ACT_HIGH	1
#define ACT_LOW		0

#define NEGATIVE_EDGE	1
#define POSITIVE_EDGE	0

#define	MAILBOX_MSG_PREAMBLE				((unsigned int)0x0043414D)

#define MAILBOX_MSG_EARLYCAMERA_READY		((unsigned int)0x00000831)
#define MAILBOX_MSG_EARLYCAMERA_STOP		((unsigned int)0x00000051)
#define MAILBOX_MSG_EARLYCAMERA_EXIT		((unsigned int)0x00000052)
#define MAILBOX_MSG_EARLYCAMERA_PGL 		((unsigned int)0x00000053)
#define MAILBOX_MSG_EARLYCAMERA_PMAP		((unsigned int)0x00000054)
#define MAILBOX_MSG_EARLYCAMERA_KNOCK		((unsigned int)0x000000FF)
#define MAILBOX_MSG_ACK						((unsigned int)0x10000000)

typedef struct cm_ctrl_msg_t {
	unsigned int cmd;
	unsigned int data[6];
	unsigned int preamble;
} cm_ctrl_msg_t;

#define CONFIG_TCC_PARKING_GUIDE_LINE_NAME "parkingline_888"

struct tcc_cif_parameters
{
	//Camera Parameters
	int Cam_p_clock_pol;
	int Cam_v_sync_pol;
	int Cam_h_sync_pol;
	int Cam_de_pol;
	int Cam_field_bfield_low;
	int Cam_gen_field_en;
	int Cam_conv_en;
	int Cam_hsde_connect_en;
	int Cam_vs_mask;
	int Cam_input_fmt;
	int Cam_data_order;
	int Cam_intl_en;
	int Cam_intpl_en;
	int Cam_format;
	int Cam_preview_w;
	int Cam_preview_h;

	//LCD Parameters
//	int Lcdc_Frame_width;
//	int Lcdc_Frame_height;
//	int Lcdc_Image_width;
//	int Lcdc_Image_height;
//	int Lcdc_offset_x;
//	int Lcdc_offset_y;
	unsigned int Lcdc_address0;
	unsigned int Lcdc_address1;
	unsigned int Lcdc_address2;
//	int Lcdc_format;

	int CIF_Port_num;
	
	unsigned int Viqe_area;
	unsigned int PGL_addr;
	
	unsigned int Camera_type;

	struct viocmg_info viocmg_info;
};

extern int stopEarlyCamera(void);
extern int startEarlyCamera(void);
extern void tcc_cif_delay(int ms);

extern void parking_guide_clear(void);
extern unsigned int load_parking_guide(void);

#endif

