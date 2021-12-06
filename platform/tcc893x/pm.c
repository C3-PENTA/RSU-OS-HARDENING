/****************************************************************************
 * platform/tcc893x//pm.c
 * Copyright (C) 2014 Telechips Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions andlimitations under the License.
 ****************************************************************************/

#include <arch/ops.h>
#include "config.h"
#include "debug.h"
#include <platform/reg_physical.h>
#include <platform/pm.h>
#include "tcc_ddr.h"
#include <platform/sram_map.h>

#define tcc_p2v(pa)     (pa)

extern int check_fwdn_mode(void);

extern unsigned int IO_ARM_ChangeStackSRAM(void);
extern void IO_ARM_RestoreStackSRAM(unsigned int);

static TCC_REG RegRepo;

#if defined(TARGET_BOARD_STB)
    #define CONFIG_MACH_TCC8930ST
    #define CONFIG_INPUT_TCC_REMOTE
    #if defined(TARGET_TCC8935_STICK)
        #define CONFIG_STB_BOARD_DONGLE_TCC893X
    #endif
#endif

/*===========================================================================

                  DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/

#define nop_delay(x) for(int cnt=0 ; cnt<x ; cnt++){ \
                    __asm__ __volatile__ ("nop\n"); }

#define addr_clk(b) (0x74000000+b)
#define addr_mem(b) (0x73000000+b)

#if defined(MEMBUS_CLK_AUTO_RESTORE)
#define SDRAM_INIT_PARAM(x)   (*(volatile unsigned long *)(SDRAM_INIT_PARAM_ADDR + (4*(x))))
enum {
    PLL_VALUE = 0,
    DDR_TCK,
};

typedef int (*AssemFuncPtr)(unsigned int dst, unsigned int src);
unsigned int tcc_pm_time2cycle(unsigned int time, unsigned int tCK)
{
    int cnt;
    if (time == 0)
        return 0;
    time = time + tCK - 1;
    cnt = 0;
    do {
        cnt++;
        time -= tCK;
    } while (time >= tCK);
    return cnt;
}
#else
#define tcc_pm_time2cycle(time, tCK)        ((int)((time + tCK - 1) / tCK))
#endif

#define denali_ctl(x)   (*(volatile unsigned long *)addr_mem(0x500000+(x*4)))
#define denali_phy(x)   (*(volatile unsigned long *)addr_mem(0x600000+(x*4)))
#define ddr_phy(x)      (*(volatile unsigned long *)addr_mem(0x420400+(x*4)))

typedef void (*FuncPtr)(void);

#define OLD_SETTING
#define INIT_CONFIRM
#define PHY_MRS_SETTING
#define IMPROVE_DDI_PERFORMANCE
//#define DQS_EXTENSION
//#define ZDEN
//#define AUTO_TRAIN_CK_CHANGE
//#define CONFIG_DRAM_AUTO_TRAINING
//#define MANUAL_TRAIN
//#define MEM_BIST

/*===========================================================================
FUNCTION
===========================================================================*/
#if defined(CONFIG_MACH_TCC8930ST)
static inline void tcc_stb_suspend(void)
{
#if defined(TCC_PM_MEMQ_PWR_CTRL)
    BITCLR(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<24); //VDDQ_MEM_ON : low
#endif    

	BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<9); //SLEEP_MODE_CTL : low
    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<22); //PLL25_ON : low

    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<19); //CORE1_ON : low
    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<21); //CORE2_ON : low

    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<24); /* LED_S_PN */
    BITCLR(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<27); /* PHY1_ON */
}

static inline void tcc_stb_resume(void)
{
#if defined(TCC_PM_MEMQ_PWR_CTRL)
    BITSET(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<24); //VDDQ_MEM_ON : high
#endif        
	BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<9); //SLEEP_MODE_CTL : high
    BITSET(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<22); //PLL25_ON : low

    BITSET(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<19); //CORE1_ON : high
    BITSET(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<21); //CORE2_ON : high    

    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<24); /* LED_S_PN */
    BITSET(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<27); /* PHY1_ON */
}
#endif


