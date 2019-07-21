/*******************************************************************************
* @brief    App for sending NMEA messages to serial port
* @author   Taras Zaporozhets <zaporozhets.taras@gmail.com>
* @date     July 22, 2019
*******************************************************************************/
#include "serial.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <errno.h>
#include <stdbool.h>
#include <string.h>

static speed_t serialBaudrateToBits(uint32_t baudrate)
{
    switch (baudrate) {
    case 50:
        return B50;
    case 75:
        return B75;
    case 110:
        return B110;
    case 134:
        return B134;
    case 150:
        return B150;
    case 200:
        return B200;
    case 300:
        return B300;
    case 600:
        return B600;
    case 1200:
        return B1200;
    case 1800:
        return B1800;
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 460800:
        return B460800;
    case 500000:
        return B500000;
    case 576000:
        return B576000;
    case 921600:
        return B921600;
    case 1000000:
        return B1000000;
    case 1152000:
        return B1152000;
    case 1500000:
        return B1500000;
    default:
        return B0;
    }
}

/*
 * @brief
 *
 * @param path
 * @param baudrate
 * @return int Return the file descriptor ,or -1.
 */
int serialOpen(const char* path, uint32_t baudrate)
{
    bool xonxoff = false;
    bool rtscts = false;

    struct termios termios_settings;

    // Open serial port
    int fd;
    if ((fd = open(path, O_RDWR | O_NOCTTY)) < 0) {
        return -1;
    }

    memset(&termios_settings, 0, sizeof(termios_settings));

    // c_iflag

    // Ignore break characters
    termios_settings.c_iflag = IGNBRK;
    if (xonxoff) {
        termios_settings.c_iflag |= (IXON | IXOFF);
    }
    // c_oflag
    termios_settings.c_oflag = 0;

    // c_lflag
    termios_settings.c_lflag = 0;

    // c_cflag
    // Enable receiver, ignore modem control lines
    termios_settings.c_cflag = CREAD | CLOCAL;

    // Databits
    termios_settings.c_cflag |= CS8;

    // RTS/CTS
    if (rtscts) {
        termios_settings.c_cflag |= CRTSCTS;
    }

    // Baudrate
    cfsetispeed(&termios_settings, serialBaudrateToBits(baudrate));
    cfsetospeed(&termios_settings, serialBaudrateToBits(baudrate));

    // Set termios attributes
    if (tcsetattr(fd, TCSANOW, &termios_settings) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

/*
 * @brief
 *
 * @param fd File descriptor to serial port
 * @param ptr Pointer to data
 * @param n data Length
 * @return ssize_t Return the number written, or -1.
 */
ssize_t serialWrite(int fd, const void* ptr, size_t n)
{
    ssize_t retval = 0;
    const uint8_t* buf = (const uint8_t*)ptr;

    // FIXME: Made for keeping SEGGER JLINK alive
    for (size_t i = 0; i < n; i++) {
        retval = write(fd, &buf[i], 1);
        if (0 > retval) {
            break;
        }
    }
    return retval;
}

/*
 * @brief Close the file descriptor FD.
 *
 * @param fd File descriptor to serial port
 * @return int
 */
int serialClose(int fd)
{
    return close(fd);
}
