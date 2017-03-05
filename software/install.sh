#!/bin/bash

echo ""
echo ""
echo ""
echo " .... installing things for terminal tedium ---------------------------------"
echo ""
echo "(this will take a while)"
echo ""

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
echo "installing git ................. ---------------------------------------------"
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
sudo apt-get --assume-yes purge wiringpi >/dev/null 2>&1
git clone git://git.drogon.net/wiringPi
cd wiringPi 
git pull origin 
./build >/dev/null 2>&1
rm -r -f /home/pi/wiringPi

echo ""
echo ""

echo "installing pd ... -------------------------------------------------------------"
echo ""
cd /home/pi
wget http://msp.ucsd.edu/Software/pd-0.47-1.armv7.tar.gz
tar -xvzf pd-0.47-1.armv7.tar.gz >/dev/null
rm pd-0.47-1.armv7.tar.gz

echo ""

echo "installing externals ... -------------------------------------------------------"
echo ""
cd /home/pi/terminal_tedium/software/externals/

echo " > terminal_tedium_adc"
gcc -std=c99 -O3 -Wall -c terminal_tedium_adc.c -o terminal_tedium_adc.o
ld --export-dynamic -shared -o terminal_tedium_adc.pd_linux terminal_tedium_adc.o  -lc -lm -lwiringPi
sudo mv terminal_tedium_adc.pd_linux /home/pi/pd-0.47-1/extra/

echo " > tedium_input"
gcc -std=c99 -O3 -Wall -c tedium_input.c -o tedium_input.o
ld --export-dynamic -shared -o tedium_input.pd_linux tedium_input.o  -lc -lm -lwiringPi
sudo mv tedium_input.pd_linux /home/pi/pd-0.47-1/extra/

echo " > tedium_output"
gcc -std=c99 -O3 -Wall -c tedium_output.c -o tedium_output.o
ld --export-dynamic -shared -o tedium_output.pd_linux tedium_output.o  -lc -lm -lwiringPi
sudo mv tedium_output.pd_linux /home/pi/pd-0.47-1/extra/

rm terminal_tedium_adc.o
rm tedium_input.o
rm tedium_output.o

echo ""

# create aliases

sudo cp /home/pi/terminal_tedium/software/bash_aliases ~/.bash_aliases

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

echo "boot/config ... ----------------------------------------------------------------"

sudo cp /home/pi/terminal_tedium/software/config.txt /boot/config.txt 

echo ""
echo ""

echo "alsa ... -----------------------------------------------------------------------"

sudo cp /home/pi/terminal_tedium/software/asound.conf /etc/asound.conf

echo ""
echo ""

sudo apt-get --assume-yes install alsa-utils

echo ""
echo ""

echo "done ... reboot ----------------------------------------------------------------"
sudo reboot