void tcc_pm_sleep(void)
{
    volatile unsigned int cnt = 0;
    volatile unsigned tmp;

    /* REMAP 0xF0000000 area to SRAM */
    *(volatile unsigned *)0x73810010 |= 0x80000000;

    /* mmu & cache off */
    __asm__ volatile (
    "mrc p15, 0, r0, c1, c0, 0 \n"
    "bic r0, r0, #(1<<12) \n"       //ICache
    "bic r0, r0, #(1<<2) \n"        //DCache
    "bic r0, r0, #(1<<0) \n"        //MMU
    "mcr p15, 0, r0, c1, c0, 0 \n"
    "nop \n"
    );

#if defined(CONFIG_DRAM_DDR3)
    /* holding to access to dram */
    denali_ctl(47) = 0xFFFFFFFF;
    BITSET(denali_ctl(44),0x1);             // inhibit_dram_cmd=1
    while(!(denali_ctl(46)&(0x1<<15)));     // wait for inhibit_dram_cmd
    BITSET(denali_ctl(47), 0x1<<15);

    /* enter self-refresh */
    BITSET(denali_phy(13), 0x1<<10);                        // lp_ext_req=1
    while(!(denali_phy(13)&(0x1<<26)));                     // until lp_ext_ack==1
    BITCSET(denali_phy(13), 0x000000FF, (2<<2)|(1<<1)|(0));
    BITSET(denali_phy(13), 0x1<<8);                         // lp_ext_cmd_strb=1
    while((denali_phy(13)&(0x003F0000)) != (0x25<<16));     // until lp_ext_state==0x25 : check self-refresh state
    BITCLR(denali_phy(13), 0x1<<8);                         // lp_ext_cmd_strb=0
    BITCLR(denali_phy(13), 0x1<<10);                        // lp_ext_req=0
    while(denali_phy(13)&(0x1<<26));                        // until lp_ext_ack==0
    BITSET(denali_ctl(96), 0x3<<8);                         // DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x3
    BITCLR(denali_ctl(0),0x1);                              // START[0] = 0

    ddr_phy(0x0E) = 0xffffffff;     // AC IO Config
#elif defined(CONFIG_DRAM_DDR2)
    #error "DDR2 is not implemented"
#endif

    nop_delay(1000);

    /* Clock <- XIN, */
    ((PCKC)HwCKC_BASE)->CLKCTRL0.nREG = 0x002ffff4;     // CPU
    ((PCKC)HwCKC_BASE)->CLKCTRL1.nREG = 0x00200014;     // Memory Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL2.nREG = 0x00000014;     // DDI Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL3.nREG = 0x00000014;     // Graphic Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL4.nREG = 0x00200014;     // I/O Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL5.nREG = 0x00000014;     // Video Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL6.nREG = 0x00000014;     // Video core
    ((PCKC)HwCKC_BASE)->CLKCTRL7.nREG = 0x00000014;     // HSIO BUS
    ((PCKC)HwCKC_BASE)->CLKCTRL8.nREG = 0x00200014;     // SMU Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL9.nREG = 0x00000014;     // G2D BUS
    ((PCKC)HwCKC_BASE)->CLKCTRL10.nREG = 0x00000014;    // CM3 Bus

    /* PLL <- OFF */
    ((PCKC)HwCKC_BASE)->PLL0CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL1CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL2CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL3CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL4CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL5CFG.nREG &= ~0x80000000;

#if defined(CONFIG_MACH_TCC8930ST)
    tcc_stb_suspend();
#endif

    /* SRAM Retention */
    BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<16); //PD_RETN : SRAM retention mode=0

    /* SSTL & IO Retention */
    while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8))
        BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode =0
    while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4))
        BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode =0

