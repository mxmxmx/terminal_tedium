externals for mcp3208 + GPIO / terminal tedium
===========================================================

*usage:*


**ADC** (reads ADC when banged):
```
[open /dev/spidev0.1(
|
[terminal_tedium_adc]
|  |  |  |  |  |  |  | 
[ADC0]  [ADC1]  	[etc]
```

**gate outputs:**

left inlet: -- ; second inlet: gate #1 (top); right inlet: gate #2 (bottom)). 

sending "1" will turn the gate on, sending "0" off; the two arguments determine the initial state (0 = off, 1 = on).

```    
       o    	    o
       |          |
[tedium_output 0 0]

```

**gate/switch inputs** (outputs bang): 

```
[tedium_input <GPIO_num>] 
|
o
```

where GPIO_num = 4, 17, 2, 3, 23, 24, or 25.

====================================================================================


*compile with:*

`gcc -O3 -Wall -c [name_of_external].c -o [name_of_external].o`

`ld --export-dynamic -shared -o [name_of_external].pd_linux [name_of_external].o -lc -lm -lwiringPi`

then move into externals folder, eg: 

`sudo mv [name_of_external].pd_linux /usr/lib/pd/extra/`

