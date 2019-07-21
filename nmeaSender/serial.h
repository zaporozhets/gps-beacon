/*******************************************************************************
* @brief    App for sending NMEA messages to serial port
* @author   Taras Zaporozhets <zaporozhets.taras@gmail.com>
* @date     July 22, 2019
*******************************************************************************/
#pragma once

#include <stdint.h>
#include <sys/types.h>

int serialOpen(const char* path, uint32_t baudrate);
ssize_t serialWrite(int fd, const void* ptr, size_t n);
int serialClose(int fd);
