PANEL=EnvPanel
CLITOOL=/home/busby/env/bin/arduino-cli
# DEVICE=/dev/cu.usbmodem101
DEVICE ?= $(shell bash -c 'read -p "Port: " device; echo $$device')
# arduino:avr:nano:cpu=atmega328old
FBQN=arduino:avr:nano
AUTOGEN_FILE := $(PANEL)/version.h
AUTOGEN_NEXT := $(shell expr $$(awk '/#define BUILD_NUMBER/' $(AUTOGEN_FILE) | tr -cd "[0-9]") + 1)
.PHONY: version upload connect clean

all: $(PANEL)/Panel.h $(PANEL)/$(PANEL).ino version
	$(CLITOOL) compile --fqbn $(FBQN) $(PANEL)
version:
	sed -i "s/#define BUILD_NUMBER .*/#define BUILD_NUMBER \"$(AUTOGEN_NEXT)\"/" $(AUTOGEN_FILE)
	sed -i "s/#define BUILD_DATE.*/#define BUILD_DATE \"$$(date +'%Y%m%d%H%M')\"/" $(AUTOGEN_FILE)
upload:
	$(CLITOOL) upload -p $(DEVICE) --fqbn $(FBQN) $(PANEL)
connect:
	screen $(DEVICE) 115200
