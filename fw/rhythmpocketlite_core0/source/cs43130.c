/*
 * Copyright (C) 2019 Wenting Zhang <zephray@outlook.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
uint8_t CS43130_Read_Register(uint32_t register) {

}

void CS43130_Write_Register(uint32_t register) {

}

void CS43130_PCM_Power_Up() {
    // Pop-free power-up settings
    CS43130_Write_Register(0x10010, 0x99);
    CS43130_Write_Register(0x80032, 0x20);
    // Power up ASP
    CS43130_Write_Register(0x20000, CS43130_Read_Register(0x20000), 0xBF);
    // Power up HP
    CS43130_Write_Register(0x20000, CS43130_Read_Register(0x20000), 0xEF);
    // Wait 12 ms
    Delay(12);
    // Restore default settings
    CS43130_Write_Register(0x80032, 0x00);
    CS43130_Write_Register(0x10010, 0x00);
}

void CS43130_Init() {
    // Release reset
    CS43130_RESET_H();
    Delay(2);

    // Configure XTAL driver
    CS43130_Write_Register(0x20052, 0x04);
    // Clear pending interrupts
    CS43130_Read_Register(0xf0000);
    // Enable XTAL interrupts
    CS43130_Write_Register(0xf0010, 0xe7);
    // Start XTAL
    CS43130_Write_Register(0x20000, 0xf6);

    // Set ASP sample rate to 44.1kHz
    CS43130_Write_Register(0x1000b, 0x01);
    // Set ASP sample bit size to 24bits
    CS43130_Write_Register(0x1000c, 0x05);
    // Set ASP numerator
    CS43130_Write_Register(0x40010, 0x01);
    CS43130_Write_Register(0x40011, 0x00);
    // Set ASP denominator
    CS43130_Write_Register(0x40012, 0x08);
    CS43130_Write_Register(0x40013, 0x00);
    // Set ASP LRCK high time
    CS43130_Write_Register(0x40014, 0x1f);
    CS43130_Write_Register(0x40015, 0x00);
    // Set ASP LRCK period
    CS43130_Write_Register(0x40016, 0x3f);
    CS43130_Write_Register(0x40017, 0x00);
    // Set ASP to master, configure clock polarity
    CS43130_Write_Register(0x40018, 0x1c);
    // Configure ASP frame
    CS43130_Write_Register(0x40019, 0x0a);
    // Set ASP channel location
    CS43130_Write_Register(0x50000, 0x00);
    CS43130_Write_Register(0x50001, 0x00);
    // Set ASP channel size and enable
    CS43130_Write_Register(0x5000a, 0x07);
    CS43130_Write_Register(0x5000b, 0x0f);

    // Configure PCM filter
    CS43130_Write_Register(0x90000, 0x02);
    // Set volume for channel B
    CS43130_Write_Register(0x90001, 0x00);
    // Set volume for channel A
    CS43130_Write_Register(0x90002, 0x00);
    // Configure PCM path signal control
    CS43130_Write_Register(0x90003, 0xec);
    CS43130_Write_Register(0x90004, 0x00);

    // Configure class H amplifier
    CS43130_Write_Register(0xb0000, 0x1e);
    // Set HP output to full scale
    CS43130_Write_Register(0x80000, 0x30);
    // Disable headphone detect
    CS43130_Write_Register(0xd0000, 0x04);

    // Enable ASP interrupts
    CS43130_Write_Register(0xf0011, 0x07);

    // Wait for XTAL to be ready
    while (!(CS43130_Read_Register(0xf0000) & 0x10));

    // Switch MCLK source to XTAL
    CS43130_Write_Register(0x10006, 0x04);

    // Wait at least 150us
    Delay(1);

    // Eanble ASP clocks
    CS43130_Write_Register(0x1000d, 0x02);

    // Power up HP
    CS43130_PCM_Power_Up();
}
