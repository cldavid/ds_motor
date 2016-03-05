BUILDTIME 		= `date +%s`
CXXFLAGS_STD 		= -Wall -Werror -Wextra -DBUILDTIME=$(BUILDTIME)
CXXFLAGS_STD 		= -std=gnu++11\
 -fno-threadsafe-statics\
 -DBUILDTIME=$(BUILDTIME)\
 -DARDUINO_AVR_YUN\
 '-DUSB_MANUFACTURER="Unknown"'
 '-DUSB_PRODUCT="Arduino Yun"'

BOARD_TAG    		= yun
MONITOR_BAUDRATE  	= 115200

USER_LIB_PATH 	= /Users/davidc/Dropbox/Development/Arduino/libraries
ARDUINO_LIBS 	= dc_time\
 EEPROM\
 Adafruit_Motor_Shield_V2_Library\
 scheduler\
 print\
 Wire\
 Bridge\
 Ethernet\
 Firmata\
 SPI\
 OneWire\
 Arduino-Temperature-Control-Library


include Arduino.mk
