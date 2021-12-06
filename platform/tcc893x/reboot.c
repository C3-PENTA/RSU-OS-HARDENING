/*
 * Copyright (C) 2010 Telechips, Inc.
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

#include <reg.h>
#include <platform/iomap.h>
#include <debug.h>

#define IOBUSCFG_HCLKEN0        (TCC_IOBUSCFG_BASE + 0x10)
#define IOBUSCFG_HCLKEN1        (TCC_IOBUSCFG_BASE + 0x14)
#define PMU_WATCHDOG            (TCC_PMU_BASE + 0x04)
#define PMU_CONFIG1             (TCC_PMU_BASE + 0x10)
#define PMU_USSTATUS            (TCC_PMU_BASE + 0x18)

void reboot(unsigned reboot_reason)
{
	unsigned int usts;

	if (reboot_reason == 0x77665500) {
		usts = 1;
	} else {
		usts = 0;
	}

	writel(usts, PMU_USSTATUS);

	writel((readl(PMU_CONFIG1) & 0xCFFFFFFF), PMU_CONFIG1);

	writel(0xFFFFFFFF, IOBUSCFG_HCLKEN0);
	writel(0xFFFFFFFF, IOBUSCFG_HCLKEN1);

	while (1) {
		writel((1 << 31) | 1, PMU_WATCHDOG);
	}
}

unsigned check_reboot_mode(void)
{
	unsigned int usts;
	unsigned int mode;

	/* XXX: convert reboot mode value because USTS register
	 * hold only 8-bit value
	 */
	usts = readl(PMU_USSTATUS);
	printf("usts(0x%08X)\n", usts);
	switch (usts) {
	case 1:
		mode = 0x77665500;	/* fastboot mode */
		break;

	case 2:
		mode = 0x77665502;	/* recovery mode */
		break;

//+[TCCQB] For QuickBoot Booting Option
	case 3:
		mode = 0x77665503;      /* force normal mode : skip quickboot */
		break;
//-[TCCQB]
//

	default:
		mode = 0x77665501;
		break;
	}

	dprintf(SPEW, "reboot mode = 0x%x\n", mode);
	return mode;
}

unsigned is_reboot_mode(void)
{
	return check_reboot_mode();
}

//+[TCCQB] Send QB LK Boot Time to kernel
void set_lk_boottime(unsigned int usts)
{
	/*       Send LK Total Time to Kernel    
	 *       usts : mili second time		*/
	
	writel(usts, PMU_USSTATUS);
}
//-[TCCQB]
//
