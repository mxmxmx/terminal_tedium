#!/usr/bin/env python

# pull up GPIO23-25 (tact switches)

import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)

GPIO.setup(23, GPIO.IN, pull_up_down = GPIO.PUD_UP)
GPIO.setup(24, GPIO.IN, pull_up_down = GPIO.PUD_UP)
GPIO.setup(25, GPIO.IN, pull_up_down = GPIO.PUD_UP)

GPIO.setup(2,  GPIO.IN)
GPIO.setup(3,  GPIO.IN)
GPIO.setup(4,  GPIO.IN)
GPIO.setup(17, GPIO.IN)