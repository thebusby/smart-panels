CLITOOL=/opt/homebrew/bin/arduino-cli
DEVICE=/dev/cu.usbmodem101
FBQN=arduino:avr:uno
.PHONY: upload connect clean

all: MusicPanel/Panel.h MusicPanel/MusicPanel.ino
	$(CLITOOL) compile --fqbn $(FBQN) MusicPanel
upload:
	$(CLITOOL) upload -p $(DEVICE) --fqbn $(FBQN) MusicPanel
connect:
	screen $(DEVICE) 115200
