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
   [1 (     
   |    
   [tedium_output <GPIO_num>]

```
inlet: 

sending < 1 > turns the gate on, sending < 0 > off; the creation arguments gives the pin number, where GPIO_num = 12, 16, or 26.

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
