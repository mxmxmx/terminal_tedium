#!/bin/bash

echo ""
echo ""
echo ""
echo ">>>>> terminal tedium <<<<<< --------------------------------------------------"
echo ""
echo "(sit back, this will take a few minutes)"
echo ""

PD_VERSION="pd-0.48-1"

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
sudo apt-get --assume-yes install git-core >/dev/null 2>&1
# also installing libjack to get rid of "error while loading shared libraries"-error
sudo apt-get --assume-yes install libjack-jackd2-dev >/dev/null 2>&1
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
git clone git://git.drogon.net/wiringPi || git clone https://github.com/WiringPi/WiringPi.git wiringPi
cd wiringPi 
git pull origin 
./build >/dev/null 2>&1
rm -r -f /home/pi/wiringPi

echo ""
echo ""

echo "installing pd ($PD_VERSION)... -------------------------------------------------"
echo ""
cd /home/pi

if [[ "$HARDWARE_VERSION" == 'armv6l' ]]; then
	wget http://msp.ucsd.edu/Software/$PD_VERSION-armv6.rpi.tar.gz
	tar -xvzf $PD_VERSION-armv6.rpi.tar.gz >/dev/null
	rm $PD_VERSION-armv6.rpi.tar.gz
else
	wget http://msp.ucsd.edu/Software/$PD_VERSION.rpi.tar.gz
	tar -xvzf $PD_VERSION.rpi.tar.gz >/dev/null
	rm $PD_VERSION.rpi.tar.gz
fi

mv $PD_VERSION* $PD_VERSION

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

echo " > abl_link~"
sudo mv abl_link/abl_link~.pd_linux /home/pi/$PD_VERSION/extra/

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
cd /home/pi/
rm install.sh

sudo reboot
echo ""
