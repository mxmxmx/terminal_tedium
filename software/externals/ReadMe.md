externals for mcp3208 + GPIO / terminal tedium
===========================================================


## ADC: terminal_tedium_adc
- wm8731 version (6 channels: outlets 1 - 6):
<img src="https://c1.staticflickr.com/5/4286/35410513881_823463e5e9_o.png" width="50%">

- message < open > opens the SPI device. reads ADC when banged. 

- **NB:** plain `[open(` is for use with the wm8731 version (in which case, we only need to read 6 ADC channels). use `[open adc(` for use with the pcm5102a version (in which case all 8 channels need to be read):
<img src="https://c1.staticflickr.com/5/4232/35410513941_bdf67d3bc3_b.jpg" width="50%">

- the ADC object has **two additional methods**, `[smooth(` and `[deadband(`. if the ADC is jittery, you can use those to smooth over some of that. for instance, `[smooth 4(` will average over four input samples,  `[smooth 8(` over eight, etc (available values are 1x, 2x, 4x, 8x, 16x); `[deadband(` takes values from 0-5. default is: `smooth` = 1x, `deadband` = 0. 

 
## gate outputs: tedium_output
<img src="https://c1.staticflickr.com/5/4278/34699462074_471051bb94_b.jpg" width="50%">
inlet: 
sending < 1 > turns the gate on, sending < 0 > off; the creation arguments gives the pin number, where GPIO_num = 12, 16, or 26.

## gate/switch inputs: tedium_input
<img src="https://c1.staticflickr.com/5/4209/35410514081_c1cc6ac906_b.jpg" width="50%">
where GPIO_num = 4, 17, 2, 3, 23, 24, or 25. outputs bang.

## switch inputs (alternative): tedium_switch 
<img src="https://c1.staticflickr.com/5/4213/35410514191_7b3abe0c24_b.jpg" width="50%">
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
