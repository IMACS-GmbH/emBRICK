#!/bin/bash

export LC_ALL=C

# Unser Github Server
REPORAW="https://raw.githubusercontent.com/IMACS-GmbH/emBRICK/main/Hardware%20Products/RaspberryBrick/Driver-SPI2Serial-SC16IS740"

ERR='\033[0;31m'
INFO='\033[0;32m'
NC='\033[0m' # No Color

if [ $EUID -ne 0 ]; then
    echo -e "$ERR ERROR: This script should be run as root. $NC" 1>&2
    exit 1
fi

wget -q --spider https://www.github.com
if [ $? -ne 0 ]; then
        echo -e "$ERR ERROR: Internet connection required! $NC" 1>&2
        exit 1
fi

FREE=`df $PWD | awk '/[0-9]%/{print $(NF-2)}'`
if [[ $FREE -lt 1048576 ]]; then
  echo -e "$ERR ERROR: 1GB free disk space required (run raspi-config, 'Expand Filesystem') $NC" > /dev/stderr
  exit 1
fi


clear
WELCOME="These drivers will be compiled and installed:\n
- CAN driver (SocketCAN)
- Serial driver (RS232/RS485)
- SPI driver\n
These software components will be installed:\n
- libncurses5-dev, gcc, build-essential, raspberrypi-kernel-headers, lib, autoconf, libtool, libsocketcan, can-utils\n
continue installation?"

if (whiptail --title "Driver-SPI2Serial-SC16IS740 Installation Script" --yesno "$WELCOME" 20 60) then
    echo ""
else
    exit 0
fi

apt-get update
apt-get upgrade -y

# compile device tree files

wget -nv $REPORAW/src/mcp2515-can0-overlay.dts -O mcp2515-can0-overlay.dts
wget -nv $REPORAW/src/sc16is7xx-ttysc0-rs232-rs485-overlay.dts -O sc16is7xx-ttysc0-rs232-rs485-overlay.dts
wget -nv $REPORAW/src/mcp7940x-i2c-rtc-overlay.dts -O mcp7940x-i2c-rtc-overlay.dts

dtc -@ -H epapr -O dtb -W no-unit_address_vs_reg -o mcp2515-can0.dtbo -b 0 mcp2515-can0-overlay.dts
dtc -@ -H epapr -O dtb -W no-unit_address_vs_reg -o sc16is7xx-ttysc0-rs232-rs485.dtbo -b 0 sc16is7xx-ttysc0-rs232-rs485-overlay.dts

dtc -@ -H epapr -O dtb -W no-unit_address_vs_reg -o mcp7940x-i2c-rtc.dtbo -b 0 mcp7940x-i2c-rtc-overlay.dts

if [ ! -f "sc16is7xx-ttysc0-rs232-rs485.dtbo" ] || [ ! -f "mcp2515-can0.dtbo" ] || [ ! -f "mcp7940x-i2c-rtc.dtbo" ]; then
 echo -e "$ERR Error: Installation failed! (driver device tree build failed) $NC" 1>&2
 whiptail --title "Error" --msgbox "Installation failed! (driver device tree build failed)" 10 60
 exit 1
fi


/bin/cp -rf mcp2515-can0.dtbo /boot/overlays/mcp2515-can0.dtbo
/bin/cp -rf sc16is7xx-ttysc0-rs232-rs485.dtbo /boot/overlays/sc16is7xx-ttysc0-rs232-rs485.dtbo
/bin/cp -rf mcp7940x-i2c-rtc.dtbo /boot/overlays/mcp7940x-i2c-rtc.dtbo

# register new driver modules
depmod -a

# update 2018-10 : check on every boot if J301 is set. if it is not set, then set RS485 mode automatically in driver using ioctl
#wget -nv $REPORAW/src/tty-auto-rs485.c -O tty-auto-rs485.c
wget -nv $REPORAW/src/tty-auto-rs485.c -O tty-auto-rs485.c
gcc tty-auto-rs485.c -o /usr/sbin/tty-auto-rs485

if [ ! -f "/usr/sbin/tty-auto-rs485" ]; then
 echo -e "$ERR Error: Installation failed! (could not build tty-auto-rs485) $NC" 1>&2
 whiptail --title "Error" --msgbox "Installation failed! (could not build tty-auto-rs485)" 10 60
 exit 1
