/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "config.h"
#include "debug.h"

#include "tcc_chip_rev.h"
#include <platform/tcc_ckc.h>
#include <platform/reg_physical.h>
#if defined(TCC_HDMI_DRIVER_V1_3)
#include <hdmi_v1_3_hdmi.h>
#elif defined(TCC_HDMI_DRIVER_V1_4)
#include <hdmi_v1_4_hdmi.h>
#endif

static unsigned int tcc_chip_rev = -1;

extern void disp_init_hdmi(void);

void tcc_extract_chip_revision(void)
{
#if defined(TCC_HDMI_DRIVER_V1_4)
#define DDIBUS_HDCP_SEL_BIT	(8)
#define DDIBUS_HDMI_MASK		(0xF)
#define DDIBUS_HDMI_SELECT		(0x7)

	unsigned long BackupDDICFG, BackupHDMICTRL, Reg_HDMIV, Reg_HDCPV;
	unsigned loop_cnt = 0, phy_loop = 0;
	DDICONFIG *pDDIBUSCFG = (DDICONFIG *)(HwDDI_CONFIG_BASE);
	HDMICORE *pHDMICORE =(HDMICORE *)(HwHDMI_CORE_BASE);
	

	BackupHDMICTRL = pDDIBUSCFG->HDMI_CTRL.nREG;
	BackupDDICFG = pDDIBUSCFG->PWDN.nREG;
	

	hdmi_ddi_config_init();
	PHYConfig(PHY_FREQ_148_352, HDMI_CD_24);

	pDDIBUSCFG->PWDN.nREG &= ~(DDIBUS_HDMI_MASK << DDIBUS_HDCP_SEL_BIT);

	while(!(pHDMICORE->PHY_STATUS.nREG )& 0x1)
	{
		phy_loop++;
		if(phy_loop > 100000)
			break;
	}
      
start_rev:
	loop_cnt++;
	
	pDDIBUSCFG->PWDN.nREG &= ~(DDIBUS_HDMI_MASK << DDIBUS_HDCP_SEL_BIT);
	
	// HDMI core register write
	pDDIBUSCFG->PWDN.nREG &= ~(DDIBUS_HDMI_SELECT << DDIBUS_HDCP_SEL_BIT);
	pHDMICORE->HDMI_CON_0.nREG = 0x10;

	// HDCP core register write
	pDDIBUSCFG->PWDN.nREG |= (DDIBUS_HDMI_SELECT << DDIBUS_HDCP_SEL_BIT);
	pHDMICORE->HDMI_CON_0.nREG = 0x28;

	// HDMI core register Read
	pDDIBUSCFG->PWDN.nREG &= ~(DDIBUS_HDMI_SELECT << DDIBUS_HDCP_SEL_BIT);
	Reg_HDMIV =  pHDMICORE->HDMI_CON_0.nREG;

	// HDCP core register Read
	pDDIBUSCFG->PWDN.nREG |= (DDIBUS_HDMI_SELECT << DDIBUS_HDCP_SEL_BIT);
	Reg_HDCPV =  pHDMICORE->HDMI_CON_0.nREG;

	if(Reg_HDMIV == 0 && (loop_cnt <= 100))
	{
		goto	start_rev;
	}
		
	pDDIBUSCFG->HDMI_CTRL.nREG = BackupHDMICTRL;
	pDDIBUSCFG->PWDN.nREG = BackupDDICFG;

       printf("%s : Reg_HDMIV:0x%x Reg_HDCPV:0x%x BackupDDICFG:0x%x BackupHDMICTRL:0x%x loop:%d phy:%d \n", 
                  __func__,Reg_HDMIV,  Reg_HDCPV,BackupDDICFG, BackupHDMICTRL, loop_cnt, phy_loop);

	if(Reg_HDMIV == Reg_HDCPV)
		tcc_chip_rev = 0xA;
	else
		tcc_chip_rev = 0xB;

	printf("Chipset revision = 0x%x\n", tcc_chip_rev);
#else
        tcc_chip_rev = 0xB;

        printf("Chipset revision = 0x%x\n", tcc_chip_rev);
#endif
}

unsigned int tcc_get_chip_revision(void)
{
	return tcc_chip_rev;
}

