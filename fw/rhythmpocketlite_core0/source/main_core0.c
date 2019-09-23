/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_mailbox.h"

#include "fsl_common.h"
#include "pin_mux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PRIMARY_CORE_MAILBOX_CPU_ID kMAILBOX_CM33_Core0
#define SECONDARY_CORE_MAILBOX_CPU_ID kMAILBOX_CM33_Core1

/* Address of RAM, where the image for core1 should be copied */
#define CORE1_BOOT_ADDRESS 0x20033000

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif defined(__GNUC__)
extern const char m0_image_start[];
extern const char *m0_image_end;
extern int m0_image_size;
#define CORE1_IMAGE_START ((void *)m0_image_start)
#define CORE1_IMAGE_SIZE ((void *)m0_image_size)
#endif
#define START_EVENT 1234
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif

void start_secondary_core(uint32_t sec_core_boot_addr);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_msg = 1;
/* For the flow control */
volatile bool g_secondary_core_started = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void)
{
    uint32_t core1_image_size;
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    core1_image_size = (uint32_t)&Image$$CORE1_REGION$$Length;
#elif defined(__ICCARM__)
#pragma section = "__sec_core"
    core1_image_size = (uint32_t)__section_end("__sec_core") - (uint32_t)&core1_image_start;
#elif defined(__GNUC__)
    core1_image_size = (uint32_t)m0_image_size;
#endif
    return core1_image_size;
}
#endif

void start_secondary_core(uint32_t sec_core_boot_addr)
{
    /* Boot source for Core 1 from flash */
    SYSCON->CPUCFG |= SYSCON_CPUCFG_CPU1ENABLE_MASK;
    SYSCON->CPBOOT = SYSCON_CPBOOT_CPBOOT(sec_core_boot_addr);

    int32_t temp = SYSCON->CPUCTRL;
    temp |= 0xc0c48000;
    SYSCON->CPUCTRL = temp | SYSCON_CPUCTRL_CPU1RSTEN_MASK | SYSCON_CPUCTRL_CPU1CLKEN_MASK;
    SYSCON->CPUCTRL = (temp | SYSCON_CPUCTRL_CPU1CLKEN_MASK) & (~SYSCON_CPUCTRL_CPU1RSTEN_MASK);
}

/* When the secondary core writes to the primary core mailbox register it causes call of this irq handler,
   in which the received value is read, incremented and sent again to the secondary core */
void MAILBOX_IRQHandler()
{
    if (!g_secondary_core_started)
    {
        if (START_EVENT == MAILBOX_GetValue(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID))
        {
            g_secondary_core_started = true;
        }
        MAILBOX_ClearValueBits(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID, 0xffffffff);
    }
    else
    {
        g_msg = MAILBOX_GetValue(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID);
        MAILBOX_ClearValueBits(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID, 0xffffffff);
        PRINTF("Read value from the primary core mailbox register: %d\r\n", g_msg);
        g_msg++;
        PRINTF("Write to the secondary core mailbox register: %d\r\n", g_msg);
        MAILBOX_SetValue(MAILBOX, SECONDARY_CORE_MAILBOX_CPU_ID, g_msg);
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware.*/
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    // Set MCLK to input mode and attach MCLK to FC7
	SYSCON->MCLKIO = 0U;
	CLOCK_AttachClk(kMCLK_to_FLEXCOMM7);

	// Reset DMA and FLEXCOMM for I2S
	RESET_PeripheralReset(kDMA0_RST_SHIFT_RSTn);
	RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);

	// Reset NVIC for FLEXCOMM 7
	NVIC_ClearPendingIRQ(FLEXCOMM7_IRQn);

	// Enable interrupts for I2S
	EnableIRQ(FLEXCOMM7_IRQn);

	// Initialize the rest
    BOARD_InitPins_Core0();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();

    PRINTF("Mailbox interrupt example\r\n");

    /* Init Mailbox */
    MAILBOX_Init(MAILBOX);

    /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);

#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image */
    uint32_t core1_image_size;
    core1_image_size = get_core1_image_size();
    PRINTF("Copy CORE1 image to address: 0x%x, size: %d\r\n", CORE1_BOOT_ADDRESS, core1_image_size);

    /* Copy application from FLASH to RAM */
    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

    /* Start the secondary core */
    start_secondary_core(CORE1_BOOT_ADDRESS);

    /* Wait for start and initialization of secondary core */
    while (!g_secondary_core_started)
        ;

    //PRINTF("Write to the secondary core mailbox register: %d\r\n", g_msg);
    /* Write g_msg to the secondary core mailbox register - it causes interrupt on the secondary core */
    //MAILBOX_SetValue(MAILBOX, SECONDARY_CORE_MAILBOX_CPU_ID, g_msg);

    // Enable LCD power
    GPIO_PortSet(GPIO, BOARD_INITPINS_CORE0_LCD_PWR_EN_PORT,
    		1u << BOARD_INITPINS_CORE0_LCD_PWR_EN_PIN);

    // Configure the codec
    PRINTF("Configure CS43131 codec\r\n");

    while (1)
    {
        __WFI();
    }
}
