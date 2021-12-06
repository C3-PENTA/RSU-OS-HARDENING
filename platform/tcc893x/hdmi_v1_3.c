
#include <stdlib.h>
#include <string.h>

#include <i2c.h>
#include <lcd.h>
#include <reg.h>

#if defined(DEFAULT_DISPLAY_HDMI) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)
#include <platform/reg_physical.h>
#include <regs-hdmi-v1-3.h>
#if defined(TCC_HDMI_DRIVER_V1_3)
#include <hdmi_v1_3_hdmi.h>
#include <hdmi_v1_3_audio.h>
#endif

#define HDMI_DEBUG	1

#if HDMI_DEBUG
#define DPRINTF	printf
#else
#define DPRINTF
#endif

/*
static struct spdif_struct spdif_struct;
*/

//==============================================================================
//                 Section x - HDMI PHY Setting
//==============================================================================

#define PHY_I2C_ADDRESS             0x70
#define PHY_REG_MODE_SET_DONE       0x28
#define PHY_CONFIG_START_OFFSET     0x01

/**
 * PHY register setting values for each Pixel clock and Bit depth (8, 10, 12 bit).\n
 * @see  Setting values are came from L6LP_HDMI_v1p3_TX_PHY_RegisterMap_REV0.90.doc document.
 */

