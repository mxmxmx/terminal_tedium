#BOM for terminal tedium IO 
#BOM for terminal tedium IO 

###resistors 0603:

| value | # | note |
| --- | ---: | --- |
| 200R | 2x | |
| 220R | 4x | filter bead; mouser # 81-BLM18BB221SN1D |
| 510R | 5x | |
|1k | 4x | |
| 4k7 | 2x | |					
| 5k6 | 2x | |        		 
| 10k | 2x | | 				
| 24k9 | 12x | |				
| 27k | 2x | | 					
| 33k | 5x | | 			
| 47k | 5x | |                
| 49k9 | 2x | (or use 47k) |
| 100k | 14x |  |			
| LED resistors | 2x | (for the 3mm LEDs) (†) |

- (†) best to use **ultrabright** LEDs, and correspondingly large resistor values (to taste) (V+ = 3.3v); for the illuminated tact switch, 200R works ok (as per BOM)

###caps 0603 (25v or better):

| value | # | note |
| --- | ---: | --- |
| 18-20p | 6x | NP0/C0G |
| 10n | 7x | NP0/C0G |	 
| 100n | 14x | MLCC | 
| 470n | 3x | MLCC |
| 1uF  | 2x | MLCC (may be 0805) |
| 10uF | 2x | MLCC (may be 0805) |

###electrolytic caps, SMD:

| value | # | note |
| --- | ---: | --- |
| 10uF | 5x | 16V or better; diameter < 5.3mm; e.g. mouser # 647-UUQ1E100MCL1GB |
| 33uF | 3x | 25V or better; diameter < 8mm |

###ICs/semis/etc:

| what | # | package | note |
| --- | ---: | --- | --- |
| WM8731 | 1 x | TSSOP-28 | mouser # 238-WM8731SEDS/V |
| MCP3208 | 1x | SOIC16 | mouser # 579-MCP3208CISL |
| TL072 | 3x | SOIC8 | see note (†) |
| MCP6002 | 1x | SOIC8 | mouser # 579-MCP6002T-I/SN |
| MCP6004 | 1x | SOIC14 | mouser # 579-MCP6004T-I/SL |
| MMBT3904 | 4x | SOT-23 | mouser # 512-MMBT3904 |
| SM5817PL-TP | 3x | SOD-123FL | mouser # 833-SM5817PL-TP |
| ADP150 | 1x | TSOT | 3v3 regulator; mouser # 584-ADP150AUJZ-3.3R7 |
| LM4040-2v5 | 1x | SOT-23 | - |
| LM4040-10v | 1x | SOT-23 | - |

- (†) two of the TL072s are in the audio path — if you’d like, use something better for the output stage.

###through-hole parts:

| what | # | package | note |
| --- | ---: | --- | --- |
| 12.288 MHz crystal | 1x | HC-49/S |  low profile |
| electrolytic capacitor, **10uF** | 2x | 2.5mm | 16V+ (for DC block / **audio** output) |
| inductance, 10uH | 1x | - | e.g. mouser # 542-78F100-RC |
| RECOM 785.0 1.0 | 1x | - | mouser # 919-R-785.0-1.0 (†) |
| 3mm LEDs | 2x | - | (use ultra-bright) |

- (†) the 500 ma version should do for models a+, b+; pi 2 needs 1.0, it looks like; as a cheaper/less efficient solution, use a 7805. if planning to power the raspberry via usb (= 5v disconnected! or no 5V regulator soldered), no such $ regulator is needed, of course.

###misc:

| what | # | note |
| --- | ---: | --- |
| jacks | 16x | thonkiconn/kobiconn |
| pots | 6x | 9mm vertical, 10kB |
| tact switches | 2x | multimecs 5E/5G series; mouser # 642-5GTH935 |
| illum. tact switch | 1x | ditto; e.g. mouser # 642-5GTH93542 (= YELLOW) |
| caps, black | 2x | mouser # 642-1SS09-15.0, or 642-1SS09-16.0 |
| caps, transparent | 1x | mouser # 642-1IS11-15.0, or 642-1IS11-16.0 |
| 2-pin header, male | 1x | RM2.54 (+ little jumper plastic thingie to match (= the 5V power jumper)|
| raspberry GPIO socket, 2x20, RM2.54 | 1x | extra-tall female socket for RPI (†) |
| spacer/standoff (M3) | 2x | to match height of the 2x20 socket/header (c. 20mm for pi B+, pi 2, pi 3) |

- (†) or four of [these](http://www.taydaelectronics.com/connectors-sockets/stackable-headers/stackable-header-10-pins-2-54mm.html), for example, will do; if planning to use model A+ or zero, the socket doesn't have to be as tall. 

