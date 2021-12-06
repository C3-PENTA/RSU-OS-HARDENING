/*
 * Copyright (c) 2011 Telechips, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <platform/reg_physical.h>
#include <platform/tcc_ckc.h>
#include <plat/cpu.h>
#include <clock.h>
#include <power.h>
#include <i2c.h>

/************************************************************
* Function    : clock_init()
* Description :
*    - increase fbus clock (1.2V or higher level)
************************************************************/
void clock_init(void)
{
#if defined(AXP192_PMIC)
	#if defined(CONFIG_SNAPSHOT_BOOT)
		tcc_set_voltage(  PWR_CPU, ENABLE, 1250);	// CPU FREQ : 1Ghz
/*
		#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 850)
            tcc_set_voltage(  PWR_CPU, ENABLE, 1175);
		#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 910)
            tcc_set_voltage(  PWR_CPU, ENABLE, 1200);
		#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 1000)
            tcc_set_voltage(  PWR_CPU, ENABLE, 1250);
		#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 1060)
            tcc_set_voltage(  PWR_CPU, ENABLE, 1300);
		#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 1112)
            tcc_set_voltage(  PWR_CPU, ENABLE, 1350);
		#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 1152)
            tcc_set_voltage(  PWR_CPU, ENABLE, 1400);
		#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 1200)
            tcc_set_voltage(  PWR_CPU, ENABLE, 1200);
		#else
			#error
		#endif
*/
	#else
        tcc_set_voltage(  PWR_CPU, ENABLE, 1200);
	#endif
    tcc_set_voltage( PWR_CORE, ENABLE, 1175);
    tcc_set_voltage( PWR_SDIO, ENABLE, 3000);
    tcc_set_voltage( PWR_IOD0, ENABLE, 3300);
    tcc_set_voltage( PWR_IOD1, ENABLE, 3300);

#elif defined(TCC270_PMIC)
    tcc_set_voltage(  PWR_CPU, ENABLE, 1200);
    tcc_set_voltage( PWR_CORE, ENABLE, 1175);
    tcc_set_voltage(  PWR_MEM, ENABLE, 1500);
    tcc_set_voltage( PWR_IOD0, ENABLE, 3000);
    tcc_set_voltage( PWR_IOD1, ENABLE, 3000);
    tcc_set_voltage( PWR_IOD2, ENABLE, 3300);
    tcc_set_voltage( PWR_HDMI_12D, ENABLE, 3300);
    tcc_set_voltage( PWR_HDMI_33D, ENABLE, 3300);
    tcc_set_voltage( PWR_BOOST_5V, ENABLE, 3300);
#endif

#if defined(CONFIG_SNAPSHOT_BOOT)
	tcc_set_clkctrl( FBUS_CPU,    ENABLE, 1000000000);	// CPU FREQ : 1Ghz
/*
	#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 850)
		tcc_set_clkctrl( FBUS_CPU,    ENABLE, 850000000);	//FBUS_CPU      850 MHz 	// 1.2V
	#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 910)
		tcc_set_clkctrl( FBUS_CPU,    ENABLE, 910000000);
	#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 1000)
		tcc_set_clkctrl( FBUS_CPU,    ENABLE, 1000000000);
	#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 1060)
		tcc_set_clkctrl( FBUS_CPU,    ENABLE, 1060000000);
	#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 1112)
		tcc_set_clkctrl( FBUS_CPU,    ENABLE, 1112000000);
	#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 1152)
		tcc_set_clkctrl( FBUS_CPU,    ENABLE, 1152000000);
	#elif (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK <= 1200)
		tcc_set_clkctrl( FBUS_CPU,    ENABLE, 1200000000);
	#else
		#error
	#endif
*/
#else
	tcc_set_clkctrl( FBUS_CPU,   ENABLE, 850000000);	/*FBUS_CPU      850 MHz */	// 1.2V
#endif

#if defined(DRAM_DDR3)
//	tcc_set_clkctrl( FBUS_MEM,   ENABLE, 600000000);	/*FBUS_MEM      600 MHz * no need change dram clock/
#else
//	tcc_set_clkctrl( FBUS_MEM,   ENABLE, 300000000);	/*FBUS_MEM      300 MHz */
#endif
	tcc_set_clkctrl( FBUS_DDI,   ENABLE, 333333334);	/*FBUS_DDI      333 MHz */
	tcc_set_clkctrl( FBUS_GPU,  DISABLE, 333333334);	/*FBUS_GRP      384 MHz */
	tcc_set_clkctrl( FBUS_IO,    ENABLE, 180000000);	/*FBUS_IOB      180 MHz */
	tcc_set_clkctrl( FBUS_VBUS, DISABLE, 300000000);	/*FBUS_VBUS     300 MHz */
	tcc_set_clkctrl( FBUS_CODA, DISABLE, 300000000);	/*FBUS_VCODEC   300 MHz */
	tcc_set_clkctrl( FBUS_HSIO,  ENABLE, 250000000);	/*FBUS_HSIO     250 MHz */
	tcc_set_clkctrl( FBUS_SMU,   ENABLE, 180000000);	/*FBUS_SMU      180 MHz */
	if (!cpu_is_tcc8933s() && !cpu_is_tcc8935s() && !cpu_is_tcc8937s()) {
		tcc_set_clkctrl( FBUS_G2D,   DISABLE, 333333334);	/*FBUS_G2D      333 MHz */
		tcc_set_clkctrl( FBUS_CMBUS, DISABLE, 250000000);	/*FBUS_CM3      250 MHz */
	}
}

/************************************************************
* Function    : clock_init_early()
* Description :
*    - set pll/peri-clock and
*    - set fbus clock to low (1.0V level)
************************************************************/
void clock_init_early(void)
{
	tcc_ckc_init();

	/* change clock sourt to XIN before change PLL values. */
	tcc_set_clkctrl(FBUS_IO, ENABLE, 6000000);	/*FBUS_IO		6 MHz */
	tcc_set_clkctrl(FBUS_SMU, ENABLE, 6000000);	/*FBUS_SMU		6 MHz */
#if defined(TSBM_ENABLE)
	tcc_set_clkctrl(FBUS_HSIO, ENABLE,   6000000);	/*FBUS_HSIO 	6 MHz */
#endif

	/* Set fixed pll values  pll0:cpu only, pll1:set by sdram */
	if (cpu_is_tcc8933s() || cpu_is_tcc8935s() || cpu_is_tcc8937s()) {
		tcc_set_pll(PLL_2,  ENABLE, 1000000000, PLLDIV_0);
		tcc_set_pll(PLL_3,  ENABLE,  720000000, PLLDIV_0);
		tcc_set_pll(PLL_4, DISABLE,  594000000, PLLDIV_0);	/* Not Used */
		tcc_set_pll(PLL_5, DISABLE,  720000000, PLLDIV_0);	/* Not Used */
	}
	else {
		tcc_set_pll(PLL_2,  ENABLE, 1000000000, PLLDIV_0);
		tcc_set_pll(PLL_3,  ENABLE,  768000000, PLLDIV_0);
		tcc_set_pll(PLL_4,  ENABLE,  594000000, PLLDIV_0);
		tcc_set_pll(PLL_5,  ENABLE,  720000000, PLLDIV_0);
	}
}
