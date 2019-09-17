/*
 * Copyright (C) 2019 Wenting Zhang <zephray@outlook.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_gpio.h"
#include "pin_mux.h"

extern const unsigned char gImage_160144test2bpp[23046];
unsigned char framebuffer[23046];

#define STN_CPG_H() GPIO_PortSet(GPIO, BOARD_INITPINS_CORE1_STN_CPG_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_CPG_PORT)
#define STN_CPG_L() GPIO_PortClear(GPIO, BOARD_INITPINS_CORE1_STN_CPG_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_CPG_PORT)
#define STN_CPL_H() GPIO_PortSet(GPIO, BOARD_INITPINS_CORE1_STN_CPL_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_CPL_PORT)
#define STN_CPL_L() GPIO_PortClear(GPIO, BOARD_INITPINS_CORE1_STN_CPL_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_CPL_PORT)
#define STN_ST_H() GPIO_PortSet(GPIO, BOARD_INITPINS_CORE1_STN_ST_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_ST_PORT)
#define STN_ST_L() GPIO_PortClear(GPIO, BOARD_INITPINS_CORE1_STN_ST_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_ST_PORT)
#define STN_CP_H() GPIO_PortSet(GPIO, BOARD_INITPINS_CORE1_STN_CP_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_CP_PORT)
#define STN_CP_L() GPIO_PortClear(GPIO, BOARD_INITPINS_CORE1_STN_CP_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_CP_PORT)
#define STN_FR_H() GPIO_PortSet(GPIO, BOARD_INITPINS_CORE1_STN_FR_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_FR_PORT)
#define STN_FR_L() GPIO_PortClear(GPIO, BOARD_INITPINS_CORE1_STN_FR_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_FR_PORT)
#define STN_FR_T() GPIO_PortToggle(GPIO, BOARD_INITPINS_CORE1_STN_FR_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_FR_PORT)
#define STN_S_W(x) GPIO_PortToggle(GPIO, BOARD_INITPINS_CORE1_STN_S_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_S_PORT)
#define STN_D0_W(x) GPIO_PortToggle(GPIO, BOARD_INITPINS_CORE1_STN_D0_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_D0_PORT)
#define STN_D1_W(x) GPIO_PortToggle(GPIO, BOARD_INITPINS_CORE1_STN_D1_GPIO,\
		1u << BOARD_INITPINS_CORE1_STN_D1_PORT)

void STN_Wait(unsigned long t)
{
	int i;
	while(t--) {
		for (i=0;i<10;i++) {
			asm("nop");
		}
	}
}

static inline void STN_Frame() {
	unsigned x, y;
	// Scan 150 lines in total, 144 lines valid
	for (y = 0; y < 150; y++) {
		// Line start sequence
		STN_CPL_H();
		STN_CPG_H();
		STN_Wait(2);
		STN_CPL_L();
		STN_FR_T(); // Toggle AC line
		STN_Wait(1);
		STN_S_W((y == 0) ? 1 : 0); // Frame Sync
		STN_Wait(1);
		STN_CPG_L();
		STN_Wait(2);
		STN_CPG_H();
		STN_Wait(2);
		STN_CPG_L();

		// Before Line start
		STN_Wait(16);
		STN_ST_H();
		STN_Wait(2);
		STN_CP_H();
		STN_Wait(2);
		STN_ST_L();
		STN_Wait(1);

		// Shift out pixels
	}
}

void STN_Init() {
	// Actually, there is nothing much to initialize
	memcpy(framebuffer, gImage_160144test2bpp, 23046);
}
