BUILDTIME 		= `date +%s`
CXXFLAGS_STD 		= -Wall -Werror -Wextra -DBUILDTIME=$(BUILDTIME)
CXXFLAGS_STD 		= -DBUILDTIME=$(BUILDTIME)
BOARD_TAG    		= uno
MONITOR_BAUDRATE  	= 115200

USER_LIB_PATH 	= /Users/davidc/Dropbox/Development/Arduino/libraries
ARDUINO_LIBS 	= dc_time EEPROM Adafruit_Motor_Shield_V2_Library scheduler print Wire 

include Arduino.mk
