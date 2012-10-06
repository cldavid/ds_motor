///
/// @file	LocalLibrary.h 
/// @brief	Library header
/// @details	<#details#>
/// @n	
/// @n @b	Project ds_motor
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
#if defined(WIRING) // Wiring specific - official
#include "Wiring.h"
#elif defined(MAPLE_IDE) // Maple specific - official
#include "WProgram.h"   
#elif defined(MPIDE) // chipKIT specific - embedXcode only
#include "WProgram.h"
#elif defined(ENERGIA) // LaunchPad specific - embedXcode only
#include "Energia.h"
#elif defined(ARDUINO) && (ARDUINO >= 100) // Arduino 1.0 specific - official
#include "Arduino.h"
#elif defined(ARDUINO) && (ARDUINO < 100) // Arduino 23 specific - official
#include "WProgram.h"
#endif

#ifndef ds_motor_LocalLibrary_h
#define ds_motor_LocalLibrary_h
#include <stddef.h>

#define SERIAL_BAUD_RATE    9600
#define SERIAL_INPUT_MAX    255

typedef struct {
    unsigned long   time;
    unsigned int    pin;
    unsigned long   rt_time;
    unsigned long   next_event;
    void          (*event_cb)(unsigned long time, unsigned int pin, unsigned long rt_time);
} event_t;

typedef struct {
	char    buffer[SERIAL_INPUT_MAX+1];
	size_t  len;
	bool    ready;
} ser_string_t; 

#define s_buffer_add(x) 	(s_input.buffer[s_input.len++] = x)
#define s_buffer			s_input.buffer
#define s_lastChar			s_input.buffer[s_input.len]
#define s_len				s_input.len
#define s_ready 			s_input.ready

void start_pump(unsigned long time, unsigned int, unsigned long rt_time);
void handleEvents(void);
void processCommand(const char *recvString);

#endif
