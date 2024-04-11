PROJECT = $(notdir $(CURDIR))
OUTPUT = target/$(PROJECT)
DEVICE = atmega328p

all: mkdir debug

mkdir:
	mkdir -p ./target/

SRC = $(wildcard src/*.c)
CC = avr-gcc
OBJCOPY = avr-objcopy
UPLOAD = avrdude
FLAGS = -Wall -pedantic -pipe -Os -mmcu=$(DEVICE)
debug: compile executable

compile: $(SRC)
	$(CC) $(SRC) -o $(OUTPUT) $(FLAGS)

executable: $(OUTPUT)
	$(OBJCOPY) -O ihex -R .eeprom $(OUTPUT) $(OUTPUT).hex

upload: $(OUTPUT).hex
	$(UPLOAD) -c arduino -p $(DEVICE) -b 115200 -P /dev/ttyACMO -U flash:w:"$(OUTPUT).hex":i

.PHONY: clean

clean:
	rm -f ./target/*

