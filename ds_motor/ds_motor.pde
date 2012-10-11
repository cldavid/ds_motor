/// 
/// @mainpage	ds_motor 
/// @details	<#details#>
/// @n 
/// @n 
/// @n @a	Developed with [embedXcode](http://embedXcode.weebly.com)
/// 
/// @author	David Cluytens
/// @author	__MyCompanyName__
/// @date	01/08/12 18:51
/// @version	<#version#>
/// 
/// @copyright	© David Cluytens, 2012
/// @copyright	CC = BY NC SA
///
/// @see	ReadMe.txt for references
///


// Core library - IDE-based
#if defined(WIRING) // Wiring specific
#include "Wiring.h"
#elif defined(MAPLE_IDE) // Maple specific
#include "WProgram.h"   
#elif defined(MPIDE) // chipKIT specific
#include "WProgram.h"
#elif defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#elif defined(ARDUINO) && (ARDUINO >= 100) // Arduino 1.0 specific
#include "Arduino.h"
#elif defined(ARDUINO) && (ARDUINO < 100) // Arduino 23 specific
#include "WProgram.h"
#endif

// Include application, user and local libraries
#include <dc_time.h>
#include "LocalLibrary.h"

ser_string_t 	s_input;
unsigned long 	time;

///
/// @brief	Setup
/// @details
///
// Add setup code 
void setup() {
	memset(&s_input, 0, sizeof(ser_string_t));
	void dc_time_init(void);
	Serial.begin(SERIAL_BAUD_RATE); 
    pinMode(8, OUTPUT); 
    pinMode(9, OUTPUT); 
    pinMode(10, OUTPUT); 
    pinMode(11, OUTPUT); 
    pinMode(12, OUTPUT); 
    pinMode(13, OUTPUT);
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
    digitalWrite(13, LOW);
        
    /* Read Event List From EEPROM */
    eeprom_read_config();
}

///
/// @brief	Loop
/// @details	Call blink
///
// Add loop code 
void loop() {
    unsigned long cur_time;
    static unsigned long prev_time;
    
	time = millis();
	
    dc_updateTime(time);
    cur_time = dc_getUnixTime();

    handleEvents(cur_time, prev_time);
    
	if (s_ready) {
		Serial.println("");
		processCommand(s_buffer);
		s_len 	= 0;
		s_ready = false;
	}
    prev_time = cur_time;
}

void serialEvent() {
	char inChar;
	/* 
	   Only read from the serial input when data is available
	   And output buffer is not being handled 
	 */
	while (Serial.available()) {
		inChar	= (char)Serial.read(); 										
		Serial.print(inChar); 									
		if (inChar == '\n' || inChar == '\r' || s_len > SERIAL_INPUT_MAX) {
			s_buffer_add('\0');	
			s_ready = true;	
			return;										
		}
		s_buffer_add(inChar); 				
	}
	return;
}
