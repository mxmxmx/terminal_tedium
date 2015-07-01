externals for mcp3208 + GPIO / terminal tedium
===========================================================

*usage:*


**ADC:**

[open /dev/spidev0.1[

|

[disis_spi]

| | | | | | | | 

[ADC0]  [ADC1]  [etc]

**gate outputs:**

left argument: gate 1 (top); right argument: gate 2 (bottom)).



     |  |
[tt_gpio]


**gate/switch inputs:**


[tedium_clk_in <GPIO_num>] 

|
 
where GPIO_num = 4, 17, 2, 3, 23, 24, or 25



*compile with:*

gcc -c [name_of_external].c -o [name_of_external].o

ld --export-dynamic -shared -o [name_of_external].pd_linux [name_of_external].o -lc -lm -lwiringPi

then move into externals folder, eg: 

sudo mv [name_of_external].pd_linux /usr/lib/pd/extra/

