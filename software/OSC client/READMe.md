# (primitive) OSC / UDP client

- atm, works with wm8731 version only.

- atm, still needs adjusting port # and IP address manually  (in `int main(void)` / main.c)

- compile with: 

`gcc *.c -Werror -lwiringPi -std=c99 -O2 -g -o tedium_osc` 

- run with:

`sudo ./tedium_osc`

- adc values are sent as an OSC bundle: 

`/adc0 value, /adc1 value, /adc2 value ... `

- buttons and triggers sent as OSC messages, ie:

`/trigger1 value`
`/trigger2 value`
`/trigger3 value`
`/trigger4 value`

`/button1 value`
`/button2 value`
`/button3 value`

- button3 is latching (LED); buttons 1 and 2 momentary.