// -------------------------------------------------------------------------
// set wake-up
//Bruce, wake-up source should be surely set here !!!!

    /* set wake-up trigger mode : edge */
    ((PPMU)HwPMU_BASE)->PMU_WKMOD0.nREG = 0xFFFFFFFF;
    ((PPMU)HwPMU_BASE)->PMU_WKMOD1.nREG = 0xFFFFFFFF;
    /* set wake-up polarity : default : active high */
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.nREG = 0x00000000;
    ((PPMU)HwPMU_BASE)->PMU_WKPOL1.nREG = 0x00000000;

    /* Power Key */
#if defined(CONFIG_MACH_TCC8930ST)
    //set wake-up polarity
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.GPIO_D14 = 1; //power key - Active Low
    //set wake-up source
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.GPIO_D14 = 1; //power key
#else
    if(*(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x1000) {
        ((PPMU)HwPMU_BASE)->PMU_WKPOL1.bREG.GPIO_E30 = 1; //power key - Active Low
        ((PPMU)HwPMU_BASE)->PMU_WKUP1.bREG.GPIO_E30 = 1; //power key
    }
    else if(*(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x2000 || *(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x3000){
        ((PPMU)HwPMU_BASE)->PMU_WKPOL1.bREG.GPIO_E24 = 1; //power key - Active Low
        ((PPMU)HwPMU_BASE)->PMU_WKUP1.bREG.GPIO_E24 = 1; //power key
    }
    else if(*(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x5000 || *(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x5001 || *(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x5002
         || *(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x5003){
        ((PPMU)HwPMU_BASE)->PMU_WKPOL1.bREG.GPIO_E27 = 1; //power key - Active Low
        ((PPMU)HwPMU_BASE)->PMU_WKUP1.bREG.GPIO_E27 = 1; //power key
    }
#endif

    /* RTC Alarm Wake Up */
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.RTC_WAKEUP = 0; //RTC_PMWKUP - Active High
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.RTC_WAKEUP = 1; //RTC_PMWKUP - PMU WakeUp Enable

#if defined(CONFIG_INPUT_TCC_REMOTE)
    /* REMOCON Wake Up */
    BITSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw26);
    BITCLR(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw25);
    BITSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw24);
    BITCLR(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw23);
#if 0
    BITSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw22|Hw21);
    BITCLR(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw20);
    /* REMOCON Wake Up */
    BITSET(((PPMU)HwPMU_BASE)->PMU_WKUP1.nREG, Hw31);
#else
    /* REMOCON GPIO Wake Up */
    if (*(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x7231)
    {
        BITSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw21);
        BITCLR(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw22|Hw20);
        ((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.GPIO_D14 = 1;
        ((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.GPIO_D14 = 1;
    }
    else
    {
        BITSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw22|Hw21);
        BITCLR(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw20);
        ((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.GPIO_G17 = 1;
        ((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.GPIO_G17 = 1;
    }
#endif
#endif



// -------------------------------------------------------------------------
// Enter Sleep !!
////////////////////////////////////////////////////////////////////////////
    BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<0);
    nop_delay(10000*3);
////////////////////////////////////////////////////////////////////////////



// -------------------------------------------------------------------------
// set wake-up
    /* disable all for accessing PMU Reg. !!! */
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.nREG = 0x00000000;
    ((PPMU)HwPMU_BASE)->PMU_WKUP1.nREG = 0x00000000;

    /* SSTL & IO Retention Disable */
    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8)))
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode disable=1
    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4)))
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode disable=1

#if defined(CONFIG_MACH_TCC8930ST)
    tcc_stb_resume();
#endif

// Exit SDRAM Self-refresh ------------------------------------------------------------
#if defined(CONFIG_DRAM_DDR3)

    //Bruce_temp.. insert a routine save/restore routine..
    //#define PLL0_VALUE      0x01013806    // PLL0 : 624MHz for CPU
    #define PLL0_VALUE      0x4201A906    // PLL0 : 425MHz for CPU
#if defined(MEMBUS_CLK_AUTO_RESTORE)
    #define PLL1_VALUE      SDRAM_INIT_PARAM(PLL_VALUE)    // PLL1 for MEM
#else
    #define PLL1_VALUE      0x01012C06    // PLL1 : 600MHz for MEM
#endif
    //#define PLL1_VALUE      0x4101F406    // PLL1 : 1GHz for MEM

//--------------------------------------------------------------------------
//Clock setting..

    *(volatile unsigned long *)addr_clk(0x000000) = 0x002ffff4; //cpu bus : XIN
    *(volatile unsigned long *)addr_clk(0x000004) = 0x00200014; //mem bus : XIN/2 
    *(volatile unsigned long *)addr_clk(0x000010) = 0x00200014; //io bus : XIN/2
    *(volatile unsigned long *)addr_clk(0x000020) = 0x00200014; //smu bus : XIN/2
    *(volatile unsigned long *)addr_clk(0x000030) = PLL0_VALUE; //pll0 for cpu
    *(volatile unsigned long *)addr_clk(0x000030) = 0x80000000 | PLL0_VALUE;
    *(volatile unsigned long *)addr_clk(0x000034) = PLL1_VALUE; //pll1 for mem
    *(volatile unsigned long *)addr_clk(0x000034) = 0x80000000 | PLL1_VALUE;
    cnt=3200; while(cnt--);
    //*(volatile unsigned long *)addr_clk(0x000000) = 0x002FFFF0;  // cpu
    *(volatile unsigned long *)addr_clk(0x000000) = 0x002FFFF0;  // cpu
    //*(volatile unsigned long *)addr_clk(0x000004) = 0x00200011;  // mem bus
    *(volatile unsigned long *)addr_clk(0x000004) = 0x00200011;  // mem bus
    *(volatile unsigned long *)addr_clk(0x000010) = 0x00200011;  // io bus
    *(volatile unsigned long *)addr_clk(0x000020) = 0x00200011;  // smu bus
    cnt=3200; while(cnt--);

//--------------------------------------------------------------------------
// Exit self-refresh

    ddr_phy(0x0E) = 0x30c01812; // AC IO Config

    BITCLR(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x0
    BITCLR(denali_ctl(39), 0x3<<24); //ZQ_ON_SREF_EXIT = 0
    BITSET(denali_ctl(0),0x1); //START[0] = 1

    //for(i=0;i<11;i++){
    //    BITSET(denali_ctl(123), 0x1<<16); //CTRLUPD_REQ = 1
    //    while(!(denali_ctl(46)&(0x20000)));
    //    BITSET(denali_ctl(47), 0x20000);
    //}

    BITCSET(denali_ctl(20), 0xFF000000, ((2<<2)|(0<<1)|(1))<<24);
    while(!(denali_ctl(46)&(0x40)));
    BITSET(denali_ctl(47), 0x40);

//--------------------------------------------------------------------------
// MRS Write

    // MRS2
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(28)&0xFFFF0000)>>16);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(31)&0x0000FFFF)<<16);
    denali_ctl(26) = (2<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

    // MRS3
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(29)&0xFFFF0000)>>16);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(32)&0x0000FFFF)<<16);
    denali_ctl(26) = (3<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

    // MRS1
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(28)&0x0000FFFF)>>0);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(30)&0xFFFF0000)<<0);
    denali_ctl(26) = (1<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

    // MRS0
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(27)&0x00FFFF00)>>8);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(30)&0x0000FFFF)<<16);
    denali_ctl(26) = (0<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

//--------------------------------------------------------------------------

    BITCLR(denali_ctl(40) ,0x1<<16); //ZQCS_ROTATE=0x0
    BITCSET(denali_ctl(39) ,0x3<<16, 0x2<<16); //ZQ_REQ=2 : 0x1=short calibration, 0x2=long calibration

//--------------------------------------------------------------------------
// release holding to access to dram

    cnt = 10;    while(cnt--) tmp = BITSET(denali_ctl(13), 1<<24); // AREFRESH = 1
    BITCLR(denali_ctl(44),0x1); //inhibit_dram_cmd=0

//--------------------------------------------------------------------------
#elif defined(CONFIG_DRAM_DDR2)
    #error "DDR2 is not implemented"
#endif

// -------------------------------------------------------------------------
// mmu & cache on
    __asm__ volatile (
    "mrc p15, 0, r0, c1, c0, 0 \n"
    "orr r0, r0, #(1<<12) \n" //ICache
    "orr r0, r0, #(1<<2) \n" //DCache
    "orr r0, r0, #(1<<0) \n" //MMU
    "mcr p15, 0, r0, c1, c0, 0 \n"
    "nop \n"
    );

// -------------------------------------------------------------------------
// REMAP 0xF0000000 area to DRAM
    *(volatile unsigned *)0x73810010 &= ~0x80000000;
}

