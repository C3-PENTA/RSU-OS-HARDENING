/***************************************************************************************
*   FileName    : rt8088.c
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
#include <dev/rt8088.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

/* device address */
#define SLAVE_ADDR_RT8088A   0x38
#define SLAVE_ADDR_RT8088C 0x28
#define SLAVE_ADDR_RT8088C_R 0x29

static int rt8088_i2c_ch0 = 0;
static int rt8088_i2c_ch1 = 0;
static int rt8088_initialized = 0;

/********************************************************************
    I2C Command & Values
********************************************************************/
#define RT8088_MONITOR  0x01
#define RT8088_PRODUCT_ID   0x03
#define RT8088_REVISION_ID  0x04
#define RT8088_FEATURE_ID   0x05
#define RT8088_VOLTAGE_PROGRAM  0x11
#define RT8088_DISCHARGE    0x12
#define RT8088_COMMAND  0x14
#define RT8088_LIMCONF  0x16

static unsigned char rt8088_read(unsigned char cmd, unsigned char type, int i2c_ch)
{
    unsigned char recv_data;
    switch (type) {
        case 0:
        i2c_xfer(SLAVE_ADDR_RT8088A, 1, &cmd, 1, &recv_data, i2c_ch);
        break;
        case 1:
        i2c_xfer(SLAVE_ADDR_RT8088C, 1, &cmd, 1, &recv_data, i2c_ch);
        break;
        default:
        return 0;
    }
    return recv_data;
}

static void rt8088_write(unsigned char cmd, unsigned char value, unsigned char type, int i2c_ch)
{
    unsigned char send_data[2];
    send_data[0] = cmd;
    send_data[1] = value;
    switch (type) {
        case 0:
        i2c_xfer(SLAVE_ADDR_RT8088A, 2, send_data, 0, 0, i2c_ch);
        break;
        case 1:
        i2c_xfer(SLAVE_ADDR_RT8088C, 2, send_data, 0, 0, i2c_ch);
        break;
    }
}

void rt8088_init(void)
{
    unsigned char value_pid, value_rid, value_fid;

    rt8088_i2c_ch0 = I2C_CH_MASTER0;
    rt8088_i2c_ch1 = I2C_CH_MASTER2;
    
    rt8088_initialized = 1;

    value_pid = rt8088_read(RT8088_PRODUCT_ID, 0, rt8088_i2c_ch0);
    value_rid = rt8088_read(RT8088_REVISION_ID, 0, rt8088_i2c_ch0);
    value_fid = rt8088_read(RT8088_FEATURE_ID, 0, rt8088_i2c_ch0);
    dprintf(INFO, "%s: core id:0x%x 0x%x 0x%x\n", __func__, value_pid, value_rid, value_fid);

    value_pid = rt8088_read(RT8088_PRODUCT_ID, 0, rt8088_i2c_ch1);
    value_rid = rt8088_read(RT8088_REVISION_ID, 0, rt8088_i2c_ch1);
    value_fid = rt8088_read(RT8088_FEATURE_ID, 0, rt8088_i2c_ch1);
    dprintf(INFO, "%s:   gv id:0x%x 0x%x 0x%x\n", __func__, value_pid, value_rid, value_fid);

    value_pid = rt8088_read(RT8088_PRODUCT_ID, 1, rt8088_i2c_ch0);
    value_rid = rt8088_read(RT8088_REVISION_ID, 1, rt8088_i2c_ch0);
    value_fid = rt8088_read(RT8088_FEATURE_ID, 1, rt8088_i2c_ch0);
    dprintf(INFO, "%s: cpu0 id:0x%x 0x%x 0x%x\n", __func__, value_pid, value_rid, value_fid);

    value_pid = rt8088_read(RT8088_PRODUCT_ID, 1, rt8088_i2c_ch1);
    value_rid = rt8088_read(RT8088_REVISION_ID, 1, rt8088_i2c_ch1);
    value_fid = rt8088_read(RT8088_FEATURE_ID, 1, rt8088_i2c_ch1);
    dprintf(INFO, "%s: cpu1 id:0x%x 0x%x 0x%x\n", __func__, value_pid, value_rid, value_fid);
}

