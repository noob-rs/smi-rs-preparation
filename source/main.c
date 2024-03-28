/******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for TCP Client Example in ModusToolbox.
 * The example establishes a connection with a remote TCP server and based on
 * the command received from the TCP server, turns the user LED ON or OFF.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * Copyright 2019-2023, Cypress Semiconductor Corporation (an Infineon company)
 *or an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *******************************************************************************/

/* Header file includes. */
#include "cy_log.h"
#include "cy_retarget_io.h"
#include "cybsp.h"
#include "cyhal.h"

/* RTOS header file. */
#if defined(COMPONENT_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>

#elif defined(COMPONENT_THREADX)
#include "tx_api.h"
#include "tx_initialize.h"
#endif

/* Include serial flash library and QSPI memory configurations only for the
 * kits that require the Wi-Fi firmware to be loaded in external QSPI NOR flash.
 */
#if defined(CY_DEVICE_PSOC6A512K)
#include "cy_serial_flash_qspi.h"
#include "cycfg_qspi_memslot.h"
#endif

/*******************************************************************************
 * Macros
 ********************************************************************************/
/* RTOS related macros. */
#if defined(COMPONENT_FREERTOS)
#define TCP_CLIENT_TASK_STACK_SIZE (10 * 1024)
#define TCP_CLIENT_TASK_PRIORITY (1)
#endif

/*******************************************************************************
 * Global Variables
 ********************************************************************************/
/* This enables RTOS aware debugging. */
volatile int uxTopUsedPriority;

int app_log_output_callback(CY_LOG_FACILITY_T facility, CY_LOG_LEVEL_T level,
                            char *logmsg) {
  (void)facility; // Can be used to decide to reduce output or send output to
                  // remote logging
  (void)level;    // Can be used to decide to reduce output, although the output
                  // has already been limited by the log routines

  return printf("%s\n", logmsg); // print directly to console
}

cy_rslt_t app_log_time(uint32_t *time) {
  if (time != NULL) {
    *time = 0;
  }
  return CY_RSLT_SUCCESS;
}

/********************************************************************************
 * Function Name: main
 ********************************************************************************
 * Summary:
 *  System entrance point. This function sets up user tasks and then starts
 *  the RTOS scheduler.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 *******************************************************************************/
int main() {
  cy_rslt_t result;

#if defined(COMPONENT_FREERTOS)
  /* This enables RTOS aware debugging in OpenOCD. */
  uxTopUsedPriority = configMAX_PRIORITIES - 1;
#elif defined(COMPONENT_THREADX)
  uxTopUsedPriority = TX_MAX_PRIORITIES - 1;
#endif

  /* Initialize the board support package. */
  result = cybsp_init();
  CY_ASSERT(result == CY_RSLT_SUCCESS);

  /* To avoid compiler warnings. */
  (void)result;

  /* Enable global interrupts. */
  __enable_irq();

  /* Initialize retarget-io to use the debug UART port. */
  cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                      CY_RETARGET_IO_BAUDRATE);

  /* Initialize the User LED. */
  cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
                  CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
  // cy_log_init(CY_LOG_DEBUG, app_log_output_callback, app_log_time);

#if defined(CY_DEVICE_PSOC6A512K)
  const uint32_t bus_frequency = 50000000lu;
  cy_serial_flash_qspi_init(smifMemConfigs[0], CYBSP_QSPI_D0, CYBSP_QSPI_D1,
                            CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC, NC, NC,
                            CYBSP_QSPI_SCK, CYBSP_QSPI_SS, bus_frequency);

  /* Enable the XIP mode to get the Wi-Fi firmware from the external flash.
   */
  cy_serial_flash_qspi_enable_xip(true);
#endif

  /* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen */
  printf("\x1b[2J\x1b[;H");
  printf("============================================================\n");
  printf("CE229112 - Connectivity Example: TCP Client\n");
  printf("============================================================\n\n");

  void smi_main(void *arg);

#if defined(COMPONENT_FREERTOS)
  /* Create the tasks. */
  xTaskCreate(smi_main, "SMI task", TCP_CLIENT_TASK_STACK_SIZE, NULL,
              TCP_CLIENT_TASK_PRIORITY, NULL);

  /* Start the FreeRTOS scheduler. */
  vTaskStartScheduler();

  /* Should never get here. */
  CY_ASSERT(0);

#elif defined(COMPONENT_THREADX)
  /*
   * Start the ThreadX kernel.
   * This routine never returns.
   */

  tx_kernel_enter();

  /* Should never get here. */
  CY_ASSERT(0);
#endif
}

#if defined(COMPONENT_THREADX)
void application_start(void) { tcp_client_task(NULL); }
#endif

/* [] END OF FILE */
