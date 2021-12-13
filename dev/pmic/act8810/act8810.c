/***************************************************************************************
*	FileName    : act8810.c
*	Description : Enhanced single Cell Li-Battery and Power System Management IC driver
****************************************************************************************
*
*	TCC Board Support Package
*	Copyright (c) Telechips, Inc.
*	ALL RIGHTS RESERVED
*
****************************************************************************************/

//#include <common.h>
//#include <dev/i2c.h>
#include <platform/reg_physical.h>
#include <lcd.h>
#include <i2c.h>
#include <debug.h>

#if defined(ACT8810_PMIC)

/* device address */
#define SLAVE_ADDR_ACT8810	0xB4

/* command registers */
#define ACT8810_OUT1_REG			0x13

#define ACT8810_OUT1_1_20V		0x17
#define ACT8810_OUT1_1_30V		0x1B
#define ACT8810_OUT1_1_40V		0x1F
#define ACT8810_OUT1_1_50V		0x23

void act8810_init(void)
{
	unsigned int i2c_ch = I2C_CH0;
	unsigned char DestAddress;
	unsigned char i2cData_data[2] = {0,0};	/*{CMD,DATA}*/

	DestAddress = SLAVE_ADDR_ACT8810;

	i2cData_data[0] = ACT8810_OUT1_REG;
	i2cData_data[1] = ACT8810_OUT1_1_50V;
	i2c_xfer(DestAddress, 2, i2cData_data, 0, 0, i2c_ch);
	
	dprintf(INFO, "%s\n", __func__);
}

#endif

/************* end of file *************************************************************/
