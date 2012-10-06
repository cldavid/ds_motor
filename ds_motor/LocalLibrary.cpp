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
// __Cluytens.Net__
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

#define P_MOTOR_1 9
#define P_MOTOR_2 10
#define P_MOTOR_3 11

extern unsigned long time;

const char *cmd_list[] = { 
    "setUnixTime", 
    "getUnixTime", 
    "getDate", 
    "test", 
    "drive_motor_1",
    "drive_motor_2",
    "drive_motor_3",
    NULL 
};
enum CMD_LIST { 
    CMD_SET_UNIXTIME = 0, 
    CMD_GET_UNIXTIME, 
    CMD_GET_DATE, 
    CMD_TEST, 
    CMD_MOTOR1,
    CMD_MOTOR2,
    CMD_MOTOR3,
    CMD_UNKNOWN 
};

event_t event_list[] = { 
    { 50, 2500,    P_MOTOR_1, 86400, start_pump },
    { 50, 2500,    P_MOTOR_2, 86400, start_pump },
    { 50, 2500,    P_MOTOR_3, 86400, start_pump },
    {  0,    0,            0,     0,       NULL }
};

void start_pump(unsigned long time, unsigned int pin, unsigned long rt_time) {
    Serial.println(time);
    Serial.print("Starting motor for ");
    Serial.println(rt_time);
    digitalWrite(pin, HIGH);
    delay(rt_time);
    digitalWrite(pin, LOW);
    return;
}

void handleEvents() {
    size_t i;
    unsigned long time = dc_getUnixTime();
    
    for (i = 0; event_list[i].event_cb != NULL; i++)
    {
        if (time >= event_list[i].time) {
            event_list[i].event_cb(event_list[i].time, event_list[i].pin, event_list[i].rt_time);
            event_list[i].time += event_list[i].next_event;
        }
    }
    return;
}

void processCommand(const char *recvString) {
	char    cmd[64]    = "";
	char    arg[64]    = "";
    size_t  i          = 0;

    unsigned long epoch = dc_getUnixTime();
    
	sscanf(recvString, "%s %s", cmd, arg);

    for (i = 0; cmd_list[i] != NULL; i++) {
        if (!strcmp(cmd_list[i], cmd)) {
            break;
        }
    }
    
    switch(i) {
        case CMD_SET_UNIXTIME:
        {
            unsigned long set_epoch = strtoul(arg, NULL, 10);
            dc_setUnixTime(set_epoch);
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
            sprintf(tmp,"UnixTime %d millis to seconds ", epoch);
            Serial.print(tmp);
            Serial.println(tijd);
            break;
        }
        
        
        case CMD_MOTOR1:
        {
            unsigned int    pin = P_MOTOR_1;
            unsigned int    t   = atoi(arg);
            
            start_pump(epoch, pin, t);
            break;
        }
            
        case CMD_MOTOR2:
        {
            unsigned int    pin = P_MOTOR_2;
            unsigned int    t   = atoi(arg);
            
            start_pump(epoch, pin, t);
            break;
        }
            
        case CMD_MOTOR3:
        {
            unsigned int    pin = P_MOTOR_3;
            unsigned int    t   = atoi(arg);

            start_pump(epoch, pin, t);
            break;
        }
            
        default:
            break;
            
    }
    return;
}