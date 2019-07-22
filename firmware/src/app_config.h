/*******************************************************************************
* @brief    Position Beacon Transmitter Demo Application.
* @author   Taras Zaporozhets <zaporozhets.taras@gmail.com>
* @date     July 23, 2019
*******************************************************************************/
#pragma once


#define NRF_PRINTF_FLAG_AUTOMATIC_CR_ON_LF_ENABLED 0

#define NRF_LOG_BACKEND_UART_ENABLED 0

#define NRF_LOG_BACKEND_RTT_ENABLED 1

//#define NRF_LOG_BACKEND_RTT_ENABLE 0
#define NRF_LOG_ENABLED 1
#define NRF_LOG_DEFERRED 1
#define NRF_LOG_DEFAULT_LEVEL 4 //AKA Info


#define APP_UART_ENABLED 1
#define APP_FIFO_ENABLED 1
#define APP_UART_DRIVER_INSTANCE 0