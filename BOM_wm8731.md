#BOM for terminal tedium IO 


###resistors 0603:###

- 200R            	2x 
- 220R filter bead    4x (BLM18BB221SN1D, mouser # 81-BLM18BB221SN1D)
- 510R            	5x
- 1k 					4x
- 4k7					2x 
- 5k6        		    2x
- 10k 				2x
- 24k9 				12x
- 27k					2x 
- 33k 				5x
- 47k                 5x 
- 49k9 				2x (or use 47k)
- 100k 				14x  
- led resistors:      2x (for 3mm LED) (†) 

==============================

###caps 0603 (16v or better):###

- 18-20p			6x (NP0/C0G) 
- 10n 				7x (NP0/C0G)  	 
- 100n 				14x 
- 470n 				3x
- 1uF 				2x (may be 0805)
- 10uF 				2x (may be 0805)

==============================

###electrolytic caps, SMD (25V or better):###

- 10uF 			 5x (diameter < 5.3mm)
- 33uF 			 3x (diameter < 8mm) 

==============================

###ICs/semis/etc:###

- WM8731  (TSSOP-28): 			1x  (mouser # 238-WM8731SEDS/V)
- MCP3208 (SOIC16):			1x  (mouser # 579-MCP3208CISL)
- TL072 (SOIC8):			3x  (††)
- MCP6002 (SOIC8):			1x (mouser # 579-MCP6002T-I/SN)
- MCP6004 (SOIC14):			1x (mouser # 579-MCP6004T-I/SL)
- NPN transistors (MMBT3904): 4x
- SM5817PL-TP Schottky diodes: 3x (or use 1206 ferrit bead, rated 1A)
- ADP150 regulator (3v3, TSOT): 1x (mouser # 584-ADP150AUJZ-3.3R7)
- LM4040-2v5 (SOT-23) : 1x
- LM4040-10v (SOT-23) : 1x

==============================

###through-hole parts:###

- crystal 12.288 MHz 						1x (HC-49/S low profile) 
- electolyic capacitor (10uF)				2x (= DC block / output)
- 10uH inductance 	    	    			1x (e.g. mouser # 542-78F100-RC)
- +5v DC/DC regulator RECOM 785.0 1.0.     	1x (mouser # 919-R-785.0-1.0) (††††)
- LEDs  2x (ultrabright) 

==============================

###misc:###

- jacks:			  16x (thonkiconn/kobiconn)
- pots:				  6x  (9mm vertical; linear (50k-100k will do))
- tact switches: 	  2x  (multimecs 5E/5G series) (mouser # 642-5GTH935)
- illum. tact switch: 1x  (multimecs 5E/5G series) (e.g. mouser # 642-5GTH93542 (= YELLOW)) (†††††)
- caps:
	- 2x  (multimecs “1SS09-15.0”) (mouser # 642-1SS09-15.0, or 642-1SS09-16.0)
	- 1x  transparent (multimecs “1IS11-15.0”) (mouser # 642-1IS11-15.0, or 642-1IS11-16.0)
- single row 2-pin male header (RM2.54) + little jumper plastic thingie to match (or use wire): 1x ( = the 5V power jumper)
- extra-tall female socket to match raspberry GPIO header (2x20, RM2.54): 1x (†††)
- spacer/standoff (M3) (to match the 2x20 header): 2x (c. 20mm for pi B+, pi 2)

==============================

###notes:###

(†) i'd use ultrabright 3mm leds, and correspondingly large resistor values (to taste) (V+ = 3.3v); for the illuminated button, 200R works ok (as per BOM)

(††) two of the TL072s are in the audio path — if you’d like to use something better for the output stage, eg. stuff like OPA1662 or LME49720.

(†††) four of these, for example, will do : http://www.taydaelectronics.com/connectors-sockets/stackable-headers/stackable-header-10-pins-2-54mm.html ; 
if planning to use model A+, they don't have to be as tall. 

(††††) the 500 ma version should do for models a+, b+; pi 2 needs 1.0, it looks like; as a cheaper/less efficient solution, use a 7805. if planning to power the raspberry via usb (= 5v disconnected! or no 5V regulator soldered), no such $ regulator is needed, of course.

(†††††) NB: these are crazy expensive at mouser etc; much cheaper to source them at soselectronic.com






