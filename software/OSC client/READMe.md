# (primitive) OSC / UDP client

- note: as is, the client works with the wm8731 version only.

- still needs adjusting port # and IP address *manually* (in `int main(void)`)

- compile with: 

`gcc *.c -Werror -lwiringPi -std=gnu99 -O2 -g -o tedium_osc` 

- run with:

`sudo ./tedium_osc`

- adc values 0-5 are sent as an OSC bundle (range = `0 (min) - 4000 (max)`): 

`/adc0 value, /adc1 value, /adc2 value ... `

- button presses (short, long) and triggers are sent as individual OSC messages (`1` or `0`), namely:

`/trigger1 value`
`/trigger2 value`
`/trigger3 value`
`/trigger4 value`

`/button1 value`
`/button2 value`
`/button3 value`

`/button1_long value`
`/button2_long value`
`/button3_long value`

- edit main.c to customize the messages.


