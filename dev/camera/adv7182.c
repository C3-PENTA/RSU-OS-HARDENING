
/****************************************************************************
  Copyright (C) 2014 Telechips, Inc.
****************************************************************************/

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <reg.h>
#include <platform/iomap.h>
#include <dev/fbcon.h>
#include <plat/cpu.h>

#include <tcc_pmap_avn.h>
#include <platform/irqs.h>
#include <platform/tcc_ckc.h>

#include <i2c.h>
#include <dev/gpio.h>
#include <tcc_lcd.h>
#include <platform/gpio.h>
#include <platform/vioc_cam_plugin.h>

#include <dev/camera/camera.h>
#include <dev/camera/sensor_if.h>
#include <dev/camera/adv7182.h>

struct sensor_gpio gpio_info = {
	.pwr_port = (int)NULL,
	.pwd_port = (int)NULL,
	.rst_port = TCC_GPF(11),

	.vsync = TCC_GPF(2),
	.hsync = TCC_GPF(1),
	.pclk  = TCC_GPF(0),

	.data = {TCC_GPF(3), TCC_GPF(4), TCC_GPF(5), TCC_GPF(6), TCC_GPF(7), TCC_GPF(8), TCC_GPF(9), TCC_GPF(10)},

	.data_func = GPIO_FN1
};

void tcc_set_gpio_pin(void) {
	int idxData;

	// Configure power.
	gpio_config(gpio_info.pwr_port, GPIO_OUTPUT);
	gpio_config(gpio_info.pwd_port, GPIO_OUTPUT);
	gpio_config(gpio_info.rst_port, GPIO_OUTPUT);

	// Configure signal port.
	gpio_config(gpio_info.hsync, gpio_info.data_func);
	gpio_config(gpio_info.vsync, gpio_info.data_func);
	gpio_config(gpio_info.pclk,  gpio_info.data_func);

	// Configue data port.
	for(idxData=0; gpio_info.data[idxData] != (int)NULL; idxData++) {
		dprintf(INFO,"gpio pin is %d\n", gpio_info.data[idxData]);
		gpio_config(gpio_info.data[idxData], gpio_info.data_func);
	}

	// Power-up.
	gpio_set(gpio_info.pwr_port, 1);
	gpio_set(gpio_info.pwd_port, 1);
	gpio_set(gpio_info.rst_port, 0);
}

struct tcc_cif_parameters parameters_data = {
	.Cam_p_clock_pol = NEGATIVE_EDGE,
	.Cam_v_sync_pol = ACT_HIGH,
	.Cam_h_sync_pol = ACT_HIGH,
	.Cam_de_pol = ACT_LOW,
	.Cam_field_bfield_low = OFF,
	.Cam_gen_field_en = OFF,
	.Cam_conv_en = ON,
	.Cam_hsde_connect_en = OFF,
	.Cam_vs_mask = OFF,
	.Cam_input_fmt = FMT_YUV422_8BIT,
	.Cam_data_order = ORDER_RGB,
	.Cam_intl_en = ON,
	.Cam_intpl_en = OFF,
	.Cam_preview_w = 720,
	.Cam_preview_h = 240,

	.PGL_use = 0,
	.PGL_addr	= PARKING_GUIDE_BASE,
	.PGL_width = 1024,
	.PGL_height = 600,
	.PGL_format = 12,	// PGL format is RGB8888

	.Lcdc_Frame_width = 1024,
	.Lcdc_Frame_height = 600,
	.Lcdc_Image_width = 1024,
	.Lcdc_Image_height = 600,
	.Lcdc_offset_x = 0,
	.Lcdc_offset_y = 0,
	.Lcdc_address0 = (unsigned int)REAR_CAMERA_BASE,
	.Lcdc_address1 = (unsigned int)NULL,
	.Lcdc_address2 = (unsigned int)NULL,
	.Lcdc_format = TCC_LCDC_IMG_FMT_RGB565,

	.R_gear_port = TCC_GPD(8),
	.R_gear_active = 0,			// low active
	.CIF_Port_num = 4,
	.Viqe_area = (unsigned int)JPEG_RAW_BASE
};

