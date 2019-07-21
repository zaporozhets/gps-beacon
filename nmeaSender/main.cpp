/*******************************************************************************
* @brief    App for sending NMEA messages to serial port
* @author   Taras Zaporozhets <zaporozhets.taras@gmail.com>
* @date     July 22, 2019
*******************************************************************************/
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
extern "C" {
#include "serial.h"
}

int main(int argc, char** argv)
{
    auto serial = argv[1];
    auto nmea = argv[2];

    if (argc != 3) {
        std::cerr << "Example: TODO" << std::endl;
    }

    // Serial port initialization
    auto fd = serialOpen(serial, 115200);
    if (fd < 0) {
        std::cerr << "Fail to open serial port: " << argv[1] << std::endl;
        return -1;
    }

    std::ifstream input(nmea);

    for (std::string line; getline(input, line);) {
        std::cout << "String: " << line.c_str() << std::endl;

        // Write NMEA message
        auto retval = serialWrite(fd, line.c_str(), line.length());
        if (0 > retval) {
            std::cerr << "Fail to write data" << std::endl;
            return -1;
        }

        // Write NMEA EOL sequence
        char lineEnd[] = { '\r', '\n' };
        retval = serialWrite(fd, lineEnd, sizeof(lineEnd));
        if (0 > retval) {
            std::cerr << "Fail to write data" << std::endl;
            return -1;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "End of file" << std::endl;

    serialClose(fd);

    return 0;
}
