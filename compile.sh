#!/usr/bin/env sh

if [ -z "$1" ]; then
    echo "You didn't give me a file"
    exit 1
fi

avr-gcc -std=c2x -mmcu=atmega328p -pedantic -Os -c "$1"
# avr-gcc -mmcu=atmega328p out.o -o out
avr-objcopy -O ihex -R .eeprom out a.hex


