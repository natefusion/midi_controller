PROJECT = $(notdir $(CURDIR))
OUTPUT = $(PROJECT)
DEVICE = atmega328p

ifeq ($(OS),Windows_NT)
	PLATFORM_OS=WINDOWS
else
	UNAMEOS=$(shell uname)
	ifeq ($(UNAMEOS),Linux)
		PLATFORM_OS=LINUX
	endif
endif

MAKE = make
SERIAL = /dev/ttyACMO

ifeq ($(PLATFORM_OS),WINDOWS)
	MAKE = mingw32-make
	SERIAL = COM3
endif

all: debug

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
	$(UPLOAD) -c arduino -p $(DEVICE) -b 115200 -P $(SERIAL) -U flash:w:"$(OUTPUT).hex":i