int rt8088_output_control(rt8088_src_id id, unsigned int onoff, int i2c_ch)
{
    unsigned char reg, old_value, value;

    reg = RT8088_VOLTAGE_PROGRAM;

    old_value = rt8088_read(reg, id, i2c_ch);
    if (onoff)
        value = (old_value | (1<<7));
    else
        value = (old_value & ~(1<<7));

    //dprintf(INFO, "%s: id:%d, onoff:%d, reg:0x%x, value:0x%x, master:%d\n", __func__, id, onoff, reg, value, i2c_ch);
    rt8088_write(reg, value, id, i2c_ch);

    return 0;
}

static int rt8088a_set_voltage(rt8088_src_id id, unsigned int mV, int i2c_ch)
{
    unsigned char reg;
    unsigned  int value = 0;

    reg = RT8088_VOLTAGE_PROGRAM;

    if (mV < 600){
        dprintf(INFO,"Wrong dcdc%d value!\n", id);
        return -1;
    }

    value = ((mV - 600)<<4) / 100;

    dprintf(INFO, "%s: id:%d, mV:%d, avalue:0x%x, master:%d\n", __func__, id, mV, value, i2c_ch);

    value |= 0x80;
    
    rt8088_write(reg, value, id, i2c_ch);
    rt8088_output_control(id, 1, i2c_ch);

    return 0;
}

static int rt8088c_set_voltage(rt8088_src_id id, unsigned int mV, int i2c_ch)
{
    unsigned char reg;
    unsigned int value = 0;

    reg = RT8088_VOLTAGE_PROGRAM;

    if (mV < 600){
        dprintf(INFO,"Wrong dcdc%d value!\n", id);
        return -1;
    }

    value = ((mV - 600)<<4) / 100;

    dprintf(INFO, "%s: id:%d, mV:%d, cvalue:0x%x, master:%d\n", __func__, id, mV, value, i2c_ch);

    value |= 0x80;
    rt8088_write(reg, value, id, i2c_ch);
    rt8088_output_control(id, 1, i2c_ch);
    
    return 0;
}

int rt8088_set_voltage(int type, unsigned int mV)
{
    rt8088_src_id id;
    int i2c_ch;

    if (rt8088_initialized == 0)
        return -1;

    switch(type)
    {
        case PWR_CPU0:
        id = RT8088C;
        i2c_ch = rt8088_i2c_ch0;
        break;
        case PWR_CPU1:
        id = RT8088C;
        i2c_ch = rt8088_i2c_ch1;
        break;
        case PWR_CORE:
        id = RT8088A;
        i2c_ch = rt8088_i2c_ch0;
        break;
        case PWR_GV:
        id = RT8088A;
        i2c_ch = rt8088_i2c_ch1;
        break;
        default:
        return -1;
        break;
    }

    /* power off */
    if (mV == 0)
        rt8088_output_control(id, 0, i2c_ch);
    else {
        switch (id){
            case RT8088C:
            rt8088c_set_voltage(id, mV, i2c_ch);
            break;
            case RT8088A:
            rt8088a_set_voltage(id, mV, i2c_ch);
            break;
            default:
            return -1;
        }
    }

    return 0;
}

int rt8088_set_power(int type, int onoff)
{
    rt8088_src_id id;
    int i2c_ch;

    if (rt8088_initialized == 0)
        return -1;

    switch(type)
    {
        case PWR_CPU0:
        id = RT8088C;
        i2c_ch = rt8088_i2c_ch0;
        break;
        case PWR_CPU1:
        id = RT8088C;
        i2c_ch = rt8088_i2c_ch1;
        break;
        case PWR_CORE:
        id = RT8088A;
        i2c_ch = rt8088_i2c_ch0;
        break;
        case PWR_GV:
        id = RT8088A;
        i2c_ch = rt8088_i2c_ch1;
        break;
        default:
        return -1;
        break;
    }

    if (onoff)
        rt8088_output_control(id, 1, i2c_ch);
    else
        rt8088_output_control(id, 0, i2c_ch);

    return 0;
}




