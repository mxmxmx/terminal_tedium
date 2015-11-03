terminal_tedium
===============


![My image](https://farm1.staticflickr.com/423/19280194146_4568770dcf_c.jpg)

raspberry pi 2 / pi a+ / b+  eurorack stereo DAC (PCM5102a) or codec (wm8731) breakout board *

- 4x digital inputs (100k input impedance; threshold > 3V)
- 2x digital outputs (~ 6V )
- 6x resp. 8x CV inputs (100k input impedance; 12 bit, +/- 5V range)
- 2x audio outputs (10VPP, 16bit / 48kHz)
- 2x audio inputs (wm8731 only)


build info, see here: https://github.com/mxmxmx/terminal_tedium/wiki

## pin usage:

### wm8731 (blue) / pcm5102a (pink) : 

![My image](https://c1.staticflickr.com/1/574/22746127472_0a0b691900_b.jpg)



cf. muffwiggler.com for more details:

http://www.muffwiggler.com/forum/viewtopic.php?t=104896&postdays=0&postorder=asc&start=0

* pi 2 draws more power, so is less suitable from that perspective. 
* the 'teensy tedium' folder has gerbers + test sketch for a teensy 3.x adapter (to be used instead of the rpi, if desired. only works with pcm5102a version)
