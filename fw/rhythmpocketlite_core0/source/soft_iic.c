/*
 * Copyright (C) 2019 Wenting Zhang <zephray@outlook.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_gpio.h"
#include "pin_mux.h"

gpio_pin_config_t output_config = {
	kGPIO_DigitalOutput,
	0,
};

gpio_pin_config_t input_config = {
	kGPIO_DigitalInput,
	0,
};

#define IIC_SDA_H() GPIO_PortSet(GPIO, BOARD_INITPINS_CORE0_I2C_SDA_PORT,\
		1u << BOARD_INITPINS_CORE0_I2C_SDA_PIN)
#define IIC_SDA_L() GPIO_PortClear(GPIO, BOARD_INITPINS_CORE0_I2C_SDA_PORT,\
		1u << BOARD_INITPINS_CORE0_I2C_SDA_PIN)
#define IIC_SCL_H() GPIO_PortSet(GPIO, BOARD_INITPINS_CORE0_I2C_SCL_PORT,\
		1u << BOARD_INITPINS_CORE0_I2C_SCL_PIN)
#define IIC_SCL_L() GPIO_PortClear(GPIO, BOARD_INITPINS_CORE0_I2C_SCL_PORT,\
		1u << BOARD_INITPINS_CORE0_I2C_SCL_PIN)

#define IIC_SDA_Read() GPIO_PinRead(GPIO, BOARD_INITPINS_CORE0_I2C_SDA_PORT,\
		1u << BOARD_INITPINS_CORE0_I2C_SDA_PIN)

void IIC_WaitCycles(unsigned long t)
{
	int i;
	while(t--) {
		for (i=0;i<100;i++) {
			asm("nop");
		}
	}
}

void IIC_SDA_Out(void)
{
	GPIO_PinInit(GPIO, BOARD_INITPINS_CORE0_I2C_SDA_PORT,
			BOARD_INITPINS_CORE0_I2C_SDA_PIN, &output_config);
}

void IIC_SDA_In(void)
{
	GPIO_PinInit(GPIO, BOARD_INITPINS_CORE0_I2C_SDA_PORT,
			BOARD_INITPINS_CORE0_I2C_SDA_PIN, &input_config);
}

// Generate IIC start signal
void IIC_Start(void)
{
	IIC_SDA_Out(); // Set SDA as output
	IIC_SDA_H();
	IIC_WaitCycles(1);
	IIC_SCL_H();
	IIC_WaitCycles(5);
 	IIC_SDA_L(); // START: when CLK is high, DATA change form high to low
	IIC_WaitCycles(5);
	IIC_SCL_L(); // Take control of the bus
	IIC_WaitCycles(2);
}

// Generate IIC stop signal
void IIC_Stop(void)
{
	IIC_SDA_Out(); // Set SDA as output
	IIC_SCL_L();
	IIC_SDA_L(); // STOP: when CLK is high DATA change form low to high
 	IIC_WaitCycles(4);
	IIC_SCL_H();
	IIC_WaitCycles(5);
	IIC_SDA_H(); // Send I2C bus stop signal
	IIC_WaitCycles(4);
}

// Wait acknowledge signal
// Return value: 1 - Failed, 0 -s Succeed
uint8_t IIC_Wait_Ack(void)
{
	uint16_t ucErrTime=0;
	IIC_SDA_In();
	IIC_SDA_H(); IIC_WaitCycles(1);
	IIC_SCL_H(); IIC_WaitCycles(1);
	while(IIC_SDA_Read())
	{
		ucErrTime++;
		if(ucErrTime>500)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL_L();
	return 0;
}

void IIC_SAck(void)
{
	IIC_SCL_L();
	IIC_SDA_Out();
	IIC_SDA_L();
	IIC_WaitCycles(2);
	IIC_SCL_H();
	IIC_WaitCycles(2);
	IIC_SCL_L();
}

void IIC_NAck(void)
{
	IIC_SCL_L();
	IIC_SDA_Out();
	IIC_SDA_H();
	IIC_WaitCycles(2);
	IIC_SCL_H();
	IIC_WaitCycles(2);
	IIC_SCL_L();
}

// IIC Send byte
// Result: 0 - NACK, 1 - ACK
uint8_t IIC_Write_Byte(uint8_t txd)
{
    uint8_t t;

    IIC_SDA_Out();
    IIC_SCL_L();//拉低时钟开始数据传输
    for(t = 0; t < 8; t++)
    {
    	if(txd & 0x80)
    		IIC_SDA_H();
    	else
    		IIC_SDA_L();
		txd <<= 1;
		IIC_WaitCycles(2);
		IIC_SCL_H();
		IIC_WaitCycles(2);
		IIC_SCL_L();
		IIC_WaitCycles(2);
    }

    t = IIC_Wait_Ack();

    IIC_WaitCycles(5);

    return t;
}

// Read one byte
uint8_t IIC_Read_Byte()
{
	unsigned char i,receive=0;
	IIC_SDA_In();
    for(i=0;i<8;i++ )
	{
        IIC_SCL_L();
        IIC_WaitCycles(2);
		IIC_SCL_H();
        receive<<=1;
        if(IIC_SDA_Read())
        	receive++;
		IIC_WaitCycles(1);
    }
    return receive;
}

void IIC_Ack(unsigned char ack)
{
	if (!ack)
        IIC_NAck();
    else
        IIC_SAck();
}