static void gpio_rearrange(void)
{
    // TODO:
}

static void sleep_mode(void)
{
    unsigned stack;
    FuncPtr  pFunc = (FuncPtr )SLEEP_FUNC_ADDR;
#if defined(MEMBUS_CLK_AUTO_RESTORE)
    unsigned int pll1,p,m,s, tck_value;
#endif

    memcpy((void*)SLEEP_FUNC_ADDR,       (void*)tcc_pm_sleep,  SLEEP_FUNC_SIZE);
#if defined(MEMBUS_CLK_AUTO_RESTORE)
    pll1 = (*(volatile unsigned long *)tcc_p2v(addr_clk(0x000034))) & 0x7FFFFFFF;
    SDRAM_INIT_PARAM(PLL_VALUE) = pll1;
    p = pll1&0x3F;
    m = (pll1>>8)&0x3FF;
    s = (pll1>>24)&0x7;
    tck_value = 1000000/((((XIN_CLK_RATE/10000)*m)/p)>>s);
    SDRAM_INIT_PARAM(DDR_TCK) = tck_value;
#endif

    /*--------------------------------------------------------------
     CKC & gpio
    --------------------------------------------------------------*/
    memcpy(&RegRepo.ckc, (PCKC)tcc_p2v(HwCKC_BASE), sizeof(CKC));

    memcpy(&RegRepo.gpio, (PCKC)tcc_p2v(HwGPIO_BASE), sizeof(GPIO));
    gpio_rearrange();
    
    /*--------------------------------------------------------------
     flush tlb & cache
    --------------------------------------------------------------*/
//    local_flush_tlb_all();
//    flush_cache_all();

    stack = IO_ARM_ChangeStackSRAM();

    /////////////////////////////////////////////////////////////////
    pFunc();
    /////////////////////////////////////////////////////////////////
    IO_ARM_RestoreStackSRAM(stack);

    /*--------------------------------------------------------------
     CKC & GPIO
    --------------------------------------------------------------*/

    memcpy((PCKC)tcc_p2v(HwGPIO_BASE), &RegRepo.gpio, sizeof(GPIO));

    //memcpy((PCKC)tcc_p2v(HwCKC_BASE), &RegRepo.ckc, sizeof(CKC));
    {
        //PLL
        //((PCKC)tcc_p2v(HwCKC_BASE))->PLL0CFG.nREG = RegRepo.ckc.PLL0CFG.nREG;
        //((PCKC)tcc_p2v(HwCKC_BASE))->PLL1CFG.nREG = RegRepo.ckc.PLL1CFG.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL2CFG.nREG = RegRepo.ckc.PLL2CFG.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL3CFG.nREG = RegRepo.ckc.PLL3CFG.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL4CFG.nREG = RegRepo.ckc.PLL4CFG.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL5CFG.nREG = RegRepo.ckc.PLL5CFG.nREG;
        nop_delay(1000);

        //Divider
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKDIVC0.nREG = RegRepo.ckc.CLKDIVC0.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKDIVC1.nREG = RegRepo.ckc.CLKDIVC1.nREG;
        nop_delay(100);

        //((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL0.nREG = RegRepo.ckc.CLKCTRL0.nREG; //CPU Clock can't be adjusted freely.
        //((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL1.nREG = RegRepo.ckc.CLKCTRL1.nREG; //Memory Clock can't be adjusted freely.
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL2.nREG = RegRepo.ckc.CLKCTRL2.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL3.nREG = RegRepo.ckc.CLKCTRL3.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL4.nREG = RegRepo.ckc.CLKCTRL4.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL5.nREG = RegRepo.ckc.CLKCTRL5.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL6.nREG = RegRepo.ckc.CLKCTRL6.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL7.nREG = RegRepo.ckc.CLKCTRL7.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL8.nREG = RegRepo.ckc.CLKCTRL8.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL9.nREG = RegRepo.ckc.CLKCTRL9.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL10.nREG = RegRepo.ckc.CLKCTRL10.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->SWRESET.nREG = ~(RegRepo.ckc.SWRESET.nREG);

        //Peripheral clock
        memcpy((void*)&(((PCKC)tcc_p2v(HwCKC_BASE))->PCLKCTRL00.nREG), (void*)&(RegRepo.ckc.PCLKCTRL00.nREG), 0x17c-0x80);
    }
    nop_delay(100);
}

