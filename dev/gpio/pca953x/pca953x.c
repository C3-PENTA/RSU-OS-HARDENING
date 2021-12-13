/****************************************************************************
 * Copyright (C) 2014 Telechips Inc.
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

#define PCA953_I2C_ADDR	0x74

extern int register_ext_gpio(unsigned gpio_id, struct ext_gpio *gpios);

enum pca9539_cmd
{
	PCA9539_INPUT_0		= 0,
	PCA9539_INPUT_1		= 1,
	PCA9539_OUTPUT_0	= 2,
	PCA9539_OUTPUT_1	= 3,
	PCA9539_INVERT_0	= 4,
	PCA9539_INVERT_1	= 5,
	PCA9539_DIRECTION_0	= 6,
	PCA9539_DIRECTION_1	= 7,
};

enum pca9538_cmd
{
	PCA9538_INPUT_0		= 0,
	PCA9538_OUTPUT_0	= 1,
	PCA9538_INVERT_0	= 2,
	PCA9538_DIRECTION_0	= 3,
};

static int pca953x_config(struct ext_gpio *ext, unsigned n, unsigned flags)
{
	unsigned port = n&GPIO_BITMASK;
	unsigned idx = port/8;
	unsigned char mode[3] = {PCA9539_DIRECTION_0,0,0};
	unsigned char data[3] = {PCA9539_OUTPUT_0,0,0};
	i2c_xfer(ext->addr, 1, &data[0], 2, &data[1], ext->i2c_ch);
	i2c_xfer(ext->addr, 1, &mode[0], 2, &mode[1], ext->i2c_ch);

	if (idx >= 2)
		return -1;

	port %= 8;
	if (flags & GPIO_OUTPUT) {
		if (flags & GPIO_HIGH)
			data[idx+1] |= 1<<port;
		if (flags & GPIO_LOW)
			data[idx+1] &= ~(1<<port);
		mode[idx+1] &= ~(1<<port);
	} else
		mode[idx+1] |= 1<<port;

	i2c_xfer(ext->addr, 3, data, 0, 0, ext->i2c_ch);
	i2c_xfer(ext->addr, 3, mode, 0, 0, ext->i2c_ch);

	return 0;
}

static void pca953x_set(struct ext_gpio *ext, unsigned n, unsigned on)
{
	unsigned port = n&GPIO_BITMASK;
	unsigned idx = port/8;
	unsigned char data[3] = {PCA9539_OUTPUT_0,0,0};
	i2c_xfer(ext->addr, 1, &data[0], 2, &data[1], ext->i2c_ch);

	if (idx >= 2)
		return;

	port %= 8;
	on ? (data[idx+1] |= 1<<port) : (data[idx+1] &= ~(1<<port));
	i2c_xfer(ext->addr, 3, data, 0, 0, ext->i2c_ch);
}

static int pca953x_get(struct ext_gpio *ext, unsigned n)
{
	unsigned port = n&GPIO_BITMASK;
	unsigned idx = port/8;
	unsigned char data[3] = {PCA9539_OUTPUT_0,0,0};
	i2c_xfer(ext->addr, 1, &data[0], 2, &data[1], ext->i2c_ch);

	if (idx >= 2)
		return 0;

	port %= 8;
	return (data[idx+1] & (1<<port)) ? 1 : 0;
}

/*
 * Initialize PCA953X device
 * i2c_ch: i2c controller channel
 * id    : pca953x address id.
 *         ex) 00,01,02,...
 * direct: port direction bit mask.
 *         b'1: output, b'0: input
 * level : high/low bit mask if the port set output mode.
 *         b'1: output, b'0: input
 */
void pca953x_init(unsigned gpio_id, unsigned i2c_ch, unsigned id, unsigned direct, unsigned level)
{
	struct ext_gpio *gpios;

	unsigned char DestAddress;
	unsigned char mode[3] = {0,0,0};	/*{CMD,PORT0,PORT1}*/
	unsigned char data[3] = {0,0,0};	/*{CMD,PORT0,PORT1}*/

	DestAddress = (PCA953_I2C_ADDR+(id&0x3))<<1;
	mode[0] = PCA9539_DIRECTION_0;
	mode[1] = (~direct)&0xFF;
	mode[2] = ((~direct)>>8)&0xFF;
	data[0] = PCA9539_OUTPUT_0;
	data[1] = level&0xFF;
	data[2] = (level>>8)&0xFF;
	i2c_xfer(DestAddress, 3, data, 0, 0, i2c_ch);
	i2c_xfer(DestAddress, 3, mode, 0, 0, i2c_ch);

	gpios = (struct ext_gpio *)malloc(sizeof(struct ext_gpio));
	if (gpios) {
		gpios->addr = DestAddress;
		gpios->i2c_ch = i2c_ch;
		gpios->gpio_id = gpio_id;
		gpios->config = &pca953x_config;
		gpios->set = &pca953x_set;
		gpios->get = &pca953x_get;
		if (register_ext_gpio(gpio_id, gpios))
			free(gpios);
	}
}
