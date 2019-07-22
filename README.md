# Task

Make BLE beacon to transmit GPS coordinates:
- GPS coordinates should be given to the beacon via command from PC
- Bluetooth beacon should have a distinctive Bluetooth device name
- GPS coordinates should be transmitted as advertising data


# Solution

Solution consists of firmware for NRF52 and two PC applications (nmeaSender and bleReceiver).

## Firmware
It receives NMEA GPRMC messages via UART and parses them. After parsing, firmware updates advertising data with new latitude and longitude.

#### UART settings and data format
Port settings: 115200, 8bit, no parity, no flow.
NMEA string example:
```
$GPRMC,094236.013,A,5926.669,N,02444.875,E,060.4,203.3,210719,000.0,W*73
$GPRMC,094240.013,A,5926.547,N,02444.962,E,083.4,227.0,210719,000.0,W*72
```
```<CR><LF>``` ends the message.

#### BLE advertising packet format
```02 01 04```= First element with flags

 ```0B``` = length of "MANUFACTURER_SPECIFIC_DATA"

```FF``` = indicates "MANUFACTURER_SPECIFIC_DATA"

```FF FF``` = Company identifier

```00 00 00 00 00 00 00 00``` = 4 bytes of latitude and 4bytes of longitude

```08 09 74 72 61 63 6B 65 72 D2``` = Device short name "tracker"


## nmeaSender
App reads line by line NMEA data from file and sends to serial port with period 1 sec.
~~~sh
./nmeaSender/nmeaSender /dev/ttyACM0 nmeaSender/sample.nmea
~~~
Tool for generating GPS logs in NMEA format: [NMEA Generator](https://nmeagen.org/)

## bleReceiver
App receives BLE advertising packets, parse and draw points on the map. It uses Bluez HCI so it requires root privileges to run. 
~~~sh
sudo ./bleReceiver/bleReceiver 
~~~
![Screenshot](https://github.com/zaporozhets/gps-beacon/raw/master/bleReceiver.png)



## Required software
1. Cross-platform IDE for embedded systems: [Segger Embedded Studio](
https://www.segger.com/downloads/embedded-studio). After installation open and add ```nRF CPU Support Package``` via Tools->Package Manager
	
2. All-in-one debugging solution: [J-Link / J-Trace Downloads](
https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack) 

3. Software development kit for the nRF52 SoC: [nRF5 SDK ver 15.3059 ac345](
https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/nRF5SDK153059ac345.zip). Download and extract content to ```firmware/nRF5_SDK``` folder

4. Libraries to compile PC utils:
```
sudo apt install libbluetooth-dev 
```
And [Qt 5.12](https://www.qt.io/download) framework


## How to build

1. nmeaSender

~~~sh
cd nmeaSender
qmake
make
~~~


2. firmware
Open ```firmware/beacon.emProject``` in Segger Embedded Studio, build and flash.


3. bleReceiver
~~~sh
cd bleReceiver
qmake
make
~~~
