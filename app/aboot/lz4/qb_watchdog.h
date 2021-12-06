//+[TCCQB] WatchDog Control for QuickBoot
/*===================================================================
 *																	*
 *		 			WatchDog Setting Functions.						*
 *																	*
 *=================================================================*/


static volatile unsigned int cnt = 0;
#define nop_delay(x) for(cnt=0 ; cnt<x ; cnt++){ \
	__asm__ __volatile__ ("nop\n"); }


#if defined(CONFIG_QB_WATCHDOG_ENABLE)	// ================================================

//////////////////////////////////////////////////////////////////////
//							INCLUDES								//
//////////////////////////////////////////////////////////////////////
#include <platform/iomap.h>
#include <reg.h>



//////////////////////////////////////////////////////////////////////
//						Register Control 							//
//////////////////////////////////////////////////////////////////////
#define IO_OFFSET	0x00000000
#define io_p2v(pa)	((pa) - IO_OFFSET)
#define tcc_p2v(pa)         io_p2v(pa)

#define wdt_readl       readl
#define wdt_writel      writel

#define reg_set(x,MASK)  wdt_writel(wdt_readl(x)|((uint32_t)(MASK)), x);  // Set register x to MASK.
#define reg_clr(x,MASK)  wdt_writel(wdt_readl(x)&(~((uint32_t)(MASK))), x); // Clear reg x to MASK.


//////////////////////////////////////////////////////////////////////
//						WATCHDOG Registers							//
//////////////////////////////////////////////////////////////////////
#define WDT_EN          (1<<31)
#define WDT_CLR         (1<<30)
#define WDT_EN_STS      (1<<24)
#define WDT_CLR_STS     (1<<23)
#define WDT_CNT_MASK    (0xFFFF)

#define TWDCFG          0x00
#define TWDCLR          0x04
#define TWDCNT          0x08

#define TWDCFG_EN       (1<<0)
#define TWDCFG_IEN      (1<<3)
#define TWDCFG_TCLKSEL_SHIFT    4
#define TWDCFG_TCLKSEL_MASK (7<<4)
#define TWDCFG_TCLKSET_4    32
#define TWDCFG_TCLKSET_5    1024
#define TWDCFG_TCLKSET_6    4096

#define TCCWDT_CNT_MASK_UNIT	(WDT_RATE/65536)



//////////////////////////////////////////////////////////////////////
//							PLATFORM								//
//////////////////////////////////////////////////////////////////////
#if defined(PLATFORM_TCC893X)	// =================== Platform ===================
	#define TCC_WDT_TIMER_BASE      0x74300070      // TWDCFG

	#define WDT_RATE		(24*1000*1000)	// PMU Clock : 24M
	#define PMU_REMAP_VALUE	0x30000000		// 29, 28 bit
	
	#define PMU_CONFIG		0x10
	#define PMU_WDTCTRL		0x04

#elif defined(PLATFORM_TCC896X)	// =================== Platform ===================
	#define TCC_WDT_TIMER_BASE      0x14300070      // TWDCFG

	#define WDT_RATE		(24*1000*1000)	// clock-frequency : 24M
//	#define PMU_REMAP_VALUE	0x3C000000		// EVT0 - Cortex-A15 : 29, 28 bit / Cortex-A7 : 27,26 bit
	#define PMU_REMAP_VALUE	0x3F000000		// EVT1 - Cortex-A15 : 29~27 bit / Cortex-A7 : 26~24 bit

	#define PMU_CONFIG		0x14
	#define PMU_ISOL		0x9C
//	#define PWRUP_MBUS		0x64	
	#define PWRUP_CMBUS		0xE8	
	#define PWRUP_DMBUS0	0xEC	
	#define PWRUP_DMBUS1	0xF0	
	#define PWRUP_VBUS		0xF4
	#define PWRUP_GBUS		0xF8
	#define PWRUP_DBUS		0xFC
	#define PWRUP_HSBUS		0x100
	#define PMU_SYSRST		0x10
	#define PMU_WDTCTRL		0x08
	#define TWDCFG_TCLKSEL_MASK (7<<4)

#elif defined(PLATFORM_TCC897X)	// =================== Platform ===================
	#define TCC_WDT_TIMER_BASE      0x74300070      // TWDCFG

	#define WDT_RATE		(24*1000*1000)	// clock-frequency : 24M
	#define PMU_REMAP_VALUE	0x30000000		// 29~28 : 0b00

	#define PMU_CONFIG		0x14
	#define PMU_ISOL		0x9C
//	#define PWRUP_MBUS		0x64	
	#define PWRUP_CMBUS		0xE8	
	#define PWRUP_DMBUS0	0xEC	
	#define PWRUP_DMBUS1	0xF0	
	#define PWRUP_VBUS		0xF4
	#define PWRUP_GBUS		0xF8
	#define PWRUP_DBUS		0xFC
	#define PWRUP_HSBUS		0x100
	#define PMU_SYSRST		0x10
	#define PMU_WDTCTRL		0x08
	#define TWDCFG_TCLKSEL_MASK (7<<4)
#endif	 // =========================================== Platform ===================



//////////////////////////////////////////////////////////////////////
//							DEFINES									//
//////////////////////////////////////////////////////////////////////
#define DEFAULT_TIMEOUT		10		// sec

//#define KICK_ERR

