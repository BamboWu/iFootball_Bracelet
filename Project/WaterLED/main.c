/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf.h"
#include "boards.h"

const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;

#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1                           /**< UART RX buffer size. */

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    // Configure LED-pins as outputs.
    LEDS_CONFIGURE(LEDS_MASK);

    uint32_t err_code;
    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_ENABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud115200
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

    APP_ERROR_CHECK(err_code);

    printf("\n\rRunning!\n\r");

    // Toggle LEDs.
    while (true)
    {
	static int i = 0;
	uint8_t cr;

        while(app_uart_get(&cr) != NRF_SUCCESS);
	if(cr == ' ')
	{
            LEDS_INVERT(1 << leds_list[i]);
	    i++;
	}
	else if(cr == 'q' || cr == 'Q')
	{
	    LEDS_OFF(LEDS_MASK);
	    i = 0;
	}
	else
	    printf("\n\rChar:%c\n\r",cr);

	if (i == LEDS_NUMBER) i = 0;
        
	nrf_delay_ms(500);
        
    }
//        while(app_uart_put(cr) != NRF_SUCCESS);

}


/** @} */
