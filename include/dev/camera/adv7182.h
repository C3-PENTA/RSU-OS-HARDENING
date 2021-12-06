
/****************************************************************************
  Copyright (C) 2014 Telechips, Inc.
****************************************************************************/

#ifndef __TCC_ADV7182_H
#define __TCC_ADV7182_H

#define ADV7182_I2C_ADDR	0x42	// Module I2C Slave address
#define ADV7182_I2C_CH_NO	1	// I2C Channel number

struct sensor_gpio {
	int pwr_port;	// setting for power control pin
	int pwd_port;	// setting for stand by control pin
	int rst_port;	// setting for reset control pin

	int data_func;	// settung fot gpio function(data & sync)
	int data[25];	// max data pin count(24pin) + 1(for NULL pointer)
	int hsync;	// setting for H - Sync polarity
	int vsync;	// setting for V - Sync polarity
	int pclk;	// Settung for P Clocks(CLKI) Phase
};

struct sensor_reg {
	unsigned short reg;
	unsigned short val;
};

extern int sensor_if_connect_api(SENSOR_FUNC_TYPE * sensor_func);

#endif