int tcc_pm_enter(void)
{
    unsigned long flags, tmp;

    if (check_fwdn_mode())
        return 0;

    /* initial register back-up variables. */
    memset(&RegRepo, 0x0, sizeof(TCC_REG));

    //local_irq_save(flags);
    //local_irq_disable();
    /* disable interrupt */
    __asm__ __volatile__( \
    "mrs    %0, cpsr\n" \
    "cpsid    i\n" \
    "mrs    %1, cpsr\n" \
    "orr    %1, %1, #128\n" \
    "msr    cpsr_c, %1" \
    : "=r" (flags), "=r" (tmp) \
    : \
    : "memory", "cc");

    /* set board information */
    *(volatile unsigned long *)BORAD_PRAMETER_ADDR = HW_REV;

    /* enter sleep mode */
    sleep_mode();

    //local_irq_restore(flags);
    /* enable interrupt */
    __asm__ __volatile__( \
    "msr    cpsr_c, %0\n " \
    "cpsid    i" \
    : \
    : "r" (flags) \
    : "memory", "cc");

    return 0;
}

#ifdef CONFIG_ARM_TRUSTZONE
static void shutdown(void)
{
// -------------------------------------------------------------------------
// REMAP 0xF0000000 area to SRAM
    *(volatile unsigned *)0x73810010 |= 0x80000000;

// -------------------------------------------------------------------------
// mmu & cache off
    __asm__ volatile (
    "mrc p15, 0, r0, c1, c0, 0 \n"
    "bic r0, r0, #(1<<12) \n" //ICache
    "bic r0, r0, #(1<<2) \n" //DCache
    "bic r0, r0, #(1<<0) \n" //MMU
    "mcr p15, 0, r0, c1, c0, 0 \n"
    "nop \n"
    );

// -------------------------------------------------------------------------
// SDRAM Self-refresh
#if defined(CONFIG_DRAM_DDR3)
//--------------------------------------------------------------------------
// holding to access to dram
    denali_ctl(47) = 0xFFFFFFFF;
    BITSET(denali_ctl(44),0x1); //inhibit_dram_cmd=1

//--------------------------------------------------------------------------
//enter self-refresh
    BITSET(denali_phy(13), 0x1<<10); //lp_ext_req=1
    while(!(denali_phy(13)&(0x1<<26))); //until lp_ext_ack==1
    BITCSET(denali_phy(13), 0x000000FF, (2<<2)|(1<<1)|(0));
    BITSET(denali_phy(13), 0x1<<8); //lp_ext_cmd_strb=1
    while((denali_phy(13)&(0x003F0000)) != (0x25<<16)); //until lp_ext_state==0x25 : check self-refresh state
    BITCLR(denali_phy(13), 0x1<<8); //lp_ext_cmd_strb=0
    BITCLR(denali_phy(13), 0x1<<10); //lp_ext_req=0
    while(denali_phy(13)&(0x1<<26)); //until lp_ext_ack==0
    BITSET(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x3
    BITCLR(denali_ctl(0),0x1); //START[0] = 0
    ddr_phy(0x0E) = 0xffffffff; // AC IO Config
#elif defined(CONFIG_DRAM_DDR2)
    #error "DDR2 is not implemented"
#endif
    BITSET(ddr_phy(0x0F), Hw1|Hw3|Hw4); //DXIOM=1(LVCMOS mode), DXPDD=1(output driver powered down), DXPDR=1(input buffer powered down)
    nop_delay(1000);

// -------------------------------------------------------------------------
// SRAM Retention
    BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<16); //PD_RETN : SRAM retention mode=0

// -------------------------------------------------------------------------
// Remap
    BITCSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, 0x30000000, 0x10000000); //remap : 0x00000000 <- sram(0x10000000)
    BITSET(((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSCFG.nREG, 1<<31); //RMIR : remap for top area : 0xF0000000 <- sram(0x10000000)

// -------------------------------------------------------------------------
// SSTL & IO Retention
    while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8))
        BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode =0
    while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4))
        BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode =0

