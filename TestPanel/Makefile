CLITOOL=/opt/homebrew/bin/arduino-cli
DEVICE=/dev/cu.usbmodem101
FBQN=arduino:avr:uno
.PHONY: upload connect clean

all: TestPanel/Panel.h TestPanel/TestPanel.ino
	$(CLITOOL) compile --fqbn $(FBQN) TestPanel
upload:
	$(CLITOOL) upload -p $(DEVICE) --fqbn $(FBQN) TestPanel
connect:
	screen $(DEVICE) 115200
