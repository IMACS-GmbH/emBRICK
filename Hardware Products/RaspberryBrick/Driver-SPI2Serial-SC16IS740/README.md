# # Driver-SPI2Serial-SC16IS740# by IMACS GmbH

This script installs and configures Linux **Socket CAN**, **Serial port RS232/RS485** and **RTC** drivers

## :large_orange_diamond: Installation Instructions

**:heavy_exclamation_mark: _create a backup copy of your µSD card before applying these steps!_**

**Step 1:**

Install one of the listed RASPBIAN operating system versions from below: 

**Tested by Raspberry Pi OS Kernel Version 5.10  Release date: March 04th 2021

unter https://www.raspberrypi.org/software/operating-systems/ you can find a the current version of the PI OS.

   _install.sh_ script uses the mainline kernel driver sources with only a few source code patches, see _install.sh_ for more details. Our performance optimizations of the CAN, UART and SPI drivers are currently not included in these mainline drivers.   


**Step 2:**

```
sudo bash
cd /tmp
wget https://raw.githubusercontent.com/IMACS-GmbH/emBRICK-and-brickBUS/main/Hardware%20Products/RaspberryBrick/Driver-SPI2Serial-SC16IS740/install.sh -O install.sh
bash install.sh
```
<br />
<br />
<br />




