# The WunderBar Firmware

### This repository contains the firmware source files for the WunderBar hardware.
Please check the readme files located in the sub-directories for instructions on compiling and flashing the firmware.

##Project structure:

* The **"/USB_MSD_Device_bootloader_v1.0"** directory contains the USB Mass Storage Device Bootloader for the Kinetis K24 MCU. This is useful to update the firmware over USB using the drag and drop method.
* **"/WunderBar_WiFi"** contains the main application for the K24 on the Master module. This MCU uses SPI to talk with the nRF51 (acting as a central device) and UART to talk with the Gainspan WiFi Module. This application contains the logic for Onboarding the WB, talking with the BLE modules trought the nRF51, holding the actual time using an NTP client, connecting with the relayr servers over SSL, subscribing to/publishing MQTT messages to the cloud, etc.
* **"/dfu bootloader"** contains the "over-the-air" bootloader for the NRF51852 MCU. Nordic provides applications and SDK examples for updating the firmware over BLE from Android and iOS.
* **"/wunderbar_BLE"** contains the main application for the nRF51. Inside is located the FW for the WunderBar Master Module and the 6 sensor nodes.
* **"/wunderbar_common"** holds common functions and macros necesary for both, the nRF51 and the K24 MCUs.
