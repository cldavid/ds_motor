//
// LocalLibrary.cpp 
// Library C++ code
// ----------------------------------
// Developed with embedXcode 
// http://embedXcode.weebly.com
//
// Project ds_motor
//
// Created by David Cluytens, 01/08/12 18:51
// __MyCompanyName__
//	
//
// Copyright © David Cluytens, 2012
// Licence CC = BY NC SA
//
// See LocalLibrary.h and ReadMe.txt for references
//


#include "LocalLibrary.h"
#include "string.h"
#include "dc_time.h"

void processCommand(const char *recvString) {
	char cmd[64];
	char arg[64];
    
	sscanf(recvString, "%s %s", cmd, arg);
    
	if (!strcmp("setEpoch", cmd)) {
		unsigned long epoch = strtoul(arg, NULL, 10);
		dc_setUnixTime(epoch);
		return;
	}
    
	if (!strcmp("getEpoch", cmd)) {
		Serial.println(dc_getUnixTime());
		return;
	}
    
    if (!strcmp("getDate", cmd)) {
        char humanTime[24] = "";
		dc_ctime(humanTime, sizeof(humanTime));
		Serial.print(humanTime);
		Serial.println("");
		return;   
    }
    
	if (!strcmp("Hello World", recvString)) {
		Serial.print("Received Hello World");
		Serial.println("");
	}
}