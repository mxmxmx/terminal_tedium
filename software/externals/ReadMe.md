externals for mcp3208 + GPIO / terminal tedium
===========================================================


## ADC:

wm8731 version (6 channels): 

```
[open(
|
[terminal_tedium_adc]
|  |  |  |  |  |  |  | 
[ADC0]  [ADC1]  ... 	[ADC5]
```

pcm5102a version (8 channels): 

```
[open adc(
|
[terminal_tedium_adc]
|  |  |  |  |  |  |  | 
[ADC0]  [ADC1]  ... 	[ADC7]
```

message "open" opens the device. reads ADC when banged. (NB: plain `[open(` is for use with the wm8731 version (in which case, we only need to read 6 ADC channels. use `[open adc(` for use with the pcm5102a version, in which case all 8 channels need to be read.). the object has two additional methods, `[smooth(` and `[deadband(`. if the ADC is jittery, you can use those to smooth over some of that. for instance, `[smooth 4(` will average over four input samples,  `[smooth 8(` over eight, etc (available values are 1x, 2x, 4x, 8x, 16x); `[deadband(` takes values from 0-5. default is: `smooth` = 1x, `deadband` = 0. 

 
## gate outputs:

```
   [1 (     
   |    
   [tedium_output <GPIO_num>]

```
inlet: 

sending < 1 > turns the gate on, sending < 0 > off; the creation arguments gives the pin number, where GPIO_num = 12, 16, or 26.

## gate/switch inputs:

```
[tedium_input <GPIO_num>] 
|
o
```
where GPIO_num = 4, 17, 2, 3, 23, 24, or 25. outputs bang.

## switch inputs (alternative), use for long press etc:

```
[tedium_switch <GPIO_num>] 
|						|
o 						o
```
where GPIO_num = 23, 24, or 25. 
left outlet: time switch is held down (in milliseconds).
right outlet: push = < 1 > / release = < 0 >.

====================================================================================


**compile with:**

`gcc -std=c99 -O3 -Wall -c [name_of_external].c -o [name_of_external].o`

`ld --export-dynamic -shared -o [name_of_external].pd_linux [name_of_external].o -lc -lm -lwiringPi`

then move things into externals folder, eg: 

`sudo mv [name_of_external].pd_linux /usr/lib/pd/extra/`

**osx_dummies**

same thing, but non-functional ... for use on osx. 
