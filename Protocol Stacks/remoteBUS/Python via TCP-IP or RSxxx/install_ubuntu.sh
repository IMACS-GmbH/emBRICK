#!/bin/bash

if [ $EUID -ne 0 ]; then
    echo -e "ERROR: This script should be run as root." 1>&2
    exit 1
fi

FILE=($(find . -type f -iname "*.tgz"))

WELCOME="These Bash File will extract and install or upgrade\n
following Modules for Python3:\n
- emBRICK
- threaded
- pyserial
- pymodbus
- sockets
- tabulate
- DateTime
- numpy\n"

FILEINFO="Founded following File to extract: $FILE\nStart with installing?"

if (whiptail --title "emBRICK Module Installation Script" --yesno "$WELCOME" 20 60) then
    echo ""
else
	whiptail --title "Installation failed" --msgbox "Try it again!" 8 78
    exit 0
fi

clear

if (whiptail --title "emBRICK Module Installation Script" --yesno "$FILEINFO" 20 60) then
    tar xvfz $FILE
	cd emBRICK
	python3 -m pip install emBRICK-0.07.tar.gz -f ./ --no-index
else
	whiptail --title "Installation failed" --msgbox "Try it again!" 8 78
    exit 0
fi

clear

cd ..
DICT=$(pwd)
whiptail --title "Installation complete" --msgbox "Extracted Folder emBRICK will be deleted!" 8 78
rm -r $DICT/emBRICK