//////////////////////////////////////////////////////////////////////
//							FUNCTIONS								//
//////////////////////////////////////////////////////////////////////

static void __iomem *pmu_reg;
static void __iomem *wdt_ctrl_base;
static void __iomem *wdt_timer_base;

static uint32_t wdt_cont = 0;

static inline void watchdog_clear(void)
{
	/*	TCC896x -
	 *	If set WATCHDOG Clear bit, watchdog reboot is not work!		
	 *	So, re-init watchdog timer counter instead of setting WatchDog Clear bit.
	 *	*/
#ifndef KICK_ERR
	reg_set(wdt_ctrl_base, WDT_CLR);		// Clear WatchDog bit -> 1
	reg_clr(wdt_ctrl_base, WDT_CLR);		// Clear WatchDog bit -> 0
	reg_set(wdt_ctrl_base, WDT_EN);			// Enable WatchDog
#else
	if (wdt_cont == 0)						// invalid value
		return;

	reg_clr(wdt_ctrl_base, WDT_EN);			// Disable WatchDog
	reg_clr(wdt_ctrl_base, WDT_CNT_MASK);	// Clear WatchDog Counter
	reg_set(wdt_ctrl_base, wdt_cont);		// Set WatchDog Timer Counter.
	reg_set(wdt_ctrl_base, WDT_EN);			// Enable WatchDog
#endif
}

/*		Initialize WatchDog		*/
static inline void watchdog_init(void)
{
	int wdt_timeout = 0;					// Sec.

	pmu_reg = (void __iomem *)TCC_PMU_BASE;
	wdt_ctrl_base = pmu_reg + PMU_WDTCTRL;
	wdt_timer_base = (void __iomem *)TCC_WDT_TIMER_BASE;

	/* 		remap internal ROM 		*/
//	reg_clr(pmu_reg + PMU_CONFIG, PMU_REMAP_VALUE);	// PMU Remap (Boot-ROM)
	
#if 0
	/* 	PMU is not initialized with WATCHDOG 	*/
	wdt_writel(0x0, pmu_reg + PMU_ISOL);	// DATA
	nop_delay(100000);
//	wdt_writel(0x0, pmu_reg + PWRUP_MBUS);	// DATA - Cannot find this register....
	nop_delay(100000);
	wdt_writel(0x0, pmu_reg + PWRUP_CMBUS); // DATA
	nop_delay(100000);
	wdt_writel(0x0, pmu_reg + PWRUP_DMBUS0); // DATA
	nop_delay(100000);
	wdt_writel(0x0, pmu_reg + PWRUP_DMBUS1); // DATA
	nop_delay(100000);
	wdt_writel(0x0, pmu_reg + PWRUP_VBUS);	// DATA
	nop_delay(100000);
	wdt_writel(0x0, pmu_reg + PWRUP_GBUS);	// DATA
	nop_delay(100000);
	wdt_writel(0x0, pmu_reg + PWRUP_DBUS);	// DATA
	nop_delay(100000);
	wdt_writel(0x0, pmu_reg + PWRUP_HSBUS);// DATA
	nop_delay(100000);

	reg_set(pmu_reg + PMU_SYSRST, 0x1 << 30); // VB
	reg_set(pmu_reg + PMU_SYSRST, 0x1 << 28); // GB
	reg_set(pmu_reg + PMU_SYSRST, 0x1 << 29); // DB
	reg_set(pmu_reg + PMU_SYSRST, 0x1 << 31); // HSB
//	reg_set(pmu_reg + PMU_SYSRST, 0x1 << 4); // Reset Output Signal
#endif
	nop_delay(100000);

	wdt_timeout = CONFIG_WATCHDOG_TIMEOUT;	// Sec.
	if (wdt_timeout*TCCWDT_CNT_MASK_UNIT > WDT_CNT_MASK) {
		dprintf(CRITICAL, "WatchDog TimeOut %dms is invalid.\n", wdt_timeout);
		wdt_timeout = DEFAULT_TIMEOUT;	
		dprintf(CRITICAL, "Set WatchDog to default [%d ms]\n", wdt_timeout);
	}
	wdt_cont = wdt_timeout*TCCWDT_CNT_MASK_UNIT;	// WatchDog Timer Counter Value.

#ifndef KICK_ERR
	reg_clr(wdt_ctrl_base, WDT_EN);			// Disable WatchDog
	reg_clr(wdt_ctrl_base, WDT_CNT_MASK);	// Clear WatchDog Counter
	reg_set(wdt_ctrl_base, wdt_cont);		// Set WatchDog Timer Counter.
	reg_set(wdt_ctrl_base, WDT_EN);			// Enable WatchDog
#else
	watchdog_clear();						// init watchdog timer counter.
#endif

	dprintf(CRITICAL, "QB WatchDog TimeOut : %dsec\n", wdt_timeout);
	nop_delay(100000);
}
static inline void watchdog_disable(void)
{
	reg_clr(wdt_ctrl_base, WDT_EN);			// Disable WatchDog
}




#else	// !CONFIG_QB_WATCHDOG_ENABLE // ===============================================

/*		Initialize WatchDog		*/
static inline void watchdog_init(void) { dprintf(CRITICAL, "QB WatchDog is Disabled.\n"); }
static inline void watchdog_clear(void) { }
static inline void watchdog_disable(void) { }

#endif	// ==============================================================================
//-[TCCQB]
//



