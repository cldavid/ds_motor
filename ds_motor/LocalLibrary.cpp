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
extern unsigned long time;

const char *cmd_list[] = { "setUnixTime", "getUnixTime", "getDate", "test", NULL };
enum CMD_LIST { CMD_SET_UNIXTIME = 0, CMD_GET_UNIXTIME, CMD_GET_DATE, CMD_TEST, CMD_UNKNOWN };

event_t event_list[] = { 
    { 50, 100,    10, start_pump_one },
    {  0,   0,     0, NULL }
};

void start_pump_one(unsigned long time, unsigned long rt_time) {
    Serial.println(time);
    Serial.print("Executing pump one for");
    Serial.println(rt_time);
}

void testEvent() {
    size_t i;
    unsigned long time = dc_getUnixTime();
    
    for (i = 0; event_list[i].event_cb != NULL; i++)
    {
        if (time >= event_list[i].time) {
            event_list[i].event_cb(event_list[i].time, event_list[i].rt_time);
            event_list[i].time += event_list[i].next_event;
        }
    }
    return;
}

void processCommand(const char *recvString) {
	char cmd[64];
	char arg[64];
    size_t i;
    
	sscanf(recvString, "%s %s", cmd, arg);

    for (i = 0; cmd_list[i] != NULL; i++) {
        if (!strcmp(cmd_list[i], cmd)) {
            break;
        }
    }
    
    switch(i) {
        case CMD_SET_UNIXTIME:
        {
            unsigned long epoch = strtoul(arg, NULL, 10);
            dc_setUnixTime(epoch);
            break;
        }
            
        case CMD_GET_UNIXTIME:
        {
       		Serial.println(dc_getUnixTime());     
            break;
        }
            
        case CMD_GET_DATE:
        {
            char humanTime[24] = "";
            dc_ctime(humanTime, sizeof(humanTime));
            Serial.print(humanTime);
            Serial.println("");
            break;
        }
            
        case CMD_TEST:
        {
            char tmp[64];
            unsigned long tijd = time / 1000;
            sprintf(tmp,"UnixTime %d millis to seconds", dc_getUnixTime());
            Serial.print(tmp);
            Serial.println(tijd);
            break;
        }
            
        default:
            break;
            
    }
    return;
}