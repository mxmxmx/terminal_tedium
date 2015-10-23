externals for mcp3208 + GPIO / terminal tedium
===========================================================


**ADC:**

```
[open /dev/spidev0.1(
|
[terminal_tedium_adc]
|  |  |  |  |  |  |  | 
[ADC0]  [ADC1]  	[etc]
```
"open /dev/spidev0.1" opens the device. reads ADC when banged.
 
**gate outputs:**

```
              o      o
              |      |
   [tedium_output 0 0]

```
first inlet: nc / second inlet: gate #1 (top) / third inlet: gate #2 (bottom). 

sending < 1 > turns the gate on, sending < 0 > off; the two arguments determine the initial state (0 = off, 1 = on).

**gate/switch inputs:** 

```
[tedium_input <GPIO_num>] 
|
o
```
where GPIO_num = 4, 17, 2, 3, 23, 24, or 25. outputs bang.

====================================================================================


**compile with:**

`gcc -O3 -Wall -c [name_of_external].c -o [name_of_external].o`

`ld --export-dynamic -shared -o [name_of_external].pd_linux [name_of_external].o -lc -lm -lwiringPi`

then move things into externals folder, eg: 

`sudo mv [name_of_external].pd_linux /usr/lib/pd/extra/`

**osx_dummies**

same thing, but non-functional ... for use on osx. 
