/****************************************************************************
 * Copyright (C) 2015 Telechips Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions
 * andlimitations under the License.
 ****************************************************************************/

#include <i2c.h>
#include <dev/gpio.h>
#include <platform/gpio.h>
#include <malloc.h>

#define PCA950_I2C_ADDR	0x20

#define PCA9506_INPUT_REG 0x00
#define PCA9506_OUTPUT_REG 0x08
#define PCA9506_POLARITY_REG 0x10
#define PCA9506_IOCFG_REG 0x18
#define PCA9506_INT_REG 0x20
#define PCA9506_MAX_BANK 5

extern int register_ext_gpio(unsigned gpio_id, struct ext_gpio *gpios);

static int pca950x_config(struct ext_gpio *ext, unsigned n, unsigned flags)
{
	unsigned port = n&GPIO_BITMASK;
	unsigned bank = port/8;
	unsigned char mode[2] = {PCA9506_IOCFG_REG + bank ,0};
	unsigned char data[2] = {PCA9506_OUTPUT_REG + bank ,0};
	i2c_xfer(ext->addr, 1, &data[0], 1, &data[1], ext->i2c_ch);
	i2c_xfer(ext->addr, 1, &mode[0], 1, &mode[1], ext->i2c_ch);

	if (bank >= PCA9506_MAX_BANK)
		return -1;

	port %= 8;
	if (flags & GPIO_OUTPUT) {
		if (flags & GPIO_HIGH)
			data[1] |= 1<<port;
		if (flags & GPIO_LOW)
			data[1] &= ~(1<<port);
		mode[1] &= ~(1<<port);
	} else
		mode[1] |= 1<<port;

	i2c_xfer(ext->addr, 2, data, 0, 0, ext->i2c_ch);
	i2c_xfer(ext->addr, 2, mode, 0, 0, ext->i2c_ch);

	return 0;
}

static void pca950x_set(struct ext_gpio *ext, unsigned n, unsigned on)
{
	unsigned port = n&GPIO_BITMASK;
	unsigned bank = port/8;
	unsigned char data[2] = {PCA9506_OUTPUT_REG + bank , 0};
	i2c_xfer(ext->addr, 1, &data[0], 1, &data[1], ext->i2c_ch);

	if (bank >= PCA9506_MAX_BANK)
		return;

	port %= 8;
	on ? (data[1] |= 1<<port) : (data[1] &= ~(1<<port));
	i2c_xfer(ext->addr, 2, data, 0, 0, ext->i2c_ch);
}

static int pca950x_get(struct ext_gpio *ext, unsigned n)
{
	unsigned port = n&GPIO_BITMASK;
	unsigned bank = port/8;
	unsigned char data[2] = {PCA9506_INPUT_REG + bank , 0};
	i2c_xfer(ext->addr, 1, &data[0], 1, &data[1], ext->i2c_ch);

	if (bank >= PCA9506_MAX_BANK)
		return 0;

	port %= 8;
	return (data[1] & (1<<port)) ? 1 : 0;
}

/*
 * Initialize PCA950X device
 * i2c_ch: i2c controller channel
 * id    : pca950x address id.
 *         ex) 00,01,02,...
 * bank0, bank1, bank2, bank3, bank4
 * input, output, Polarity, IO, mask
 */
void pca950x_init(unsigned gpio_id, unsigned i2c_ch, unsigned id, unsigned char bank, unsigned char direct, unsigned char level)
{
	struct ext_gpio *gpios;

	unsigned char DestAddress;
	unsigned char data[2] = {0,0};	/*{REG,DATA,}*/

	DestAddress = (PCA950_I2C_ADDR+(id&0x3))<<1;
	
	data[0] = PCA9506_IOCFG_REG + bank;
	data[1] = direct;
	i2c_xfer(DestAddress, 2, data, 0, 0, i2c_ch);

	data[0] = PCA9506_OUTPUT_REG + bank;
	data[1] = level;
	i2c_xfer(DestAddress, 2, data, 0, 0, i2c_ch);

	gpios = (struct ext_gpio *)malloc(sizeof(struct ext_gpio));
	if (gpios) {
		gpios->addr = DestAddress;
		gpios->i2c_ch = i2c_ch;
		gpios->gpio_id = gpio_id;
		gpios->config = &pca950x_config;
		gpios->set = &pca950x_set;
		gpios->get = &pca950x_get;
		if (register_ext_gpio(gpio_id, gpios))
			free(gpios);
	}
}



/************* end of file *************************************************************/
