#!/bin/bash

echo ""
echo ""
echo ""
echo ">>>>> terminal tedium <<<<<< --------------------------------------------------"
echo ""
echo "(sit back, this will take a 2-3 minutes)"
echo ""

PD_VERSION="pd-0.47-1"

HARDWARE_VERSION=$(uname -m)

if [[ "$HARDWARE_VERSION" == 'armv6l' ]]; then
		echo "--> using armv6l (A+, zero)"
elif [[ "$HARDWARE_VERSION" == 'armv7l' ]]; then
		echo "--> using armv7l (pi2, pi3)"
else
	echo "not using pi ?... exiting"
	exit -1
fi
echo ""
echo "installing git ... ------------------------------------------------------------"
echo ""
sudo apt-get --assume-yes install git-core
echo ""

echo "cloning terminal tedium repo ... ----------------------------------------------"
echo ""
cd /home/pi
rm -r -f /home/pi/terminal_tedium >/dev/null 2>&1
git clone https://github.com/mxmxmx/terminal_tedium
cd /home/pi/terminal_tedium
git pull origin

echo ""

echo "installing wiringPi ... -------------------------------------------------------"
echo ""
cd /home/pi
sudo apt-get --assume-yes purge wiringpi >/dev/null 2>&1
git clone git://git.drogon.net/wiringPi
cd wiringPi 
git pull origin 
./build >/dev/null 2>&1
rm -r -f /home/pi/wiringPi

echo ""
echo ""

echo "installing pd ($PD_VERSION)... -------------------------------------------------"
echo ""
cd /home/pi
wget http://msp.ucsd.edu/Software/$PD_VERSION.armv7.tar.gz
tar -xvzf $PD_VERSION.armv7.tar.gz >/dev/null
rm $PD_VERSION.armv7.tar.gz

echo ""

echo "installing externals ... -------------------------------------------------------"
echo ""
cd /home/pi/terminal_tedium/software/externals/

echo " > terminal_tedium_adc"
gcc -std=c99 -O3 -Wall -c terminal_tedium_adc.c -o terminal_tedium_adc.o
ld --export-dynamic -shared -o terminal_tedium_adc.pd_linux terminal_tedium_adc.o  -lc -lm -lwiringPi
sudo mv terminal_tedium_adc.pd_linux /home/pi/$PD_VERSION/extra/

echo " > tedium_input"
gcc -std=c99 -O3 -Wall -c tedium_input.c -o tedium_input.o
ld --export-dynamic -shared -o tedium_input.pd_linux tedium_input.o  -lc -lm -lwiringPi
sudo mv tedium_input.pd_linux /home/pi/$PD_VERSION/extra/

echo " > tedium_output"
gcc -std=c99 -O3 -Wall -c tedium_output.c -o tedium_output.o
ld --export-dynamic -shared -o tedium_output.pd_linux tedium_output.o  -lc -lm -lwiringPi
sudo mv tedium_output.pd_linux /home/pi/$PD_VERSION/extra/

echo " > tedium_switch"
gcc -std=c99 -O3 -Wall -c tedium_switch.c -o tedium_switch.o
ld --export-dynamic -shared -o tedium_switch.pd_linux tedium_switch.o  -lc -lm -lwiringPi
sudo mv tedium_switch.pd_linux /home/pi/$PD_VERSION/extra/

rm terminal_tedium_adc.o
rm tedium_input.o
rm tedium_output.o
rm tedium_switch.o

echo ""

# create aliases

cd /home/pi/

rm -f .bash_aliases >/dev/null
echo -n > .bash_aliases

echo "alias sudo='sudo '" >> .bash_aliases
echo "alias puredata='/home/pi/$PD_VERSION/bin/pd'" >> .bash_aliases
echo "alias pd='/home/pi/$PD_VERSION/bin/pd'" >> .bash_aliases

echo "done installing pd ... ---------------------------------------------------------"

echo ""
echo ""

echo "rc.local ... -------------------------------------------------------------------"

sudo cp /home/pi/terminal_tedium/software/rc.local /etc/rc.local

if [[ "$HARDWARE_VERSION" == 'armv6l' ]]; then
	sudo cp /home/pi/terminal_tedium/software/rt_start_armv6 /home/pi/terminal_tedium/software/rt_start
else
	sudo cp /home/pi/terminal_tedium/software/rt_start_armv7 /home/pi/terminal_tedium/software/rt_start
fi

sudo chmod +x /etc/rc.local
sudo chmod +x /home/pi/terminal_tedium/software/rt_start

echo ""
echo ""

echo "boot/config ... -----------------------------------------------------------------"

sudo cp /home/pi/terminal_tedium/software/config.txt /boot/config.txt 

echo ""
echo ""

echo "alsa ... ------------------------------------------------------------------------"

sudo cp /home/pi/terminal_tedium/software/asound.conf /etc/asound.conf

echo ""
echo ""

sudo apt-get --assume-yes install alsa-utils

echo ""
echo ""

echo "done ... clean up + reboot -------------------------------------------------------"

# remove hardware files and other stuff that's not needed
cd /home/pi/terminal_tedium/software/
rm -r externals
rm asound.conf
rm pdpd
rm pullup.py
rm rc.local
rm rt_start_armv*
rm config.txt
rm install.sh
cd /home/pi/terminal_tedium/
rm -rf hardware
rm *.md

sudo reboot
echo ""