// -------------------------------------------------------------------------
// Reset !!
   ((PPMU)HwPMU_BASE)->PMU_WDTCTRL.nREG = (Hw31 + 0x1);
    while (1);
}


static void wakeup(void)
{
// -------------------------------------------------------------------------
// SSTL & IO Retention Disable
    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8)))
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode disable=1
    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4)))
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode disable=1
}

extern volatile void InitRoutine_Start(void);
extern volatile void SRAM_Boot(void);
extern volatile void shutdown_mode(unsigned arg0, unsigned arg1);
int early_shutdown_enter(void)
{
    unsigned long flags;

    if (check_fwdn_mode() || (*(volatile unsigned *)CPU_DATA_REPOSITORY_ADDR == 0x18C818C8)) {
        *(volatile unsigned *)CPU_DATA_REPOSITORY_ADDR = 0x0;
        return 0;
    }

// -------------------------------------------------------------------------
// disable interrupt
    __asm__ __volatile__( \
    "cpsid    i\n" \
    "mrs    %0, cpsr\n" \
    "orr    %0, %0, #128\n" \
    "msr    cpsr_c, %0" \
    : "=r" (flags) \
    : \
    : "memory", "cc");

    /*--------------------------------------------------------------
     replace pm functions
    --------------------------------------------------------------*/
    memcpy((void*)SRAM_BOOT_ADDR,       (void*)SRAM_Boot,  SRAM_BOOT_SIZE);
    memcpy((void*)SLEEP_FUNC_ADDR,      (void*)shutdown,   SLEEP_FUNC_SIZE);
    memcpy((void*)WAKEUP_FUNC_ADDR,     (void*)wakeup,     WAKEUP_FUNC_SIZE);
    memcpy((void*)SDRAM_INIT_FUNC_ADDR, (void*)InitRoutine_Start, SDRAM_INIT_FUNC_SIZE);

    /* all peri io bus on */
    ((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN0.nREG = 0xFFFFFFFF;
    ((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN1.nREG = 0xFFFFFFFF;

    /////////////////////////////////////////////////////////////////
    shutdown_mode(SLEEP_FUNC_ADDR, CPU_DATA_REPOSITORY_ADDR);
    /////////////////////////////////////////////////////////////////

    __asm__ __volatile__ ("nop\n");

    return 0;
}
#endif
