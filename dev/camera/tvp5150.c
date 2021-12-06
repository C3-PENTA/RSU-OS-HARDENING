
/****************************************************************************
  Copyright (C) 2014 Telechips, Inc.
****************************************************************************/
/****************************************************************************
   Written by S.W.Hwang (Audio and Display Team, Telechips Inc.)
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
#include <dev/camera/tvp5150.h>

struct sensor_gpio gpio_data = {
	.pwr_port = TCC_GPEXT2(18),
	.pwd_port = TCC_GPEXT2(21),
	.rst_port = TCC_GPEXT2(20),
	.data_func = GPIO_FN1,
	.data = {TCC_GPF(3), TCC_GPF(4), TCC_GPF(5), TCC_GPF(6), TCC_GPF(7), TCC_GPF(8), TCC_GPF(9), TCC_GPF(10)},
	.hsync = TCC_GPF(1),
	.vsync = TCC_GPF(2),
	.pclk  = TCC_GPF(0)

};


struct tcc_cif_parameters parameters_data = {
	.Cam_p_clock_pol = NEGATIVE_EDGE,
	.Cam_v_sync_pol = ACT_HIGH,
	.Cam_h_sync_pol = ACT_HIGH,
	.Cam_de_pol = ACT_LOW,
	.Cam_field_bfield_low = OFF,
	.Cam_gen_field_en = ON,
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
	.PGL_addr = PARKING_GUIDE_BASE,
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
	
	.R_gear_port = TCC_GPC(22),
	.R_gear_active = 1, 		// high active
	.CIF_Port_num = 4,
	.Viqe_area = (unsigned int)JPEG_RAW_BASE
};

static struct sensor_reg sensor_initialize[] =
{
	// Force NTSC mode
	{0x03, 0x0D},	
	{0x12, 0x04},
	{0xC9, 0x00},		
	{0xFF, 0x00C8},
	{0xFF, 0x00C8},		
	{0xFF, 0xFF}
};

static int tcc_cif_i2c_write(unsigned char* data, unsigned short reg_bytes, unsigned short data_bytes)
{

	unsigned short bytes = reg_bytes + data_bytes;
	int ret = 0;
	ret = i2c_xfer(TVP5150_I2C_ADDR, bytes, data, 0, 0, TVP5150_I2C_CH_NO);

	if(ret != 0)
	{
		dprintf(INFO,"write error!!!! \n");
		return -1; 
	}
	return 0;
}

static int write_regs(const struct sensor_reg reglist[])
{
	int err;
	int err_cnt = 0;
	int sleep_cnt = 100;		
	unsigned char data[132];
	unsigned char bytes;
	const struct sensor_reg *next = reglist;

	while (!((next->reg == 0xFF) && (next->val == 0xFF)))
	{
		if(next->reg == 0xFF && next->val != 0xFF)
		{
			mdelay(next->val);
			sleep_cnt = 100;
			next++;
		}
		else
		{
		
			bytes = 0;
			data[bytes]= (unsigned char)next->reg&0xff; 	bytes++;
			data[bytes]= (unsigned char)next->val&0xff; 	bytes++;

			err = tcc_cif_i2c_write(data, 1, 1);

			if (err)
			{
				err_cnt++;
				if(err_cnt >= 3)
				{
					dprintf(INFO,"ERROR: Sensor I2C !!!! \n"); 
					return err;
				}
			}
			else
			{
				err_cnt = 0;
				next++;
			}
		}
	}

	return 0;
}


void tcc_set_gpio_pin()
{
	
	/***** gpio port setting *****/
	
	int data_count;

	for(data_count=0; gpio_data.data[data_count] != NULL; data_count++) {
		dprintf(INFO,"gpio setting!! gpio pin is %d \n", gpio_data.data[data_count]); 		
		gpio_config(gpio_data.data[data_count], gpio_data.data_func);
	}	
	gpio_config(gpio_data.hsync,gpio_data.data_func);
	gpio_config(gpio_data.vsync,gpio_data.data_func);	
	gpio_config(gpio_data.pclk,gpio_data.data_func);	

	gpio_config(gpio_data.pwr_port, GPIO_OUTPUT);	
	gpio_config(gpio_data.pwd_port, GPIO_OUTPUT);
	gpio_config(gpio_data.rst_port, GPIO_OUTPUT);	

	gpio_set(gpio_data.pwr_port, 1);		
	gpio_set(gpio_data.pwd_port, 1);	
	gpio_set(gpio_data.rst_port, 1);	



}

void tcc_cif_module_initialize()
{

	/***** start image sensor power sequence *****/
	gpio_set(gpio_data.pwr_port, 1); //sensor_power_enable
	tcc_cif_delay(10);
	
	gpio_set(gpio_data.pwd_port, 1); //sensor_powerdown_disable
	tcc_cif_delay(40);
	
    gpio_set(gpio_data.rst_port, 0);  //sensor_reset_high
	tcc_cif_delay(10);

	gpio_set(gpio_data.rst_port, 1);  //sensor_reset_high
	tcc_cif_delay(15);
	

	/***** start image sensor initialize i2c command *****/
	write_regs(sensor_initialize);

}


