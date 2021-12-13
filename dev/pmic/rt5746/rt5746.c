/***************************************************************************************
*   FileName    : rt5746.c
*   Description : Discrete Power IC driver
****************************************************************************************
*
*   TCC Board Support Package
*   Copyright (c) Telechips, Inc.
*   ALL RIGHTS RESERVED
*
****************************************************************************************/

#include <platform/reg_physical.h>
#include <i2c.h>
#include <debug.h>
#include <dev/rt5746.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

/* device address */
#define SLAVE_ADDR_RT5746   0x38

static int rt5746_i2c_ch0 = 0;
static int rt5746_initialized = 0;

/********************************************************************
    I2C Command & Values
********************************************************************/
#define RT5746_INT_MASK  0x02
#define RT5746_PRODUCT_ID   0x03
#define RT5746_REVISION_ID  0x04
#define RT5746_FEATURE_ID   0x05
#define RT5746_VENDER_ID   0x06
#define RT5746_VOLTAGE_PROGRAM_0  0x10
#define RT5746_VOLTAGE_PROGRAM_1  0x11
#define RT5746_PGOOD    0x12
#define RT5746_TIME    0x12
#define RT5746_COMMAND  0x14
#define RT5746_LIMCONF  0x16

static unsigned char rt5746_read(unsigned char cmd, int i2c_ch)
{
    unsigned char recv_data;

    i2c_xfer(SLAVE_ADDR_RT5746, 1, &cmd, 1, &recv_data, i2c_ch);
    return recv_data;
}

static void rt5746_write(unsigned char cmd, unsigned char value, int i2c_ch)
{
    unsigned char send_data[2];
    send_data[0] = cmd;
    send_data[1] = value;

    i2c_xfer(SLAVE_ADDR_RT5746, 2, send_data, 0, 0, i2c_ch);
}

void rt5746_init(void)
{
    unsigned char value_pid, value_rid, value_fid;

    rt5746_i2c_ch0 = I2C_CH_MASTER0;
    rt5746_initialized = 1;

    value_pid = rt5746_read(RT5746_PRODUCT_ID, rt5746_i2c_ch0);
    value_rid = rt5746_read(RT5746_REVISION_ID, rt5746_i2c_ch0);
    value_fid = rt5746_read(RT5746_FEATURE_ID,  rt5746_i2c_ch0);
    dprintf(INFO, "%s: VB id:0x%x 0x%x 0x%x\n", __func__, value_pid, value_rid, value_fid);
}

int rt5746_output_control(unsigned int onoff, int i2c_ch)
{
    unsigned char reg, old_value, value;

    reg = RT5746_VOLTAGE_PROGRAM_0;

    old_value = rt5746_read(reg, i2c_ch);
    if (onoff)
        value = (old_value | (1<<7));
    else
        value = (old_value & ~(1<<7));

    //dprintf(INFO, "%s: id:%d, onoff:%d, reg:0x%x, value:0x%x, master:%d\n", __func__, onoff, reg, value, i2c_ch);
    rt5746_write(reg, value, i2c_ch);

    return 0;
}

static int rt5746_set_voltage_ch(unsigned int mV, int i2c_ch)
{
    unsigned char reg;
    unsigned  int value = 0;

    reg = RT5746_VOLTAGE_PROGRAM_0;

    if (mV < 600){
        dprintf(INFO,"Wrong dcdc value!\n");
        return -1;
    }

    value = ((mV - 600)<<4) / 100;

    dprintf(INFO, "%s: mV:%d, avalue:0x%x, master:%d\n", __func__,mV, value, i2c_ch);

    value |= 0x80;
    
    rt5746_write(reg, value, i2c_ch);
    rt5746_output_control(1, i2c_ch);

    return 0;
}

int rt5746_set_voltage(int type, unsigned int mV)
{
    int i2c_ch;
    i2c_ch = rt5746_i2c_ch0;

    if (rt5746_initialized == 0)
        return -1;

    // type for other platform that use only discrete
    type += 1; // temporary

    /* power off */
    if (mV == 0)
        rt5746_output_control(0, i2c_ch);
    else 
        rt5746_set_voltage_ch(mV, i2c_ch);

    return 0;
}

int rt5746_set_power(int type, int onoff)
{
    int i2c_ch;
    i2c_ch = rt5746_i2c_ch0;
	
    if (rt5746_initialized == 0)
        return -1;

    // type for other platform that use only discrete
    type += 1; // temporary

    if (onoff)
        rt5746_output_control(1, i2c_ch);
    else
        rt5746_output_control(0, i2c_ch);

    return 0;
}