fi

if grep -q "gpio24" "/etc/rc.local"; then
        echo ""
else
        echo -e "$INFO INFO: Installing RS232/RS485 jumper check in /etc/rc.local $NC"
        sed -i 's/exit 0//g' /etc/rc.local
        echo '# if jumper J301 is not set, switch /dev/ttySC0 to RS485 mode' >>/etc/rc.local
        echo '/bin/echo '"'"'24'"'"' > /sys/class/gpio/export || true; /bin/echo '"'"'in'"'"' > /sys/class/gpio/gpio24/direction && /bin/cat /sys/class/gpio/gpio24/value | /bin/grep '"'"'1'"'"' && /usr/sbin/tty-auto-rs485 /dev/ttySC0' >>/etc/rc.local
        echo "_NetworkStatus=$(cat /sys/class/net/eth0/operstate)"  >>/etc/rc.local
	echo "if [ "$_NetworkStatus" = "down" ]; then" >>/etc/rc.local
	echo "\t sudo hwclock --hctosys" >>/etc/rc.local >>/etc/rc.local
	echo "fi" >>/etc/rc.local
	echo "exit 0" >>/etc/rc.local
fi


WELCOME2="These configuration settings will automatically be made:\n
- Install default config.txt
- Install SocketCAN initialization as service
- Install RTC initialization in initramfs
- Increase USB max. current
- Enable I2C and SPI drivers
- Set green LED as SD card activity LED\n"

# emPC-A/RPI3B
cat /proc/cpuinfo | grep Revision | grep "082" >/dev/null
if (($? == 0)); then
        WELCOME2=$WELCOME2"- Disable Bluetooth (enable serial console)\n"
        WELCOME2=$WELCOME2"- Set CPU frequency to fixed 600MHZ\n"
fi

# emPC-A/RPI3B+
cat /proc/cpuinfo | grep Revision | grep "0d3" >/dev/null
if (($? == 0)); then
        WELCOME2=$WELCOME2"- Disable Bluetooth (enable serial console)\n"
        WELCOME2=$WELCOME2"- Set CPU frequency to fixed 600MHZ\n"
fi

WELCOME2=$WELCOME2"\ncontinue installation?"


DATE=$(date +"%Y%m%d_%H%M%S")
echo -e "$INFO INFO: creating backup copy of config: /boot/config-backup-$DATE.txt $NC" 1>&2
/bin/cp -rf /boot/config.txt /boot/config-backup-$DATE.txt

echo -e "$INFO INFO: Using default config.txt $NC" 1>&2
wget -nv $REPORAW/src/config.txt -O /boot/config.txt


# installing service to start can0 on boot
if [ ! -f "/bin/systemctl" ]; then
    echo -e "$ERR Warning: systemctl not found, cannot install can0.service $NC" 1>&2
else
    wget -nv $REPORAW/src/can0.service -O /lib/systemd/system/can0.service
    systemctl enable can0.service
fi

echo -e "$INFO INFO: Installing RTC hardware clock $NC" 1>&2
apt-get -y install i2c-tools
# disable fake clock (systemd)
systemctl disable fake-hwclock
systemctl mask fake-hwclock

# disable fake clock (init.d)
service fake-hwclock stop

rm -f /etc/cron.hourly/fake-hwclock
update-rc.d fake-hwclock disable

service hwclock.sh stop
update-rc.d hwclock.sh disable

if test -e /lib/systemd/system/hwclock.service; then
	# if exists from last installation (legacy, no longer used)
	echo -e "$INFO INFO: deinstalling hwclock.service $NC"
	systemctl stop hwclock || true
	systemctl disable hwclock || true
	systemctl mask hwclock || true
	rm -f /lib/systemd/system/hwclock.service
fi


echo -e "$INFO INFO: Disabling Bluetooth to use serial port $NC"
systemctl disable hciuart


if grep -q "ssh_host_dsa_key" "/etc/rc.local"; then
        echo ""
else
        echo -e "$INFO INFO: Installing SSH key generation /etc/rc.local $NC"
        sed -i 's/exit 0//g' /etc/rc.local
        echo "test -f /etc/ssh/ssh_host_dsa_key || dpkg-reconfigure openssh-server" >>/etc/rc.local
        echo "exit 0" >>/etc/rc.local
