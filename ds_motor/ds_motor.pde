/// 
/// @mainpage	ds_motor 
/// @details	<#details#>
/// @n 
/// @n 
///
/// @author	David Cluytens
/// @author	Cluytens.net
/// @date	01/08/12 18:51
/// @version	<#version#>
/// 
/// @copyright	© David Cluytens, 2012
/// @copyright	CC = BY NC SA
///
/// @see	ReadMe.txt for references
///


// Core library - IDE-based
#include "Arduino.h"

// Include application, user and local libraries
#include <time.h>
#include <scheduler.h>
#include "LocalLibrary.h"
#include "motor.hpp"

ser_string_t 	s_input;
unsigned long   prevTime;

///
/// @brief	Setup
/// @details
///
// Add setup code 
void setup() {
	memset(&s_input, 0, sizeof(ser_string_t));
	void dc_time_init(void);
	Serial.begin(SERIAL_BAUD_RATE);

    shield_pump_init();

	/* Read Event List From EEPROM */
	eeprom_read_config();

}

///
/// @brief	Loop
/// @details	Call blink
///
// Add loop code 
void loop() {
	unsigned long time = millis();

	Time.updateTime(time);

	Scheduler.update(prevTime, Time.getUnixTime());

	if (s_ready) {
		Serial.println("");
		processCommand(s_buffer);
		s_len 	= 0;
		s_ready = false;
	}
    prevTime = Time.getUnixTime();
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
