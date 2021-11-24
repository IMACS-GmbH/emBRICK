#!/bin/bash

if [ $EUID -ne 0 ]; then
    echo -e "ERROR: This script should be run as root." 1>&2
    exit 1
fi

WELCOME="These Bash File will download and pack \n
following Modules for Python3:\n
- emBRICK
- threaded
- pyserial
- pymodbus
- sockets
- tabulate
- DateTime
- numpy\n"

if (whiptail --title "emBRICK Module Download & Packing Script" --yesno "$WELCOME" 20 60) then
    echo ""
else
    whiptail --title "Download & Packing failed" --msgbox "Try it again!" 8 78
    exit 0
fi

clear

NAME=$(whiptail --inputbox "How should the Package will be named?" 8 39 emBRICK-0.07-rpi.tgz --title "Example Dialog" 3>&1 1>&2 2>&3)
                                                                        # A trick to swap stdout and stderr.
# Again, you can pack this inside if, but it seems really long for some 80-col terminal users.
exitstatus=$?
if [ $exitstatus = 0 ]; then
    echo "User selected Ok and entered " $NAME
else
    echo "User selected Cancel."
fi

echo "(Exit status was $exitstatus)"

mkdir emBRICK
cd emBRICK
pip3 download --platform=linux_armv7l --only-binary=:all: emBRICK
cd ..
tar cvfz $NAME emBRICK

clear

DICT=$(pwd)
whiptail --title "Download and Packing complete" --msgbox "Modules are packed in $NAME." 8 78
rm -r $DICT/emBRICK
