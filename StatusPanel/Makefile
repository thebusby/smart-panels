CLITOOL=/opt/homebrew/bin/arduino-cli
DEVICE=/dev/cu.usbmodem101
FBQN=arduino:avr:uno
.PHONY: upload connect clean

all: StatusPanel/Panel.h StatusPanel/StatusPanel.ino
	$(CLITOOL) compile --fqbn $(FBQN) StatusPanel
upload:
	$(CLITOOL) upload -p $(DEVICE) --fqbn $(FBQN) StatusPanel
connect:
	screen $(DEVICE) 115200