static const unsigned char phy_config[][3][31] = {// TCC8930 HDMI PHY Setting
        //25.200
    {
		{0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x61, 0x10, 0x02, 0x51, 0x5F, 0xF1, 0x51, 0x7F, 0x84, 0x00, 0x10, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xF3, 0x26, 0x00, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x61, 0x10, 0x02, 0x51, 0x9F, 0xF6, 0x51, 0x9E, 0x84, 0x00, 0x32, 0x38, 0x00, 0xB8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xC2, 0x26, 0x00, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x61, 0x10, 0x02, 0x51, 0xFF, 0xF3, 0x51, 0xBD, 0x84, 0x00, 0x30, 0x38, 0x00, 0xA4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xA2, 0x26, 0x00, 0x00, 0x00, 0x80},

    },
        //25.175
    {
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x1E, 0x20, 0x61, 0x50, 0x10, 0x51, 0xFF, 0xF1, 0x51, 0xBD, 0x84, 0x00, 0x10, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xF3, 0x26, 0x00, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x2B, 0x40, 0x61, 0x50, 0x10, 0x51, 0x7F, 0xF2, 0x51, 0xEC, 0x84, 0x00, 0x10, 0x38, 0x00, 0xB8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xC2, 0x26, 0x00, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x1E, 0x20, 0x61, 0x10, 0x02, 0x51, 0xFF, 0xF1, 0x51, 0xBD, 0x84, 0x00, 0x10, 0x38, 0x00, 0xA4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xA2, 0x26, 0x00, 0x00, 0x00, 0x80},

    },
        //27.000
    {
		{0x05, 0x00, 0x10, 0x10, 0x9C, 0x01, 0xDB, 0x61, 0x10, 0x02, 0x51, 0xDF, 0xF2, 0x51, 0x87, 0x84, 0x00, 0x30, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xE3, 0x26, 0x00, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9E, 0x2D, 0xDB, 0x61, 0x10, 0x02, 0x51, 0xEF, 0xF1, 0x51, 0xA9, 0x84, 0x00, 0x10, 0x38, 0x00, 0xB8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xB5, 0x26, 0x00, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9E, 0xD2, 0x5B, 0x61, 0x10, 0x02, 0x51, 0x2F, 0xF2, 0x51, 0xCB, 0x84, 0x00, 0x10, 0x38, 0x00, 0xA4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x97, 0x26, 0x00, 0x00, 0x00, 0x80},
    },
        //27.027
    {
		{0x05, 0x00, 0xD4, 0x10, 0x9C, 0x09, 0x64, 0x61, 0x10, 0x02, 0x51, 0xDF, 0xF2, 0x51, 0x87, 0x84, 0x00, 0x30, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xE2, 0x26, 0x00, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD4, 0x10, 0x9C, 0x31, 0x50, 0x61, 0x10, 0x02, 0x51, 0x8F, 0xF3, 0x51, 0xA9, 0x84, 0x00, 0x30, 0x38, 0x00, 0xB8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xB5, 0x26, 0x00, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0x10, 0x10, 0x9C, 0x1B, 0x64, 0x61, 0x10, 0x02, 0x51, 0x7F, 0xF8, 0x51, 0xCB, 0x84, 0x00, 0x32, 0x38, 0x00, 0xA4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x97, 0x26, 0x00, 0x00, 0x00, 0x80},
    },
        //54.000
    {
		{0x05, 0x00, 0x10, 0x10, 0x9C, 0x01, 0xDB, 0x61, 0x10, 0x01, 0x51, 0xDF, 0xF2, 0x51, 0x87, 0x84, 0x00, 0x30, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xE3, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x2D, 0xDB, 0x61, 0x10, 0x01, 0x51, 0xCF, 0xF1, 0x51, 0xA9, 0x84, 0x00, 0x10, 0x38, 0x00, 0xF8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xB5, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xD2, 0x5B, 0x61, 0x10, 0x01, 0x51, 0x2F, 0xF2, 0x51, 0xCB, 0x84, 0x00, 0x10, 0x38, 0x00, 0xE4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x97, 0x26, 0x01, 0x00, 0x00, 0x80},
    },
        //54.054
    {
		{0x05, 0x00, 0xD4, 0x10, 0x9C, 0x09, 0x64, 0x61, 0x10, 0x01, 0x51, 0xDF, 0xF2, 0x51, 0x87, 0x84, 0x00, 0x30, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xE2, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD4, 0x10, 0x9C, 0x31, 0x50, 0x61, 0x10, 0x01, 0x51, 0x8F, 0xF3, 0x51, 0xA9, 0x84, 0x00, 0x30, 0x38, 0x00, 0xF8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xB5, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0x10, 0x10, 0x9C, 0x1B, 0x64, 0x61, 0x10, 0x01, 0x51, 0x7F, 0xF8, 0x51, 0xCB, 0x84, 0x00, 0x32, 0x38, 0x00, 0xE4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x97, 0x26, 0x01, 0x00, 0x00, 0x80},
    },
        //74.250
    {
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xE9, 0xDC, 0x61, 0x10, 0x01, 0x51, 0xFF, 0xF1, 0x51, 0xBA, 0x84, 0x00, 0x10, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xA4, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x37, 0xD0, 0x61, 0x10, 0x01, 0x51, 0xDF, 0xF4, 0x51, 0xE8, 0x84, 0x00, 0x30, 0x38, 0x00, 0xF8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x83, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xE2, 0xD0, 0x61, 0x10, 0x01, 0x51, 0xDF, 0xF5, 0x51, 0x16, 0x85, 0x00, 0x30, 0x38, 0x00, 0xE4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xDC, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //74.176
    {
		{0x05, 0x00, 0xD4, 0x10, 0x9C, 0xBC, 0xDB, 0x61, 0x10, 0x01, 0x51, 0xEF, 0xF3, 0x51, 0xB9, 0x84, 0x00, 0x30, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xA5, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD4, 0x10, 0x9C, 0xAB, 0xDB, 0x61, 0x10, 0x01, 0x51, 0xBF, 0xF9, 0x51, 0xE8, 0x84, 0x00, 0x32, 0x38, 0x00, 0xF8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x84, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xCD, 0xDB, 0x61, 0x10, 0x01, 0x51, 0xDF, 0xF5, 0x51, 0x16, 0x84, 0x00, 0x30, 0x38, 0x00, 0xE4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xDC, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //148.500
    {
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xE9, 0xDC, 0x61, 0x18, 0x00, 0x51, 0xFF, 0xF1, 0x51, 0xBA, 0x84, 0x00, 0x10, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xA4, 0x26, 0x02, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x37, 0xD0, 0x61, 0x18, 0x00, 0x51, 0xDF, 0xF4, 0x51, 0xE8, 0x84, 0x00, 0x30, 0x38, 0x00, 0xF8, 0x10, 0x60, /*0x22, 0x40,*/ 0x08,0x42, 0x83, 0x26, 0x02, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xE2, 0xD0, 0x61, 0x18, 0x00, 0x51, 0xDF, 0xF5, 0x51, 0x16, 0x85, 0x00, 0x30, 0x38, 0x00, 0xE4, 0x10, 0x60, /*0x22, 0x40,*/ 0x08,0x42, 0x6D, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //148.352
    {
		{0x05, 0x00, 0xD4, 0x10, 0x9C, 0xBC, 0xDB, 0x61, 0x18, 0x00, 0x51, 0xEF, 0xF3, 0x51, 0xB9, 0x84, 0x00, 0x30, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xA5, 0x26, 0x02, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD4, 0x10, 0x9C, 0xAB, 0xDB, 0x61, 0x18, 0x00, 0x51, 0xBF, 0xF9, 0x51, 0xE8, 0x84, 0x00, 0x32, 0x38, 0x00, 0xF8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x08,0x42, 0x84, 0x26, 0x02, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xCD, 0xDB, 0x61, 0x18, 0x00, 0x51, 0xDF, 0xF5, 0x51, 0x16, 0x85, 0x00, 0x30, 0x38, 0x00, 0xE4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x08,0x42, 0x6D, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //108.108
    {
		{0x05, 0x00, 0xD4, 0x10, 0x9C, 0x09, 0x64, 0x61, 0x18, 0x00, 0x51, 0xDF, 0xF2, 0x51, 0x87, 0x84, 0x00, 0x30, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xE2, 0x26, 0x02, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD4, 0x10, 0x9C, 0x31, 0x50, 0x61, 0x18, 0x00, 0x51, 0x8F, 0xF3, 0x51, 0xA9, 0x84, 0x00, 0x30, 0x38, 0x00, 0xF8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xB5, 0x26, 0x02, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0x10, 0x10, 0x9C, 0x1B, 0x64, 0x61, 0x18, 0x00, 0x51, 0x7F, 0xF8, 0x51, 0xCB, 0x84, 0x00, 0x32, 0x38, 0x00, 0xE4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x97, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //72.000
    {
		{0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x61, 0x10, 0x01, 0x51, 0xEF, 0xF1, 0x51, 0xB4, 0x84, 0x00, 0x10, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xAA, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x61, 0x10, 0x01, 0x51, 0xBF, 0xF4, 0x51, 0xE1, 0x84, 0x00, 0x30, 0x38, 0x00, 0xF8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x88, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x61, 0x18, 0x00, 0x51, 0xDF, 0xF2, 0x51, 0x87, 0x84, 0x00, 0x30, 0x38, 0x00, 0xE4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xE3, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //25.000
    {
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x20, 0x40, 0x61, 0x50, 0x10, 0x51, 0xFF, 0xF1, 0x51, 0xBC, 0x84, 0x00, 0x10, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xF5, 0x26, 0x00, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x08, 0x40, 0x61, 0x50, 0x10, 0x51, 0x7F, 0xF2, 0x51, 0xEA, 0x84, 0x00, 0x10, 0x38, 0x00, 0xB8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xC4, 0x26, 0x00, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x20, 0x40, 0x61, 0x10, 0x02, 0x51, 0xFF, 0xF1, 0x51, 0xBC, 0x84, 0x00, 0x10, 0x38, 0x00, 0xA4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xA3, 0x26, 0x00, 0x00, 0x00, 0x80},
    },
        //65.000
    {
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x02, 0x0C, 0x61, 0x10, 0x01, 0x51, 0xBF, 0xF1, 0x51, 0xA3, 0x84, 0x00, 0x10, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xBC, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xF2, 0x30, 0x61, 0x10, 0x01, 0x51, 0x1F, 0xF2, 0x51, 0xCB, 0x84, 0x00, 0x10, 0x38, 0x00, 0xF8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x96, 0x26, 0x01, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xD0, 0x40, 0x61, 0x10, 0x01, 0x51, 0x9F, 0xF2, 0x51, 0xF4, 0x84, 0x00, 0x10, 0x38, 0x00, 0xE4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x7D, 0x26, 0x01, 0x00, 0x00, 0x80},
    },
        //108.000
    {
		{0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x61, 0x18, 0x00, 0x51, 0xDF, 0xF2, 0x51, 0x87, 0x84, 0x00, 0x30, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xE3, 0x26, 0x02, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x02, 0x08, 0x61, 0x18, 0x00, 0x51, 0xCF, 0xF1, 0x51, 0xA9, 0x84, 0x00, 0x10, 0x38, 0x00, 0xF8, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0xB5, 0x26, 0x02, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xFC, 0x08, 0x61, 0x18, 0x00, 0x51, 0x2F, 0xF2, 0x51, 0xCB, 0x84, 0x00, 0x10, 0x38, 0x00, 0xE4, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x97, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //162.000
    {
		{0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x61, 0x18, 0x00, 0x51, 0x7F, 0xF8, 0x51, 0xCB, 0x84, 0x00, 0x32, 0x38, 0x00, 0x08, 0x10, 0xE0, /*0x22, 0x40,*/ 0x00,0x40, 0x97, 0x26, 0x02, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0x18, 0x40, 0x61, 0x18, 0x00, 0x51, 0xAF, 0xF2, 0x51, 0xFD, 0x84, 0x00, 0x10, 0x38, 0x00, 0xF8, 0x10, 0x60, /*0x22, 0x40,*/ 0x08,0x42, 0x78, 0x26, 0x02, 0x00, 0x00, 0x80},
		{0x05, 0x00, 0xD8, 0x10, 0x9C, 0xD0, 0x40, 0x61, 0x18, 0x00, 0x51, 0x3F, 0xF3, 0x51, 0x30, 0x85, 0x00, 0x10, 0x38, 0x00, 0xE4, 0x10, 0x60, /*0x22, 0x40,*/ 0x08,0x42, 0x64, 0x26, 0x02, 0x00, 0x00, 0x80},
    }
};

/*
void sTorm_delay_us(unsigned int us)
{
	volatile unsigned int i;
	while(us--)
	{
	//	for(i=0 ; i<20 ; i++) __asm__ __volatile__ ("nop");
		for(i=0 ; i<40 ; i++) __asm__ __volatile__ ("nop");		// <<!!
	}
}

void sTorm_delay_ms(unsigned int ms)
{
	sTorm_delay_us(1000*ms);
}
*/

/**
 * Write data though I2C. For more information of I2C, refer I2C Spec.
 * @param   devAddr    [in]    Device address
 * @param   offset  [in]    Byte offset
 * @param   size    [in]    Size of data
 * @param   buffer  [out]   Pointer to buffer
 * @return  If succeed in writing, return 1;Otherwise, return 0
 */
int PHYWrite(unsigned char devAddr, unsigned char offset, unsigned char size, unsigned char* buffer)
{
	unsigned char temp_mem[64];
	unsigned char* temp;
	int i;
	//int bytes;
	int retval = 0;

	//TODO: debugging only


	// allocate temporary buffer
	//temp = (unsigned char*) malloc((size+1)*sizeof(unsigned char));
	temp = &temp_mem[0];
	if (!temp)    {
		printf("not enough resources at %s \n", __func__);
		return retval;
	}

	temp[0] = offset;

#if 0
	memcpy(&temp[1], &buffer[0], size);
#else
	for (i=0; i<size; i++)
		temp[i+1] = buffer[i];
#endif

	// write temp buffer
	i2c_xfer(devAddr, size+1, temp, 0, 0, I2C_CH_SMU);

	retval = 1;

//exit:
    // free temp buffer
    //if (temp) free(temp);

    return retval;
}



/**
 * This function configures PHY registers via I2C.
 *
 * @param    clk    [in]    Pixel clock value.\n
 *                ex : PIXEL_FREQ_25_200, PIXEL_FREQ_74_250...
 * @param    depth    [in]    Color bit depth.\n
 *                 One of (HDMI_CD_24, HDMI_CD_30, HDMI_CD_36).
 * @return    1=Success, 0=Fail
 */
int PHYConfig(const enum PixelFreq clk, const enum ColorDepth depth)
{
    int index, freq;
    int size;
    unsigned char *buffer;
    unsigned char reg;

    // get depth index
    switch (depth)
    {
        case HDMI_CD_24:
            index = 0;
            break;
        case HDMI_CD_30:
            index = 1;
            break;
        case HDMI_CD_36:
            index = 2;
            break;
        default:
            DPRINTF("not available depth arg = %d\n", (int)depth);
            return 0;
    }

    // get clk freq index
    switch (clk)
    {
        case PIXEL_FREQ_25_200 :
            freq = 0;
            break;
        case PIXEL_FREQ_25_175 :
            freq = 1;
            break;
        case PIXEL_FREQ_27 :
            freq = 2;
            break;
        case PIXEL_FREQ_27_027 :
            freq = 3;
            break;
        case PIXEL_FREQ_54 :
            freq = 4;
            break;
        case PIXEL_FREQ_54_054 :
            freq = 5;
            break;
        case PIXEL_FREQ_74_250 :
            freq = 6;
            break;
        case PIXEL_FREQ_74_176 :
            freq = 7;
            break;
        case PIXEL_FREQ_148_500 :
            freq = 8;
            break;
        case PIXEL_FREQ_148_352 :
            freq = 9;
            break;
        case PIXEL_FREQ_108_108 :
            freq = 10;
            break;
        case PIXEL_FREQ_72 :
            freq = 11;
            break;
        case PIXEL_FREQ_25 :
            freq = 12;
            break;
        case PIXEL_FREQ_65 :
            freq = 13;
            break;
        case PIXEL_FREQ_108 :
            freq = 14;
            break;
        case PIXEL_FREQ_162 :
            freq = 15;
            break;
        default:
            DPRINTF("not availlable clk arg = %d\n",(int)clk);
            return 0;
    }

    // start to reconfig after that phy_ready goes down
    reg = 0x00;
    if (!PHYWrite(PHY_I2C_ADDRESS,PHY_REG_MODE_SET_DONE, 1, &reg))
    {
        DPRINTF("fail to write reconfig 0x02%x\n",PHY_REG_MODE_SET_DONE);
        return 0;
    }

	{volatile int ttt;for(ttt=0;ttt<500;ttt++);}

    size = sizeof(phy_config[freq][index]) / sizeof(phy_config[freq][index][0]);
    buffer = (unsigned char *) phy_config[freq][index];

    if (!PHYWrite(PHY_I2C_ADDRESS, PHY_CONFIG_START_OFFSET, size, buffer))
    {
        return 0;
    }

    //for ( idx = 0 ; idx < 1000 ; idx ++ );            // delay

    return 1;
}


//==============================================================================
//                 Section x - HDMI Link / LCDC Timming
//==============================================================================

//! Structure for video timing parameters
static const struct hdmi_video_params
{
    /** [H Blank] */
    unsigned int    HBlank;

    /** [V Blank] */
    unsigned int    VBlank;

    /**
     * [H Total : V Total] @n
     * For more information, refer HDMI register map.
     */
    unsigned int    HVLine;

    /**
     * [H Sync polarity : H Sync end point : H Sync start point]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    HSYNCGEN;

    /**
     * [V Sync start line num + V Sync end line num] @n
     * For more information, refer HDMI register map.
     */
    unsigned int    VSYNCGEN;

    /** CEA VIC */
    unsigned char   AVI_VIC;
    /** CEA VIC for 16:9 pixel ratio */
    unsigned char   AVI_VIC_16_9;

    /** 0 - progresive, 1 - interlaced */
    unsigned char   interlaced;

    /** Pixel repetition if double, set 1 */
    unsigned char   repetition;

    /** V Sync polarity */
    unsigned char   polarity;

    /**
     * In case of interlaced mode, @n
     * [end point of bottom field's active region : start point of that]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    VBLANK_F;

    /**
     * In case of interlaced mode, @n
     * [start line of bottom field's V Sync : end line of that]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    VSYNCGEN2;

    /**
     * In case of interlaced mode, @n
     * [start transition point of bottom field's V Sync : end transition of that]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    VSYNCGEN3;

    /** Pixel frequency */
    enum PixelFreq PixelClock; // pixel clock
} HDMIVideoParams[] =
{

  //{ 0x140, 0x13326, 0x540326, 0x1070A0, 0x2008 , 1 , 1 , 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_65, },  // v1024x768p_60Hz
  //{ 0x140, 0x13326, 0x540326, 0x1701C , 0x3009 , 1 , 1 , 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_65, },  // v1024x768p_60Hz
    { 0xA0 , 0x16A0D, 0x32020D, 0x11B80E, 0xA00C , 1 , 1 , 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_25_200,  },  // v640x480p_60Hz
    { 0x8A , 0x16A0D, 0x35A20D, 0x11300E, 0x900F , 2 , 3 , 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_27_027,  },  // v720x480p_60Hz
    { 0x172, 0xF2EE , 0x6722EE, 0x2506C , 0x500A , 4 , 4 , 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1280x720p_60Hz
    { 0x118, 0xB232 , 0x898465, 0x20856 , 0x2007 , 5 , 5 , 1, 0, 0, 0x232A49, 0x234239, 0x4A44A4, PIXEL_FREQ_74_250,  },  // v1920x1080i_60Hz
    { 0x114, 0xB106 , 0x6B420D, 0x128024, 0x4007 , 6 , 7 , 1, 1, 1, 0x10691D, 0x10A10D, 0x380380, PIXEL_FREQ_27_027,  },  // v720x480i_60Hz
    { 0x114, 0xB106 , 0x6B4106, 0x128024, 0x4007 , 8 , 9 , 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_27_027,  },  // v720x240p_60Hz
    { 0x228, 0xB106 , 0xD6820D, 0x15084A, 0x4007 , 10, 11, 1, 1, 1, 0x10691D, 0x10A10D, 0x700700, PIXEL_FREQ_54_054,  },  // v2880x480i_60Hz
    { 0x228, 0xB106 , 0x6B4106, 0x15084A, 0x4007 , 12, 13, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54_054,  },  // v2880x240p_60Hz
    { 0x114, 0x16A0D, 0x6B420D, 0x12681E, 0x900F , 14, 15, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54_054,  },  // v1440x480p_60Hz
    { 0x118, 0x16C65, 0x898465, 0x20856 , 0x4009 , 16, 16, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1920x1080p_60Hz
    { 0x90 , 0x18A71, 0x360271, 0x11280A, 0x500A , 17, 18, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_27,      },  // v720x576p_50Hz
    { 0x2BC, 0xF2EE , 0x7BC2EE, 0x779B6 , 0x500A , 19, 19, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1280x720p_50Hz
    { 0x2D0, 0xB232 , 0xA50465, 0x8EA0E , 0x2007 , 20, 20, 1, 0, 0, 0x232A49, 0x234239, 0x738738, PIXEL_FREQ_74_250,  },  // v1920x1080i_50Hz
    { 0x120, 0xC138 , 0x6C0271, 0x125016, 0x2005 , 21, 22, 1, 1, 1, 0x138951, 0x13A13D, 0x378378, PIXEL_FREQ_27,      },  // v720x576i_50Hz
    { 0x120, 0xC138 , 0x6C0138, 0x125016, 0x3006 , 23, 24, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_27,      },  // v720x288p_50Hz
    { 0x240, 0xC138 , 0xD80271, 0x14A82E, 0x2005 , 25, 26, 1, 1, 1, 0x138951, 0x13A13D, 0x6F06F0, PIXEL_FREQ_54,      },  // v2880x576i_50Hz
    { 0x240, 0xC138 , 0xD80138, 0x14A82E, 0x2005 , 27, 28, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54,      },  // v2880x288p_50Hz
    { 0x120, 0x18A71, 0x6C0271, 0x125816, 0x500A , 29, 30, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54,      },  // v1440x576p_50Hz
    { 0x2D0, 0x16C65, 0xA50465, 0x8EA0E , 0x4009 , 31, 31, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1920x1080p_50Hz
    { 0x33E, 0x16C65, 0xABE465, 0xAA27C , 0x4009 , 32, 32, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1920x1080p_24Hz
    { 0x2D0, 0x16C65, 0xA50465, 0x8EA0E , 0x4009 , 33, 33, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1920x1080p_25Hz
    { 0x2D0, 0x16C65, 0xA50465, 0x8EA0E , 0x4009 , 34, 34, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1920x1080p_30Hz
    { 0x228, 0x16A0D, 0xD6820D, 0x14D83E, 0x900F , 35, 36, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_108_108, },  // v2880x480p_60Hz
    { 0x240, 0x18A71, 0xD80271, 0x14B82E, 0x500A , 37, 38, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_108,     },  // v2880x576p_50Hz
    { 0x180, 0x2AA71, 0x9004E2, 0x3181E , 0x1701C, 39, 39, 0, 0, 0, 0x2712C6, 0x28728F, 0x4a44a4, PIXEL_FREQ_72,      },  // v1920x1080i_50Hz(1250)
    { 0x2D0, 0xB232 , 0xA50465, 0x8EA0E , 0x2007 , 40, 40, 1, 0, 0, 0x232A49, 0x234239, 0x738738, PIXEL_FREQ_148_500, },  // v1920x1080i_100Hz
    { 0x2BC, 0xF2EE , 0x7BC2EE, 0x779B6 , 0x500A , 41, 41, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1280x720p_100Hz
    { 0x90 , 0x18A71, 0x360271, 0x11280A, 0x500A , 42, 43, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_54,      },  // v720x576p_100Hz
    { 0x120, 0xC138 , 0x6C0271, 0x125016, 0x2005 , 44, 45, 1, 1, 1, 0x138951, 0x13A13D, 0x378378, PIXEL_FREQ_54,      },  // v720x576i_100Hz
    { 0x118, 0xB232 , 0x898465, 0x20856 , 0x2007 , 46, 46, 1, 0, 0, 0x232A49, 0x234239, 0x4A44A4, PIXEL_FREQ_148_500, },  // v1920x1080i_120Hz
    { 0x172, 0xF2EE , 0x6722EE, 0x2506C , 0x500A , 47, 47, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1280x720p_120Hz
    { 0x8A , 0x16A0D, 0x35A20D, 0x11300E, 0x900F , 48, 49, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_54_054,  },  // v720x480p_120Hz
    { 0x114, 0xB106 , 0x6B420D, 0x128024, 0x4007 , 50, 51, 1, 1, 1, 0x10691D, 0x10A10D, 0x380380, PIXEL_FREQ_54_054,  },  // v720x480i_120Hz
    { 0x90 , 0x18A71, 0x360271, 0x11280A, 0x500A , 52, 53, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_108,     },  // v720x576p_200Hz
    { 0x120, 0xC138 , 0x6C0271, 0x125016, 0x2005 , 54, 55, 1, 1, 1, 0x138951, 0x13A13D, 0x378378, PIXEL_FREQ_108,     },  // v720x576i_200Hz
    { 0x8A , 0x16A0D, 0x35A20D, 0x11300E, 0x900F , 56, 57, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_108_108, },  // v720x480p_240Hz
    { 0x114, 0xB106 , 0x6B420D, 0x128024, 0x4007 , 58, 59, 1, 1, 1, 0x10691D, 0x10A10D, 0x380380, PIXEL_FREQ_108_108, },  // v720x480i_240Hz
};


/**
 * N value of ACR packet.@n
 * 4096  is the N value for 32 KHz sampling frequency @n
 * 6272  is the N value for 44.1 KHz sampling frequency @n
 * 12544 is the N value for 88.2 KHz sampling frequency @n
 * 25088 is the N value for 176.4 KHz sampling frequency @n
 * 6144  is the N value for 48 KHz sampling frequency @n
 * 12288 is the N value for 96 KHz sampling frequency @n
 * 24576 is the N value for 192 KHz sampling frequency @n
 */
static const unsigned int ACR_N_params[] =
{
    4096,
    6272,
    12544,
    25088,
    6144,
    12288,
    24576
};


void hdmi_phy_reset(void)
{
	unsigned int  regl;
	unsigned char phy_status;

	PDDICONFIG		pDDIBUSCFG;
	PCKC				pCKC ;
	
	pDDIBUSCFG  = (DDICONFIG *)HwDDI_CONFIG_BASE;
#if !defined(HDMI_USE_INTERNAL_PLL)
	pCKC = (CKC *)HwCKC_BASE;

	pCKC->PCLKCTRL17.nREG = 0x2D000000;
	pCKC->PCLKCTRL18.nREG = 0x2D000000;
#endif /* HDMI_USE_INTERNAL_PLL */
	
    pDDIBUSCFG->HDMI_CTRL.bREG.PHYRESET = 0;
    //for(i=0;i<4000;i++);
    pDDIBUSCFG->HDMI_CTRL.bREG.SPDIFRESET = 0;

	phy_status = readb(HDMI_PHY_STATUS);
	DPRINTF("%s  phy_status:%d \n", __func__,  phy_status);    

    pDDIBUSCFG->HDMI_CTRL.bREG.TMDSRESET = 0;

    pDDIBUSCFG->SWRESET.bREG.HDMI = 1;
	

	// HDMI PHY Reset
	regl = readl(DDICFG_HDMICTRL);
	writel(regl | HDMICTRL_RESET_HDMI, DDICFG_HDMICTRL);
	{volatile int ttt;for(ttt=0;ttt<0x5000;ttt++);}
	writel(regl & ~HDMICTRL_RESET_HDMI, DDICFG_HDMICTRL);

}


void hdmi_ddi_config_init (void)
{
#define HDMI_CONFIG_DELAY 	0x500
	unsigned char reg;
	unsigned int regl;	
	
	// HDMI PHY Reset
	hdmi_phy_reset();

	// HDMI SPDIF Reset
	regl = readl(DDICFG_HDMICTRL);
	writel(regl | HDMICTRL_RESET_SPDIF, DDICFG_HDMICTRL);	
	{volatile int ttt;for(ttt=0;ttt<HDMI_CONFIG_DELAY;ttt++);}
	writel(regl & ~HDMICTRL_RESET_SPDIF, DDICFG_HDMICTRL);



	// HDMI TMDS Reset
	regl = readl(DDICFG_HDMICTRL);
	writel(regl | HDMICTRL_RESET_TMDS, DDICFG_HDMICTRL);
	{volatile int ttt;for(ttt=0;ttt<HDMI_CONFIG_DELAY;ttt++);}
	writel(regl & ~HDMICTRL_RESET_TMDS, DDICFG_HDMICTRL);


	// enable DDI_BUS HDMI CLK
	regl = readl(DDICFG_HDMICTRL);
	writel(regl | HDMICTRL_HDMI_ENABLE, DDICFG_HDMICTRL);
	{volatile int ttt;for(ttt=0;ttt<HDMI_CONFIG_DELAY;ttt++);}

	// disable HDCP INT
	reg = readb(HDMI_SS_INTC_CON);
	writeb(reg & ~(1<<HDMI_IRQ_HDCP), HDMI_SS_INTC_CON);
	// disable SPDIF INT
	reg = readb(HDMI_SS_INTC_CON);
	writeb(reg & ~(1<<HDMI_IRQ_SPDIF), HDMI_SS_INTC_CON);


}

/**
 * Set checksum in Audio InfoFrame Packet. @n
 * Calculate a checksum and set it in packet.
 */
void hdmi_aui_update_checksum(void)
{
    unsigned char index, checksum;

    checksum = AUI_HEADER;
    for (index = 0; index < AUI_PACKET_BYTE_LENGTH; index++)
    {
#if 1
        // when write this byte(PB5), HW shift 3 bit to right direction.
        // to compensate it, when read it, SW should shift 3 bit to left.
        if (index == 4)
            checksum += (readb(HDMI_AUI_BYTE1 + 4*index)<<3);
        else
            checksum += readb(HDMI_AUI_BYTE1 + 4*index);
#else
        checksum += readb(HDMI_AUI_BYTE1 + 4*index);
#endif
    }
    writeb(~checksum+1,HDMI_AUI_CHECK_SUM);
}

/**
 * Set checksum in AVI InfoFrame Packet. @n
 * Calculate a checksum and set it in packet.
 */
void hdmi_avi_update_checksum(void)
{
    unsigned char index, checksum;

    checksum = AVI_HEADER;
    for (index = 0; index < AVI_PACKET_BYTE_LENGTH; index++)
    {
        checksum += readb(HDMI_AVI_BYTE1 + 4*index);
    }
    writeb(~checksum+1,HDMI_AVI_CHECK_SUM);
}

/**
 * Set HDMI/DVI mode
 * @param   mode   [in] HDMI/DVI mode
 * @return  If argument is invalid, return 0;Otherwise return 1.
 */
int hdmi_set_hdmi_mode(int mode)
{
	int ret = 1;

	switch(mode)
	{
		case HDMI:
	        writeb(HDMI_MODE_SEL_HDMI,HDMI_MODE_SEL);
	        writeb(HDMICON2_HDMI,HDMI_CON_2);
			break;
		case DVI:
	        writeb(HDMI_MODE_SEL_DVI,HDMI_MODE_SEL);
	        writeb(HDMICON2_DVI,HDMI_CON_2);
			break;
		default:
			ret = 0;
			break;
	}

	return ret;
}

/**
 * Set pixel aspect ratio information in AVI InfoFrame
 * @param   ratio   [in] Pixel Aspect Ratio
 * @return  If argument is invalid, return 0;Otherwise return 1.
 */
int hdmi_set_pixel_aspect_ratio(enum PixelAspectRatio ratio)
{
    int ret = 1;
    unsigned char reg = AVI_FORMAT_ASPECT_AS_PICTURE;

    switch (ratio)
    {
        case HDMI_PIXEL_RATIO_16_9:
            reg |= AVI_PICTURE_ASPECT_RATIO_16_9;
            break;
        case HDMI_PIXEL_RATIO_4_3:
            reg |= AVI_PICTURE_ASPECT_RATIO_4_3;
            break;
        default:
            ret = 0;
     }
    writeb(reg,HDMI_AVI_BYTE2);
    return ret;
}


/**
 * Set video timing parameters.@n
 * @param   mode   [in] Video timing parameters
 */
void hdmi_set_video_timing(struct device_video_params mode)
{
    unsigned char reg;
    unsigned int  val;

    // set HBLANK;
    val = mode.HBlank;
    reg = val & 0xff;
    writeb(reg,HDMI_H_BLANK_0);
    reg = (val>>8) & 0xff;
    writeb(reg,HDMI_H_BLANK_1);

    // set VBlank
    val = mode.VBlank;
    reg = val & 0xff;
    writeb(reg, HDMI_V_BLANK_0);
    reg = (val>>8) & 0xff;
    writeb(reg, HDMI_V_BLANK_1);
    reg = (val>>16) & 0xff;
    writeb(reg, HDMI_V_BLANK_2);

    // set HVLine
    val = mode.HVLine;
    reg = val & 0xff;
    writeb(reg, HDMI_H_V_LINE_0);
    reg = (val>>8) & 0xff;
    writeb(reg, HDMI_H_V_LINE_1);
    reg = (val>>16) & 0xff;
    writeb(reg, HDMI_H_V_LINE_2);

    // set VSync Polarity
    writeb(mode.polarity, HDMI_VSYNC_POL);

    // set HSyncGen
    val = mode.HSYNCGEN;
    reg = val & 0xff;
    writeb(reg, HDMI_H_SYNC_GEN_0);
    reg = (val>>8) & 0xff;
    writeb(reg, HDMI_H_SYNC_GEN_1);
    reg = (val>>16) & 0xff;
    writeb(reg, HDMI_H_SYNC_GEN_2);

    // set VSyncGen1
    val = mode.VSYNCGEN;
    reg = val & 0xff;
    writeb(reg, HDMI_V_SYNC_GEN1_0);
    reg = (val>>8) & 0xff;
    writeb(reg, HDMI_V_SYNC_GEN1_1);
    reg = (val>>16) & 0xff;
    writeb(reg, HDMI_V_SYNC_GEN1_2);

    // set interlace or progresive mode
    writeb(mode.interlaced,HDMI_INT_PRO_MODE);

    if ( mode.interlaced ) // interlaced mode
    {
        // set VBlank_F
        val = mode.VBLANK_F;
        reg = val & 0xff;
        writeb(reg, HDMI_V_BLANK_F_0);
        reg = (val>>8) & 0xff;
        writeb(reg, HDMI_V_BLANK_F_1);
        reg = (val>>16) & 0xff;
        writeb(reg, HDMI_V_BLANK_F_2);

        // set VSyncGen2
        val = mode.VSYNCGEN2;
        reg = val & 0xff;
        writeb(reg, HDMI_V_SYNC_GEN2_0);
        reg = (val>>8) & 0xff;
        writeb(reg, HDMI_V_SYNC_GEN2_1);
        reg = (val>>16) & 0xff;
        writeb(reg, HDMI_V_SYNC_GEN2_2);

        // set VSyncGen3
        val = mode.VSYNCGEN3;
        reg = val & 0xff;
        writeb(reg, HDMI_V_SYNC_GEN3_0);
        reg = (val>>8) & 0xff;
        writeb(reg, HDMI_V_SYNC_GEN3_1);
        reg = (val>>16) & 0xff;
        writeb(reg, HDMI_V_SYNC_GEN3_2);
    }
    else
    {
        // set VBlank_F with default value
        writeb(0x00, HDMI_V_BLANK_F_0);
        writeb(0x00, HDMI_V_BLANK_F_1);
        writeb(0x00, HDMI_V_BLANK_F_2);

        // set VSyncGen2 with default value
        writeb(0x01, HDMI_V_SYNC_GEN2_0);
        writeb(0x10, HDMI_V_SYNC_GEN2_1);
        writeb(0x00, HDMI_V_SYNC_GEN2_2);

        // set VSyncGen3 with default value
        writeb(0x01, HDMI_V_SYNC_GEN3_0);
        writeb(0x10, HDMI_V_SYNC_GEN3_1);
        writeb(0x00, HDMI_V_SYNC_GEN3_2);
    }

    // set pixel repetition
    reg = readb(HDMI_CON_1);
    if ( mode.repetition )
    {
        // set pixel repetition
        writeb(reg|HDMICON1_DOUBLE_PIXEL_REPETITION,HDMI_CON_1);
        // set avi packet
        writeb(AVI_PIXEL_REPETITION_DOUBLE,HDMI_AVI_BYTE5);
    }
    else
    {
        // clear pixel repetition
        writeb(reg & ~(1<<1|1<<0),HDMI_CON_1);
        // set avi packet
        writeb(0x00,HDMI_AVI_BYTE5);
    }

    // set AVI packet with VIC
	reg = readb(HDMI_AVI_BYTE2);

	if (reg & (unsigned char)AVI_PICTURE_ASPECT_RATIO_4_3)
		writeb(mode.AVI_VIC,HDMI_AVI_BYTE4);
	else
		writeb(mode.AVI_VIC_16_9,HDMI_AVI_BYTE4);

    return;
}

/**
 * Set color space in HDMI H/W. @n
 * @param   space   [in] Color space
 * @return  If argument is invalid, return 0;Otherwise return 1.
 */
int hdmi_set_color_space(enum ColorSpace space)
{
    unsigned char reg,aviYY;
    int ret = 1;

    reg = readb(HDMI_CON_0);
    aviYY = readb(HDMI_AVI_BYTE1);
    // clear fields
    writeb(aviYY & ~(AVI_CS_Y422|AVI_CS_Y444),HDMI_AVI_BYTE1);
	aviYY = readb(HDMI_AVI_BYTE1);

    if (space == HDMI_CS_YCBCR422)
    {
        // set video input interface
        writeb( reg | HDMI_YCBCR422_ENABLE, HDMI_CON_0);
        // set avi
        writeb( aviYY | AVI_CS_Y422, HDMI_AVI_BYTE1);
    }
    else
    {
        // set video input interface
        writeb( reg & ~HDMI_YCBCR422_ENABLE, HDMI_CON_0);
        if (space == HDMI_CS_YCBCR444)
        {
            // set AVI packet
            writeb( aviYY | AVI_CS_Y444, HDMI_AVI_BYTE1);
        }
        // aviYY for RGB = 0, nothing to set
        else if (space != HDMI_CS_RGB)
        {
            ret = 0;
        }
    }

    return ret;
}


/**
 * Set color depth.@n
 * @param   depth   [in] Color depth of input vieo stream
 * @return  If argument is invalid, return 0;Otherwise return 1.
 */
int hdmi_set_color_depth(enum ColorDepth depth)
{
    int ret = 1;
    switch (depth)
    {
        case HDMI_CD_36:
        {
            // set GCP CD
            writeb(GCP_CD_36BPP,HDMI_GCP_BYTE2);
            // set DC_CTRL
            writeb(HDMI_DC_CTL_12,HDMI_DC_CONTROL);
            break;
        }
        case HDMI_CD_30:
        {
            // set GCP CD
            writeb(GCP_CD_30BPP,HDMI_GCP_BYTE2);
            // set DC_CTRL
            writeb(HDMI_DC_CTL_10,HDMI_DC_CONTROL);
            break;
        }
        case HDMI_CD_24:
        {
            // set GCP CD
            writeb(GCP_CD_24BPP,HDMI_GCP_BYTE2);
            // set DC_CTRL
            writeb(HDMI_DC_CTL_8,HDMI_DC_CONTROL);
            break;
        }

        default:
        {
            ret = 0;
        }
    }
    return ret;
}

/**
 * Set pixel limitation.
 * @param   limit   [in] Pixel limitation.
* @return  If argument is invalid, return 0;Otherwise return 1.
 */
int hdmi_set_pixel_limit(enum PixelLimit limit)
{
    int ret = 1;
    unsigned char reg,aviQQ;

    // clear field
    reg = readb(HDMI_CON_1);
    reg &= ~HDMICON1_LIMIT_MASK;

    aviQQ = readb(HDMI_AVI_BYTE3);
    aviQQ &= ~AVI_QUANTIZATION_MASK;

    switch (limit) // full
    {
        case HDMI_FULL_RANGE:
        {
            aviQQ |= AVI_QUANTIZATION_FULL;
            break;
        }
        case HDMI_RGB_LIMIT_RANGE:
        {
            reg |= HDMICON1_RGB_LIMIT;
            aviQQ |= AVI_QUANTIZATION_LIMITED;
            break;
        }
        case HDMI_YCBCR_LIMIT_RANGE:
        {
            reg |= HDMICON1_YCBCR_LIMIT;
            aviQQ |= AVI_QUANTIZATION_LIMITED;
            break;
        }
        default:
        {
            ret = 0;
        }
    }
    // set pixel repetition
    writeb(reg,HDMI_CON_1);
    // set avi packet body
    writeb(aviQQ,HDMI_AVI_BYTE3);

    return ret;
}

/**
 * Configurate PHY.
 * @param   hdmi_video_mode [in]    Video mode to set
 * @return  If success, return 1; Otherwise, return 0.
 */
int hdmi_set_phy_config(const struct HDMIVideoParameter * const hdmi_video_mode)
{
    if (!PHYConfig(HDMIVideoParams[hdmi_video_mode->resolution].PixelClock,
          hdmi_video_mode->colorDepth))
    {
        DPRINTF("phy config failed!\n");
        return 0;
    }

    return 1;
}

/**
 * Set requested video mode.
 * @param   hdmi_video_mode [in]   requested video mode to set
 * @return  If success, return 1; Otherwise, return 0
 */
int hdmi_set_video_mode(const struct HDMIVideoParameter * const hdmi_video_mode)
{
    struct device_video_params device;
	enum PixelLimit pxl_lmt = HDMI_FULL_RANGE;

    if (!hdmi_video_mode)
    {
        DPRINTF("bad args: hdmi_video_mode\n");
        return 0;
    }

    // set pixel aspect ratio
    // !! must be setting before 'HDMI_IOC_SET_VIDEOMODE'
    hdmi_set_pixel_aspect_ratio(hdmi_video_mode->pixelAspectRatio);

    // parsing video parameters
    memcpy((void*)&device,(const void*)&(HDMIVideoParams[hdmi_video_mode->resolution]),sizeof(device));

    // set video parameters
    hdmi_set_video_timing(device);

    // set video format information
	//gHdmiVideoParms.resolution = hdmi_video_mode->resolution;

    // set color space
    if ( !hdmi_set_color_space(hdmi_video_mode->colorSpace) )
    {
        DPRINTF("bad args: hdmi_video_mode->colorSpace : Not Correct Arg = %d\n", hdmi_video_mode->colorSpace);
        //return -EFAULT;
    }
	//gHdmiVideoParms.colorSpace = hdmi_video_mode->colorSpace;

    // set color depth
    if ( !hdmi_set_color_depth(hdmi_video_mode->colorDepth) )
    {
        DPRINTF("bad args: hdmi_video_mode->colorDepth : Not Correct Arg = %d\n", hdmi_video_mode->colorDepth);
        //return -EFAULT;
    }
	//gHdmiVideoParms.colorDepth = hdmi_video_mode->colorDepth;

	// set pixel limitation.
	switch(hdmi_video_mode->colorSpace) 
	{
		case HDMI_CS_RGB:		/** RGB color space */
			if (hdmi_video_mode->resolution == v640x480p_60Hz)
				pxl_lmt = HDMI_FULL_RANGE;
			else
				pxl_lmt = HDMI_RGB_LIMIT_RANGE;
			break;
		case HDMI_CS_YCBCR444:	/** YCbCr 4:4:4 color space */
		case HDMI_CS_YCBCR422:	/** YCbCr 4:2:2 color space */
			pxl_lmt = HDMI_YCBCR_LIMIT_RANGE;
			break;
	}

    if (!hdmi_set_pixel_limit(pxl_lmt))
    {
		DPRINTF("bad args: hdmi_video_mode->colorDepth : Not Correct Arg = %d\n", pxl_lmt);
        //return -EFAULT;
    }
	//gPixelLimit = pxl_lmt;

    // set phy
    if (!hdmi_set_phy_config(hdmi_video_mode))
    {
        DPRINTF("fail to config PHY!\n");
        //return 0;
    }

    return 1;
}


/**
 * Set audio input port.
 *
 * @param   port    [in]    Audio input port.
 * @return  If argument is invalid, return 0;Otherwise, return 1.
 */

//int setAudioInputPort(enum HDMIAudioPort port)
int hdmi_set_audio_inputport(enum HDMIAudioPort port)
{
    switch (port)
    {
        case I2S_PORT:
        {
		    // disable SPDIF INT
			{
			    unsigned char reg;
			    reg = readb(HDMI_SS_INTC_CON);
			    writeb(reg & ~(1<<HDMI_IRQ_SPDIF), HDMI_SS_INTC_CON);
			}
			
            // enable audio
            //writeb(0, HDMI_SS_SPDIF_CLK_CTRL); // enable clock???
            writeb(I2S_CLK_CON_ENABLE,HDMI_SS_I2S_CLK_CON);

            // disable DSD
            writeb(I2S_DSD_CON_DISABLE, HDMI_SS_I2S_DSD_CON);

            // I2S control
            writeb(I2S_CON_SC_POL_FALLING | I2S_CON_CH_POL_LOW, HDMI_SS_I2S_CON_1);

            // I2S MUX Control
            writeb(I2S_IN_MUX_ENABLE | I2S_IN_MUX_CUV_ENABLE | I2S_IN_MUX_SELECT_I2S | I2S_IN_MUX_IN_ENABLE, HDMI_SS_I2S_IN_MUX_CON);

            // enable all channels
            writeb(I2S_MUX_CH_ALL_ENABLE , HDMI_SS_I2S_MUX_CH);

            // enable CUV from right and left channel
            writeb(I2S_MUX_CUV_LEFT_ENABLE| I2S_MUX_CUV_RIGHT_ENABLE , HDMI_SS_I2S_MUX_CUV);

            break;
        }
        case SPDIF_PORT:
        {
			//TODO: implement
            break;
        }
        case DSD_PORT:
        {
			//TODO: implement
            break;
        }
        default:
            return 0;
    }
    return 1;
}

/**
 * Set sampling frequency in I2S receiver.
 *
 * @param   freq    [in]   Sampling frequency.
 * @return  If argument is invalid, return 0;Otherwise, return 1.
 */
//int setCUVSampleFreq(enum SamplingFreq freq)
int hdmi_set_audio_cuv_samplefreq(enum SamplingFreq freq)
{
    int ret = 1;
    unsigned char reg = readb(HDMI_SS_I2S_CH_ST_3) & ~I2S_CH_ST_3_SF_MASK;

    switch (freq)
    {
        case SF_32KHZ:
            reg |= I2S_CH_ST_3_SF_32KHZ;
            break;
        case SF_44KHZ:
            reg |= I2S_CH_ST_3_SF_44KHZ;
            break;
        case SF_88KHZ:
            reg |= I2S_CH_ST_3_SF_88KHZ;
            break;
        case SF_176KHZ:
            reg |= I2S_CH_ST_3_SF_176KHZ;
            break;
        case SF_48KHZ:
            reg |= I2S_CH_ST_3_SF_48KHZ;
            break;
        case SF_96KHZ:
            reg |= I2S_CH_ST_3_SF_96KHZ;
            break;
        case SF_192KHZ:
            reg |= I2S_CH_ST_3_SF_192KHZ;
            break;
        default:
            ret = 0;
    }

    writeb(reg, HDMI_SS_I2S_CH_ST_3);

    return ret;
}

/**
 * Set coding type in I2S receiver.
 *
 * @param   coding    [in]   Coding type.
 * @return  If argument is invalid, return 0;Otherwise, return 1.
 */
//int setCUVCodingType(enum CUVAudioCoding coding)
int hdmi_set_audio_cuv_codingtype(enum CUVAudioCoding coding)
{
    int ret = 1;
    unsigned char reg = readb(HDMI_SS_I2S_CH_ST_0) & ~I2S_CH_ST_0_TYPE_MASK;

    switch (coding)
    {
        case CUV_LPCM:
            reg |= I2S_CH_ST_0_TYPE_LPCM;
            break;

        case CUV_NLPCM:
            reg |= I2S_CH_ST_0_TYPE_NLPCM;
            break;

        default:
            ret = 0;
    };

    writeb(reg, HDMI_SS_I2S_CH_ST_0);

    return ret;
}

/**
 * Set the number of channels in I2S receiver.
 *
 * @param   num     [in]   Number of channels.
 * @return  If argument is invalid, return 0;Otherwise, return 1.
 */
//int setCUVChannelNum(enum CUVChannelNumber num)
int hdmi_set_audio_cuv_channelnum(enum CUVChannelNumber num)
{
    int ret = 1;
    unsigned char reg = readb(HDMI_SS_I2S_CH_ST_2) & ~I2S_CH_ST_2_CHANNEL_MASK;

    switch (num)
    {
        case CUV_CH_UNDEFINED:
            reg |= I2S_CH_ST_2_CH_UNDEFINED;
            break;

        case CUV_CH_01:
            reg |= I2S_CH_ST_2_CH_01;
            break;

        case CUV_CH_02:
            reg |= I2S_CH_ST_2_CH_02;
            break;

        case CUV_CH_03:
            reg |= I2S_CH_ST_2_CH_03;
            break;

        case CUV_CH_04:
            reg |= I2S_CH_ST_2_CH_04;
            break;

        case CUV_CH_05:
            reg |= I2S_CH_ST_2_CH_05;
            break;

        case CUV_CH_06:
            reg |= I2S_CH_ST_2_CH_06;
            break;

        case CUV_CH_07:
            reg |= I2S_CH_ST_2_CH_07;
            break;

        case CUV_CH_08:
            reg |= I2S_CH_ST_2_CH_08;
            break;

        case CUV_CH_09:
            reg |= I2S_CH_ST_2_CH_09;
            break;

        case CUV_CH_10:
            reg |= I2S_CH_ST_2_CH_10;
            break;

        case CUV_CH_11:
            reg |= I2S_CH_ST_2_CH_11;
            break;

        case CUV_CH_12:
            reg |= I2S_CH_ST_2_CH_12;
            break;

        case CUV_CH_13:
            reg |= I2S_CH_ST_2_CH_13;
            break;

        case CUV_CH_14:
            reg |= I2S_CH_ST_2_CH_14;
            break;

        case CUV_CH_15:
            reg |= I2S_CH_ST_2_CH_15;
            break;

        default:
            ret = 0;
    }

    writeb(reg, HDMI_SS_I2S_CH_ST_2);

    return ret;
}

/**
 * Set word length in I2S receiver.
 *
 * @param   length    [in]   Word length.
 * @return  If argument is invalid, return 0;Otherwise, return 1.
 */
//int setCUVWordLength(enum CUVWordLength length)
int hdmi_set_audio_cuv_wordlength(enum CUVWordLength length)
{
    int ret = 1;
    unsigned char reg = readb(HDMI_SS_I2S_CH_ST_4) & ~I2S_CH_ST_4_WL_MASK;

    switch (length)
    {
        case CUV_WL_20_NOT_DEFINED:
            reg |= I2S_CH_ST_4_WL_20_NOT_DEFINED;
            break;

        case CUV_WL_20_16:
            reg |= I2S_CH_ST_4_WL_20_16;
            break;

        case CUV_WL_20_18:
            reg |= I2S_CH_ST_4_WL_20_18;
            break;

        case CUV_WL_20_19:
            reg |= I2S_CH_ST_4_WL_20_19;
            break;

        case CUV_WL_20_20:
            reg |= I2S_CH_ST_4_WL_20_20;
            break;

        case CUV_WL_20_17:
            reg |= I2S_CH_ST_4_WL_20_17;
            break;

        case CUV_WL_24_NOT_DEFINED:
            reg |= I2S_CH_ST_4_WL_24_NOT_DEFINED;
            break;

        case CUV_WL_24_20:
            reg |= I2S_CH_ST_4_WL_24_20;
            break;

        case CUV_WL_24_22:
            reg |= I2S_CH_ST_4_WL_24_22;
            break;

        case CUV_WL_24_23:
            reg |= I2S_CH_ST_4_WL_24_23;
            break;

        case CUV_WL_24_24:
            reg |= I2S_CH_ST_4_WL_24_24;
            break;

        case CUV_WL_24_21:
            reg |= I2S_CH_ST_4_WL_24_21;
            break;

        default:
            ret = 0;
    }
    writeb(reg, HDMI_SS_I2S_CH_ST_4);

    return ret;
}

/**
 * Set I2S audio paramters in I2S receiver.
 *
 * @param   i2s     [in]   I2S audio paramters.
 * @return  If argument is invalid, return 0;Otherwise, return 1.
 */
//int setI2SParameter(struct I2SParameter i2s)
int hdmi_set_audio_i2s_parameter(struct I2SParameter i2s)
{
    unsigned char reg;
    // bit per channel
    switch(i2s.bpc)
    {
        case I2S_BPC_16:
            reg = I2S_CON_DATA_NUM_16;
            break;

        case I2S_BPC_20:
            reg = I2S_CON_DATA_NUM_20;
            break;

        case I2S_BPC_24:
            reg = I2S_CON_DATA_NUM_24;
            break;
        default:
            return 0;
    }

    // LR clock
    switch(i2s.clk)
    {
        case I2S_32FS:
            reg = (I2S_CON_BIT_CH_32 | I2S_CON_DATA_NUM_16);
            break;

        case I2S_48FS:
            reg |= I2S_CON_BIT_CH_48;
            break;

        case I2S_64FS:
            reg |= I2S_CON_BIT_CH_64;
            break;
        default:
            return 0;
    }

    // format
    switch(i2s.format)
    {
        case I2S_BASIC:
            reg |= I2S_CON_I2S_MODE_BASIC;
            break;

        case I2S_LEFT_JUSTIFIED:
            reg |= I2S_CON_I2S_MODE_LEFT_JUSTIFIED;
            break;

        case I2S_RIGHT_JUSTIFIED:
            reg |= I2S_CON_I2S_MODE_RIGHT_JUSTIFIED;
            break;
        default:
            return -1;
    }
    writeb(reg , HDMI_SS_I2S_CON_2);

    return 1;

}


/**
 * Set Audio Clock Recovery and Audio Infoframe packet -@n
 * based on sampling frequency.
 * @param   freq   [in] Sampling frequency
 * @return  If argument is invalid, return 0;Otherwise return 1.
 */
int hdmi_set_audio_sample_freq(enum SamplingFreq freq)
{
    unsigned char reg;
    unsigned int n;
    int ret = 1;

    // check param
    if ( freq > sizeof(ACR_N_params)/sizeof(unsigned int))
        return 0;

    // set ACR packet
    // set N value
    n = ACR_N_params[freq];
    reg = n & 0xff;
    writeb(reg,HDMI_ACR_N0);
    reg = (n>>8) & 0xff;
    writeb(reg,HDMI_ACR_N1);
    reg = (n>>16) & 0xff;
    writeb(reg,HDMI_ACR_N2);

    // set as measure cts mode
    writeb(ACR_MEASURED_CTS_MODE,HDMI_ACR_CON);

    // set AUI packet
    reg = readb(HDMI_AUI_BYTE2) & ~HDMI_AUI_SF_MASK;

    switch (freq)
    {
        case SF_32KHZ:
            reg |= HDMI_AUI_SF_SF_32KHZ;
            break;

        case SF_44KHZ:
            reg |= HDMI_AUI_SF_SF_44KHZ;
            break;

        case SF_88KHZ:
            reg |= HDMI_AUI_SF_SF_88KHZ;
            break;

        case SF_176KHZ:
            reg |= HDMI_AUI_SF_SF_176KHZ;
            break;

        case SF_48KHZ:
            reg |= HDMI_AUI_SF_SF_48KHZ;
            break;

        case SF_96KHZ:
            reg |= HDMI_AUI_SF_SF_96KHZ;
            break;

        case SF_192KHZ:
            reg |= HDMI_AUI_SF_SF_192KHZ;
            break;

        default:
            ret = 0;
    }

    writeb(reg, HDMI_AUI_BYTE2);

    return ret;
}

/**
 * Set HDMI audio output packet type.
 * @param   packet   [in] Audio packet type
 * @return  If argument is invalid, return 0;Otherwise return 1.
 */
int hdmi_set_audio_packet_type(enum HDMIASPType packet)
{
    int ret = 1;
    unsigned char reg;

    reg = readb(HDMI_ASP_CON);
    reg &= ~ASP_TYPE_MASK;

    switch (packet)
    {
        case HDMI_ASP:
        {
            reg |= ASP_LPCM_TYPE;
            break;
        }
        case HDMI_DSD:
        {
            reg |= ASP_DSD_TYPE;
            break;
        }
        case HDMI_HBR:
        {
            reg |= ASP_HBR_TYPE;
            break;
        }
        case HDMI_DST:
        {
            reg |= ASP_DST_TYPE;
            break;
        }
        default:
            ret = 0;
    }
    writeb(reg,HDMI_ASP_CON);
    return ret;
}

/**
 * Set layout and sample present fields in Audio Sample Packet -@n
 * and channel number field in Audio InfoFrame packet.
 * @param   channel   [in]  Number of channels
 * @return  If argument is invalid, return 0;Otherwise return 1.
 */
int hdmi_set_audio_channel_number(enum ChannelNum channel)
{
    int ret = 1;
    unsigned char reg;

    reg = readb(HDMI_ASP_CON);
    // clear field
    reg &= ~(ASP_MODE_MASK|ASP_SP_MASK);

    // set layout & SP_PRESENT on ASP_CON
    // set AUI Packet
    switch (channel)
    {
        case CH_2:
            reg |= (ASP_LAYOUT_0|ASP_SP_0);
            writeb(AUI_CC_2CH,HDMI_AUI_BYTE1);
            break;
        case CH_3:
            reg |= (ASP_LAYOUT_1|ASP_SP_0|ASP_SP_1);
            writeb(AUI_CC_3CH,HDMI_AUI_BYTE1);
            break;
        case CH_4:
            reg |= (ASP_LAYOUT_1|ASP_SP_0|ASP_SP_1);
            writeb(AUI_CC_4CH,HDMI_AUI_BYTE1);
            break;
        case CH_5:
            reg |= (ASP_LAYOUT_1|ASP_SP_0|ASP_SP_1|ASP_SP_2);
            writeb(AUI_CC_5CH,HDMI_AUI_BYTE1);
            break;
        case CH_6:
            reg |= (ASP_LAYOUT_1|ASP_SP_0|ASP_SP_1|ASP_SP_2);
            writeb(AUI_CC_6CH,HDMI_AUI_BYTE1);
            break;
        case CH_7:
            reg |= (ASP_LAYOUT_1|ASP_SP_0|ASP_SP_1|ASP_SP_2|ASP_SP_3);
            writeb(AUI_CC_7CH,HDMI_AUI_BYTE1);
            break;
        case CH_8:
            reg |= (ASP_LAYOUT_1|ASP_SP_0|ASP_SP_1|ASP_SP_2|ASP_SP_3);
            writeb(AUI_CC_8CH,HDMI_AUI_BYTE1);
            break;
        default:
            ret = 0;
    }
    writeb(reg,HDMI_ASP_CON);
    return ret;
}

/**
 * Set requested audio mode.
 * @param   hdmi_audio_mode [in]    Audio mode to set
 * @return If success, return 1;Otherwise, return 0
 */
int hdmi_set_audio_mode(const struct HDMIAudioParameter * const hdmi_audio_mode)
{
	unsigned char reg;
	
    // check paramter
    if (!hdmi_audio_mode)
    {
        DPRINTF("bad args: hdmi_audio_mode\n");
        return 0;
    }


	// [I2S_CLK_CON.i2s_en]
	//  You must set i2s_en, after other registers are configured. 
	//  when you want to reset the i2s, this register is 0 1. 
	// disable audio
	writeb(I2S_CLK_CON_DISABLE,HDMI_SS_I2S_CLK_CON);
	
    // setting audio input port parameters
    switch (hdmi_audio_mode->inputPort)
    {
        int wordlength,codingtype;

        case I2S_PORT:
        {
            // set CUV
            // sample freq
            if (!hdmi_set_audio_cuv_samplefreq(hdmi_audio_mode->sampleFreq))
            {
                DPRINTF("(hdmi_audio_mode->sampleFreq) Not Available Arg\n");
                return 0;
            }

            // channel number
            if (!hdmi_set_audio_cuv_channelnum(hdmi_audio_mode->channelNum))
            {
                DPRINTF("(hdmi_audio_mode->channelNum) Not Available Arg\n");
                return 0;
            }

            if (hdmi_audio_mode->formatCode == LPCM_FORMAT)
            {
                codingtype = CUV_LPCM;
            }
            else
            {
                codingtype = CUV_NLPCM;
            }

            if (!hdmi_set_audio_cuv_codingtype(codingtype))
            {
                DPRINTF("(codingtype) Not Available Arg\n");
                return 0;
            }

            // word length
            if (codingtype == CUV_LPCM)
            {
                switch(hdmi_audio_mode->wordLength)
                {
                    case WORD_16:
                    {
                        wordlength = CUV_WL_20_16;
                        break;
                    }
                    case WORD_17:
                    {
                        wordlength = CUV_WL_20_17;
                        break;
                    }
                    case WORD_18:
                    {
                        wordlength = CUV_WL_20_18;
                        break;
                    }
                    case WORD_19:
                    {
                        wordlength = CUV_WL_20_19;
                        break;
                    }
                    case WORD_20:
                    {
                        wordlength = CUV_WL_24_20;
                        break;
                    }
                    case WORD_21:
                    {
                        wordlength = CUV_WL_24_21;
                        break;
                    }
                    case WORD_22:
                    {
                        wordlength = CUV_WL_24_22;
                        break;
                    }
                    case WORD_23:
                    {
                        wordlength = CUV_WL_24_23;
                        break;
                    }
                    case WORD_24:
                    {
                        wordlength = CUV_WL_24_24;
                        break;
                    }
                    default:
                    {
                        wordlength = CUV_WL_24_NOT_DEFINED;
                        break;
                    }
                } // switch

	            if (!hdmi_set_audio_cuv_wordlength(wordlength))
	            {
	                DPRINTF("(wordlength) Not Available Arg\n");
	                return 0;
	            }
            } // if (LPCM)

			writeb(0x01, HDMI_SS_I2S_CH_ST_CON);

			hdmi_set_audio_i2s_parameter(hdmi_audio_mode->i2sParam);
			
            break;
        }
        case SPDIF_PORT:
        {
			//TODO: implement
			
            break;
        }
        case DSD_PORT:
        {
            //TODO: implement
            break;
        }
        default:
            DPRINTF("not available arg on input port\n");
            return 0;
    }

    // set input port
    if (!hdmi_set_audio_inputport(hdmi_audio_mode->inputPort))
    {
        DPRINTF("(hdmi_audio_mode->inputPort) Not Available Arg\n");
        return 0;
    }

    // set audio channel num on audio sample packet and audio infoframe
    if (!hdmi_set_audio_channel_number(hdmi_audio_mode->channelNum))
    {
        DPRINTF("(hdmi_audio_mode->channelNum) Not available Arg\n");
        return 0;
    }

    // set audio clock recovery packet and audio infoframe sample freq
    reg = readb(HDMI_CON_0);

    if ( !hdmi_set_audio_sample_freq(hdmi_audio_mode->sampleFreq) )
    {
        DPRINTF("(hdmi_audio_mode->sampleFreq) Not available Arg\n");
        return 0;
    }
    // set audio enable
    writeb(reg|HDMI_ASP_ENABLE ,HDMI_CON_0);

    // get hdmi audio parameters

    if (hdmi_audio_mode->outPacket == HDMI_ASP)
    {
        // reset sampling freq fields in AUI
	    reg = readb(HDMI_AUI_BYTE2) & ~HDMI_AUI_SF_MASK;
	    writeb(reg, HDMI_AUI_BYTE2);
    }

    // set audio packet type
    if (!hdmi_set_audio_packet_type(hdmi_audio_mode->outPacket))
    {
        DPRINTF("(hdmi_audio_mode->outPacket) Not available Arg\n");
        return 0;
    }

    return 1;
}

/**
 * Set speaker allocation information.
 * @param   speaker [in]    Value to set. @n
 *                          For the values, refer CEA-861 Spec.
 * @return If success, return 1;Otherwise, return 0
 */
int hdmi_set_speaker_allocation(const unsigned int speaker)
{
    writeb(speaker,HDMI_AUI_BYTE4);
    return 1;
}

/**
 * Enable/Disable blue screen mode.
 * @param   enable [in]    1 to enable blue screen mode @n
 *                         0 to disable blue screen mode
 * @return If success, return 1;Otherwise, return 0
 */
int hdmi_set_bluescreen(const unsigned char enable)
{
    unsigned char reg;

    // get arg

    reg = readb(HDMI_CON_0);
    if (enable) // if on
    {
        writeb(reg|HDMI_BLUE_SCR_ENABLE,HDMI_CON_0);
    }
    else // if off
    {
        writeb(reg &~HDMI_BLUE_SCR_ENABLE,HDMI_CON_0);
    }

    return 1;
}

/**
 * Enable/Disable audio mute.
 * @param   enable [in]    0 to enable audio mute @n
 *                         1 to disable audio mute
 * @return If success, return 1;Otherwise, return 0
 */
int hdmi_set_audio_enable(const unsigned char enable)
{
    unsigned char reg;

    DPRINTF("(audio mute) %d\n", enable);

    reg = readb(HDMI_CON_0);
    // enable audio output
    if (enable)
    {
        hdmi_aui_update_checksum();
        writeb(TRANSMIT_EVERY_VSYNC,HDMI_AUI_CON);
    //  writeb(TRANSMIT_ONCE,HDMI_AUI_CON);
    //  writeb(ACR_MEASURED_CTS_MODE,HDMI_ACR_CON);
        writeb(reg|HDMI_ASP_ENABLE,HDMI_CON_0);
    }
    else // disable encryption
    {
        writeb(DO_NOT_TRANSMIT,HDMI_AUI_CON);
        writeb(DO_NOT_TRANSMIT,HDMI_ACR_CON);
        writeb(reg& ~HDMI_ASP_ENABLE,HDMI_CON_0);
    }

    return 1;
}

/**
 * Enable/Disable audio mute.
 * @param   enable [in]    1 to enable audio mute @n
 *                         0 to disable audio mute
 * @return If success, return 1;Otherwise, return 0
 */
int hdmi_set_audio_mute(const unsigned char enable)
{
	unsigned char parm;

	if (enable)	parm = 0;
	else		parm = 1;

	hdmi_set_audio_enable(parm);

    return 1;
}

/**
 * Enable/Disable A/V mute mode.
 * @param   enable [in]    1 to enable A/V mute mode @n
 *                         0 to disable A/V mute mode
 * @return If success, return 1;Otherwise, return 0
 */
int hdmi_set_av_mute(const unsigned char enable)
{
	unsigned char reg;
	
	DPRINTF("(a/v mute) %d\n", enable);

    reg = readb(HDMI_MODE_SEL) & HDMI_MODE_SEL_HDMI;
    if (reg)
    {
        if (enable)
        {
            // set AV Mute
            writeb(GCP_AVMUTE_ON,HDMI_GCP_BYTE1);
            writeb(GCP_TRANSMIT_EVERY_VSYNC,HDMI_GCP_CON);
        }
        else
        {
            // clear AV Mute
            writeb(GCP_AVMUTE_OFF, HDMI_GCP_BYTE1);
            writeb(GCP_TRANSMIT_EVERY_VSYNC,HDMI_GCP_CON);
        }
    }

    return 1;
}

int hdmi_check_phy_ready(void)
{
	unsigned char phy_status;
	unsigned int phy_chk_cnt = 0, regl = 0;

	do
	{
		phy_status = readb(HDMI_PHY_STATUS);

		if(phy_chk_cnt++ == 200)
			break;

		{volatile int ttt;for(ttt=0;ttt<0x5000;ttt++);}
	}while(!phy_status);

	if(phy_status)	{
		DPRINTF("%s phy is ready\n", __func__);
	}
	else 	{
		DPRINTF("%s phy is not ready\n", __func__);
		DPRINTF("%s try phy reset again\n", __func__);
		// HDMI PHY Reset
		regl = readl(DDICFG_HDMICTRL);
		writel(regl | HDMICTRL_RESET_HDMI, DDICFG_HDMICTRL);
		{volatile int ttt;for(ttt=0;ttt<0x5000;ttt++);}
		writel(regl & ~HDMICTRL_RESET_HDMI, DDICFG_HDMICTRL);

		phy_chk_cnt = 0;

		do
		{
			phy_status = readb(HDMI_PHY_STATUS);

			if(phy_chk_cnt++ == 200)
				break;

			{volatile int ttt;for(ttt=0;ttt<0x5000;ttt++);}
		}while(!phy_status);
		
		DPRINTF("%s loop:%d Phy status= %d\n", __func__,phy_chk_cnt, phy_status);

		if(phy_status)
			DPRINTF("%s phy is ready\n", __func__);
		else
			DPRINTF("%s phy is not ready\n", __func__);
	}

	return 1;
}
/**
 * hdmi_phy_reset.
 */

/**
 * Enable HDMI output.
 */
void hdmi_start(void)
{
    unsigned char reg,mode;

    // check HDMI mode
    mode = readb(HDMI_MODE_SEL) & HDMI_MODE_SEL_HDMI;
    reg = readb(HDMI_CON_0);

    // enable external vido gen.
    writeb(HDMI_EXTERNAL_VIDEO,HDMI_VIDEO_PATTERN_GEN);

    if (mode) // HDMI
    {
        // enable AVI packet: mandatory
        // update avi packet checksum
        hdmi_avi_update_checksum();
        // enable avi packet
        writeb(TRANSMIT_EVERY_VSYNC,HDMI_AVI_CON);

        // check if audio is enable
        if (readb(HDMI_ACR_CON))
        {
            // enable aui packet
			hdmi_aui_update_checksum();
			writeb(TRANSMIT_EVERY_VSYNC,HDMI_AUI_CON);
		//	writeb(TRANSMIT_ONCE,HDMI_AUI_CON);
			reg |= HDMI_ASP_ENABLE;
        }

        // check if it is deep color mode or not
        if (readb(HDMI_DC_CONTROL))
        {
            // enable gcp
            writeb(GCP_TRANSMIT_EVERY_VSYNC,HDMI_GCP_CON);
        }
        // enable hdmi
	#if defined(TELECHIPS)
	writeb(reg|HDMI_SYS_ENABLE,HDMI_CON_0);
	#else
        writeb(reg|HDMI_SYS_ENABLE|HDMI_ENCODING_OPTION_ENABLE,HDMI_CON_0);
	#endif
    }
    else // DVI
    {
        // disable all packet
        writeb(DO_NOT_TRANSMIT,HDMI_AUI_CON);
        writeb(DO_NOT_TRANSMIT,HDMI_AVI_CON);
        writeb(DO_NOT_TRANSMIT,HDMI_GCP_CON);

        // enable hdmi without audio
        reg &= ~HDMI_ASP_ENABLE;
	#if defined(TELECHIPS)
	writeb(reg|HDMI_SYS_ENABLE,HDMI_CON_0);
	#else
        writeb(reg|HDMI_SYS_ENABLE|HDMI_ENCODING_OPTION_ENABLE,HDMI_CON_0);
	#endif
    }

    return;
}

/**
 * Disable HDMI output.
 */
void hdmi_stop(void)
{
    unsigned char reg;

    reg = readb(HDMI_CON_0);
    writeb(reg & ~HDMI_SYS_ENABLE,HDMI_CON_0);

}


//==============================================================================
//                 Section x - Application
//==============================================================================


void disp_init_hdmi(void)
{
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
	};

	const struct HDMIAudioParameter audio = {
	/*	audio.inputPort 		=*/	I2S_PORT,
	/*	audio.outPacket 		=*/	HDMI_ASP,
	/*	audio.formatCode		=*/	LPCM_FORMAT,
	/*	audio.channelNum		=*/	CH_2,
	/*	audio.sampleFreq		=*/	SF_44KHZ,
	/*	audio.wordLength		=*/	WORD_16,

	/*	audio.i2sParam.bpc	=*/	  { I2S_BPC_16,
	/*	audio.i2sParam.format	=*/	I2S_BASIC,
	/*	audio.i2sParam.clk		=*/	I2S_64FS },
	};

	hdmi_set_hdmi_mode(video.mode);

	hdmi_set_video_mode(&video);

	if (video.mode == HDMI)
		hdmi_set_audio_mode(&audio);

//	hdmi_stop();
	hdmi_start();

}

#endif	//#if defined(DEFAULT_DISPLAY_HDMI) || defined(DEFAULT_DISPLAY_OUTPUT_DUAL)

