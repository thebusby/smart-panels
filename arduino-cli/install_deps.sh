#!/usr/bin/bash -x
#
# Install library dependencies for Panel.h via arduino-cli
#

# Support for PCF8575 port expander
# Rob Tillaart's
# :VERSION: 0.1.8
# :URL: https://github.com/RobTillaart/PCF8575
arduino-cli lib install 'PCF8575'

# Support 20x4 LCD displays
# Macro Schwartz's library by Frank de Brabander for DFRobot I2C LDC displays
# :VERSION: 1.1.2
# :URL: https://github.com/marcoschwartz/LiquidCrystal_I2C.git
arduino-cli lib install 'LiquidCrystal I2C'

# Support DHT11 and DHT22 temp and humidity sensor
# By adafruit
# :VERSION: 1.4.2
# :URL: https://github.com/adafruit/DHT-sensor-library
arduino-cli lib install 'DHT sensor library'

# Support TM1637 7-segment four digit displays
# By Seeed Studio
# :VERSION: 1.0.0
# :URL: https://github.com/Seeed-Studio/Grove_4Digital_Display
arduino-cli lib install 'Grove 4-Digit Display'

Adafruit_MCP23017

# Support DS3231M RTC (Real-Time-Clock)
# By Arnd <Zanshin_Github@sv-zanshin.com>
# :VERSION: 1.0.6
# :URL: https://github.com/Zanduino/DS3231M
arduino-cli lib install 'DS3231M'

# Support MAX7129 and MAX7221 7-Segment eight digit displays
# By Eberhard Fahle <e.fahle@wayoda.org>
# :VERSION: 1.0.6
# :URL: http://wayoda.github.io/LedControl/
arduino-cli lib install 'LedControl'

# Support MCP23017 I2C port expander
# By adafruit
# :VERSION: 1.3.0
# :URL: https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library
arduino-cli lib install 'Adafruit MCP23017 Arduino Library'