static struct sensor_reg sensor_regs_initialize[] = {
	{0x0f, 0x80},
	{0x0e, 0x00},
	{0xFF, 10},
	{0x0f, 0x00},
#if 1
	{0x52, 0xcd},
	{0x00, 0x00},
	{0x0e, 0x80},
	{0x9c, 0x00},
	{0x9c, 0xff},
	{0x0e, 0x00},
	{0x17, 0x41},
	{0x03, 0x0c},
	{0x04, 0x07},
	{0x13, 0x00},
	{0x1d, 0x40},
#else
	{0x52, 0xcd},
	{0x00, 0x00},
	{0x0e, 0x80},
	{0x9c, 0x00},
	{0x9c, 0xff},
	{0x0e, 0x00},
	{0x0e, 0x80},
	{0xd9, 0x44},
	{0x0e, 0x40},
	{0xe0, 0x01},
	{0x0e, 0x00},
	{0x17, 0x41},
	{0x03, 0x0c},
	{0x04, 0x07},
	{0x13, 0x00},
	{0x1d, 0x40},
#endif
	{0xFF, 0xFF}
};

struct sensor_reg * sensor_regs_type_and_encode[CAM_TYPE_MAX][CAM_ENC_MAX] = {
	// CAM_TYPE_DEFAULT
	{
		sensor_regs_initialize,     //sensor_camera_ntsc,
		NULL,
	},
};

static int tcc_cif_i2c_write(unsigned char* data, unsigned short reg_bytes, unsigned short data_bytes) {
	unsigned short bytes = reg_bytes + data_bytes;
	int ret = 0;
	ret = i2c_xfer(ADV7182_I2C_ADDR, bytes, data, 0, 0, ADV7182_I2C_CH_NO);

	if(ret != 0)
	{
		dprintf(INFO,"write error!!!! \n");
		return -1;
	}
	return 0;
}

static int write_regs(const struct sensor_reg reglist[]) {
	int err;
	int err_cnt = 0;
	int sleep_cnt = 100;
	unsigned char data[132];
	unsigned char bytes;
	const struct sensor_reg *next = reglist;

	while (!((next->reg == 0xFF) && (next->val == 0xFF))) {
		if(next->reg == 0xFF && next->val != 0xFF) {
			thread_sleep(next->val);
//			mdelay(next->val);
			sleep_cnt = 100;
			next++;
		} else {
			bytes = 0;
			data[bytes]= (unsigned char)next->reg&0xff; 	bytes++;
			data[bytes]= (unsigned char)next->val&0xff; 	bytes++;

			err = tcc_cif_i2c_write(data, 1, 1);
			if (err) {
				err_cnt++;
				if(err_cnt >= 3) {
					dprintf(INFO,"ERROR: Sensor I2C !!!! \n");
					return err;
				}
			} else {
				err_cnt = 0;
				next++;
			}
		}
	}
	return 0;
}

int sensor_tune(unsigned int camera_type, unsigned int camera_encode) {
	dprintf(INFO, "!@#---- %s()\n", __func__);

	if(((CAM_TYPE_MAX <= camera_type) || (sensor_regs_type_and_encode[camera_type][camera_encode] == NULL)) ||
	   ((CAM_ENC_MAX <= camera_encode) || (sensor_regs_type_and_encode[camera_type][camera_encode] == NULL))) {
		dprintf(INFO, "!@#---- %s() - WRONG arguments\n", __func__);
		return -1;
	}
	return write_regs(sensor_regs_type_and_encode[camera_type][camera_encode]);
}

int sensor_open(void) {
	dprintf(INFO, "!@#---- %s()\n", __func__);

	tcc_set_gpio_pin();

	// Power-up sequence
	gpio_set(gpio_info.pwr_port, 1);	// sensor_power_enable
	tcc_cif_delay(10);

	gpio_set(gpio_info.pwd_port, 1);	// sensor_powerdown_disable
	tcc_cif_delay(40);

	gpio_set(gpio_info.rst_port, 0);	// sensor_reset_high
	tcc_cif_delay(10);

	gpio_set(gpio_info.rst_port, 1);	// sensor_reset_high
	tcc_cif_delay(15);

	return sensor_tune(CAM_TYPE_DEFAULT, CAM_ENC_DEFAULT);
}

int sensor_close(void) {
	dprintf(INFO, "!@#---- %s()\n", __func__);
	return 0;
}

int sensor_if_connect_api(SENSOR_FUNC_TYPE * sensor_func) {
	sensor_func->close	= sensor_close;
	sensor_func->open	= sensor_open;
	sensor_func->tune	= sensor_tune;

	return 0;
}

