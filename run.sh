#!/bin/bash

arduino-cli compile --fqbn arduino:avr:uno Nokia5110_Shield/Nokia5110_Shield.ino

arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno Nokia5110_Shield/Nokia5110_Shield.ino

./waf build run --app=project
