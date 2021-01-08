# # CAE_Z-RaspberryBrick-1#-RB_0# by IMACS GmbH

This script installs and configures Linux **Socket CAN**, **Serial port RS232/RS485** and **RTC** drivers

## :large_orange_diamond: Installation Instructions

**:heavy_exclamation_mark:  recommended for new designs!**

_create a backup copy of your µSD card before applying these steps!_

**Step 1:**

Install one of the listed RASPBIAN operating system versions from below: 

1) **Raspbian Stretch with desktop version 2018-11-13 or later**

   _install.sh_ script uses the mainline kernel driver sources with only a few source code patches, see _install.sh_ for more details. Our performance optimizations of the CAN, UART and SPI drivers are currently not included in these mainline drivers.   

   https://www.raspberrypi.org/downloads/raspbian/


**Step 2a:**


```
sudo bash
cd /tmp
wget https://raw.githubusercontent.com/52serk/embrick_raspberry/main/install.sh -O install.sh
bash install.sh
```


<br />
<br />
<br />




