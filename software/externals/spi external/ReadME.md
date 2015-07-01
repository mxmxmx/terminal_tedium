Pd-L2Ork SPI external adapted for mcp3208 / terminal tedium
===========================================================

compile with:

gcc -c disis_spi.c -o disis_spi.o

ld --export-dynamic -shared -o disis_spi.pd_linux disis_spi.o -lc -lm -lwiringPi

then move into externals folder, eg: 

sudo mv disis_spi.pd_linux /usr/lib/pd/extra/