fi



# read RTC time in initramfs (early as possible)
wget -nv $REPORAW/src/hwclock.hooks -O /etc/initramfs-tools/hooks/hwclock
wget -nv $REPORAW/src/hwclock.init-bottom -O /etc/initramfs-tools/scripts/init-bottom/hwclock

chmod +x /etc/initramfs-tools/hooks/hwclock
chmod +x /etc/initramfs-tools/scripts/init-bottom/hwclock

echo -e "$INFO INFO: generating initramfs $NC"
mkinitramfs -o /boot/initramfs.gz

if test -e /boot/initramfs.gz; then
	echo -e "$INFO INFO: Installing initramfs $NC"
	echo "initramfs initramfs.gz followkernel" >>/boot/config.txt
fi



wget -nv $REPORAW/scripts/empc-can-configbaudrate.sh -O /usr/sbin/empc-can-configbaudrate.sh
chmod +x /usr/sbin/empc-can-configbaudrate.sh
/usr/sbin/empc-can-configbaudrate.sh


if [ ! -f "/usr/local/bin/cansend" ]; then
 if (whiptail --title "Driver-SPI2Serial-SC16IS740 Installation Script" --yesno "Third party SocketCan library and utilities\n\n- libsocketcan-0.0.10\n- can-utils\n - candump\n - cansend\n - cangen\n\ninstall?" 16 60) then

    apt-get -y install git
    apt-get -y install autoconf
    apt-get -y install libtool

    cd /usr/src/

    wget http://www.pengutronix.de/software/libsocketcan/download/libsocketcan-0.0.10.tar.bz2
    tar xvjf libsocketcan-0.0.10.tar.bz2
    rm -rf libsocketcan-0.0.10.tar.bz2
    cd libsocketcan-0.0.10
    ./configure && make -j4 && make install

    cd /usr/src/

    git clone https://github.com/linux-can/can-utils.git
    cd can-utils
    ./autogen.sh
    ./configure && make -j4 && make install

 fi
fi



if [ ! -f "/etc/CODESYSControl.cfg" ]; then
    echo ""
else
    echo -e "$INFO INFO: CODESYS installation found $NC"

 if (whiptail --title "CODESYS installation found" --yesno "CODESYS specific settings:\n- Set SYS_COMPORT1 to /dev/ttySC0\n- Install rts_set_baud.sh SocketCan script\n\ninstall?" 16 60) then

    wget -nv $REPORAW/src/codesys-settings.sh -O /tmp/codesys-settings.sh
    bash /tmp/codesys-settings.sh

 fi

fi


if grep -q "sc16is7xx" "/etc/profile"; then
        echo ""
else
        echo -e "$INFO INFO: Installing driver installation check in /etc/profile $NC"
        echo "" >>/etc/profile
        echo "/sbin/lsmod | /bin/grep sc16is7xx >>/dev/null || /bin/echo -e \"\033[1;31mError:\033[0m driver for SPI2Serial-SC16IS740 RS232/RS485 port not loaded! installation instructions: https://github.com/IMACS-GmbH/emBRICK/edit/master/Driver-SPI2Serial-SC16IS740\"" >>/etc/profile
        echo "/sbin/lsmod | /bin/grep mcp251x >>/dev/null || /bin/echo -e \"\033[1;31mError:\033[0m driver for SPI2Serial-SC16IS740 CAN port not loaded! installation instructions: https://github.com/IMACS-GmbH/emBRICK/edit/master/Driver-SPI2Serial-SC16IS740\"" >>/etc/profile
        echo "/sbin/lsmod | /bin/grep rtc_ds1307 >>/dev/null || /bin/echo -e \"\033[1;31mError:\033[0m driver for  SPI2Serial-SC16IS740 RTC not loaded! installation instructions: https://github.com/IMACS-GmbH/emBRICK/edit/master/Driver-SPI2Serial-SC16IS740\"" >>/etc/profile
fi


cd /

if (whiptail --title "Driver-SPI2Serial-SC16IS740 Installation Script" --yesno "Installation completed! reboot required\n\nreboot now?" 12 60) then

    reboot

fi
