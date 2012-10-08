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

#define P_MOTOR_1   9
#define P_MOTOR_2   10
#define P_MOTOR_3   11
#define S_WIDTH     80

extern unsigned long time;

enum M_LIST {
    MOTOR_1 = 1,
    MOTOR_2,
    MOTOR_3
};

const char *cmd_list[] = { 
    "help",
    "setUnixTime", 
    "getUnixTime", 
    "getDate", 
    "drive_motor_1",
    "drive_motor_2",
    "drive_motor_3",
    "set_motor_1",
    "set_motor_2",
    "set_motor_3",
    "get_motor_1",
    "get_motor_2",
    "get_motor_3",
    "system_info",
    "debug",
    NULL 
};

enum CMD_LIST {
    CMD_HELP         = 0,
    CMD_SET_UNIXTIME, 
    CMD_GET_UNIXTIME, 
    CMD_GET_DATE,  
    CMD_MOTOR1,
    CMD_MOTOR2,
    CMD_MOTOR3,
    CMD_SET_MOTOR1,
    CMD_SET_MOTOR2,
    CMD_SET_MOTOR3,
    CMD_GET_MOTOR1,
    CMD_GET_MOTOR2,
    CMD_GET_MOTOR3,
    CMD_SYSTEM_INFO,
    CMD_DEBUG,
    CMD_UNKNOWN 
};

#define T_DAY               86400
#define NOT_USED            0xffffffff

unsigned int debug  =   0;

/*
 *          Event Scheduler List (event_t)
 * 
 * Index [0-5] are reserved for motor 1,2 and 3
 *
 * Settings for Motor 1 are located at index 0 and 1
 * Settings for Motor 2 are located at index 2 and 3
 * Settings for Motor 3 are located at index 4 and 5
 *
 * Index [6-... Free to use
 *
 */
 
event_t event_list[] = { 
    { 30,    0,    P_MOTOR_1, T_DAY, start_pump     },
    { 33,    0,    P_MOTOR_1, T_DAY, stop_pump      },
    
    { 30,    0,    P_MOTOR_2, T_DAY, start_pump     },
    { 34,    0,    P_MOTOR_2, T_DAY, stop_pump      },
    
    { 30,    0,    P_MOTOR_3, T_DAY, start_pump     },
    { 35,    0,    P_MOTOR_3, T_DAY, stop_pump      },

    { 10,    0,    NOT_USED,     10, print_datetime },
/*  LAST ELEMENT DO NOT CHANGE THIS */
    {  0,    0,    NOT_USED,     0,  NULL           }      
};

void get_motor_event_info(unsigned int motor) {
    char buffer[S_WIDTH];
    size_t index;
    
    if (motor <= 0 || motor > 3 ) {
        println("Invalid motor selected");
        return;
    }

    /* 
     * Motor 1 => Index = (1 - 1) * 2 = 0;
     * Motor 2 => Index = (2 - 1) * 2 = 2;
     * Motor 3 => Index = (3 - 2) * 2 = 4;
     */
    index = (motor - 1) * 2;

    snprintf(buffer, S_WIDTH, "Motor %u", motor);
    println(buffer);
    
    snprintf(buffer, S_WIDTH, "Start: %lu", event_list[index].time);
    println(buffer);
    
    snprintf(buffer, S_WIDTH, "Stop: %lu", event_list[index+1].time);
    println(buffer);

    snprintf(buffer, S_WIDTH, "Rotate for: %lu seconds", (event_list[index+1].time - event_list[index].time));
    println(buffer);
    
    snprintf(buffer, S_WIDTH, "Every: %lu seconds", event_list[index].next_event);
    println(buffer);
    
}

void set_motor_event_info(unsigned int motor, unsigned long start_time, unsigned long rt_time, unsigned long rp_time) {
    size_t index;
    
    if (motor <= 0 || motor > 3 ) {
        println("Invalid motor selected");
        return;
    }
    
    /* 
     * Motor 1 => Index = (1 - 1) * 2 = 0;
     * Motor 2 => Index = (2 - 1) * 2 = 2;
     * Motor 3 => Index = (3 - 2) * 2 = 4;
     */
    index = (motor - 1) * 2;
    event_list[index].time          = start_time;
    event_list[index+1].time        = start_time + rt_time;
    event_list[index].next_event    = rp_time;
    event_list[index+1].next_event  = rp_time;
    return;
}

void println(const char *buffer) {
    Serial.print(buffer);
    Serial.println();
}

void print_datetime(unsigned long time, unsigned int pin, unsigned long rt_time) {
    if (debug) {
        char humanTime[24] = "";
        dc_ctime(humanTime, sizeof(humanTime));
        println(humanTime);
    }
    return;
}

void start_pump2(unsigned long time, unsigned int pin, unsigned long rt_time) {
    char buffer[S_WIDTH];
    sprintf(buffer, "[BLOCKING]Time: %lu Starting motor on pin %u for %u ms", time, pin, rt_time);
    println(buffer);
    
    digitalWrite(pin, HIGH);
    delay(rt_time);
    digitalWrite(pin, LOW);
    return;
}

void start_pump(unsigned long time, unsigned int pin, unsigned long rt_time) {
    char buffer[S_WIDTH];
    sprintf(buffer, "Time: %lu Starting motor on pin %u for %u ms", time, pin, rt_time);
    println(buffer);
    
    digitalWrite(pin, HIGH);
    return;
}

void stop_pump(unsigned long time, unsigned int pin, unsigned long rt_time) {
    char buffer[S_WIDTH];
    sprintf(buffer, "Time: %lu Stoping motor on pin %u for %u ms", time, pin, rt_time);
    println(buffer);

    digitalWrite(pin, LOW);
    return;
}

void handleEvents() {
    size_t i;
    unsigned long time      = dc_getUnixTime();
    unsigned long prev_time = 0;
    
    for (i = 0; event_list[i].event_cb != NULL; i++) {
        if (time >= event_list[i].time) {
            event_list[i].event_cb(event_list[i].time, event_list[i].pin, event_list[i].rt_time);
            event_list[i].time += event_list[i].next_event;
        }
    }
    
    return;
}

void processCommand(const char *recvString) {
	char    cmd[S_WIDTH]    = "";
	char    arg[S_WIDTH]    = "";
    size_t  i               = 0;

    unsigned long epoch = dc_getUnixTime();
    
    /* S_WIDTH = 80. 80 / 2 = 40. 40 - 1 (\0) = 39 */
	sscanf(recvString, "%s %39[^\n]", cmd, arg);
    
    for (i = 0; cmd_list[i] != NULL; i++) {
        if (!strcmp(cmd_list[i], cmd)) {
            break;
        }
    }

    switch(i) {
        case CMD_HELP:
        {
            println("Serial Command List:");
            for (i = 0; cmd_list[i] != NULL; i++) {
                println(cmd_list[i]);
            }
        }
            
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
            print_datetime(epoch, NOT_USED, 0);
            break;
        }

        case CMD_MOTOR1:
        {
            unsigned int    pin = P_MOTOR_1;
            unsigned int    t   = atoi(arg);
            
            start_pump2(epoch, pin, t);
            break;
        }
            
        case CMD_MOTOR2:
        {
            unsigned int    pin = P_MOTOR_2;
            unsigned int    t   = atoi(arg);
            
            start_pump2(epoch, pin, t);
            break;
        }
            
        case CMD_MOTOR3:
        {
            unsigned int    pin = P_MOTOR_3;
            unsigned int    t   = atoi(arg);

            start_pump2(epoch, pin, t);
            break;
        }
        
        case CMD_SET_MOTOR1:
        {
            unsigned long start_time    = 0;
            unsigned long rt_time       = 0;
            unsigned long rp_time       = 0;
            
            if (3 != sscanf(arg, "start\t%lu\tfor\t%lu\tevery\t%lu", &start_time, &rt_time, &rp_time)) {
                println("Error invalid input");
                break;
            }
            
            set_motor_event_info(MOTOR_1, start_time, rt_time, rp_time);
            break;
        }
        
        case CMD_SET_MOTOR2:
        {
            unsigned long start_time    = 0;
            unsigned long rt_time       = 0;
            unsigned long rp_time       = 0;
            
            if (3 != sscanf(arg, "start\t%lu\tfor\t%lu\tevery\t%lu", &start_time, &rt_time, &rp_time)) {
                println("Error invalid input");
                break;
            }
            
            set_motor_event_info(MOTOR_2, start_time, rt_time, rp_time);
            break;
        }
        case CMD_SET_MOTOR3:
        {
            unsigned long start_time    = 0;
            unsigned long rt_time       = 0;
            unsigned long rp_time       = 0;
            
            if (3 != sscanf(arg, "start\t%lu\tfor\t%lu\tevery\t%lu", &start_time, &rt_time, &rp_time)) {
                println("Error invalid input");
                break;
            }
            
            set_motor_event_info(MOTOR_3, start_time, rt_time, rp_time);
            break;
        }
            
        case CMD_GET_MOTOR1:
        {
            get_motor_event_info(MOTOR_1);
            break;
        }
        
        case CMD_GET_MOTOR2:
        {
            get_motor_event_info(MOTOR_2);
            break;
        }

        case CMD_GET_MOTOR3:
        {
            get_motor_event_info(MOTOR_3);
            break;
        }

        case CMD_SYSTEM_INFO:
        {
            char buffer[S_WIDTH];
            
            snprintf(buffer, S_WIDTH, "M_LIST size %d bytes", sizeof(M_LIST));
            println(buffer);
            
            snprintf(buffer, S_WIDTH, "cmd_list size %d bytes", sizeof(cmd_list));
            println(buffer);
            
            snprintf(buffer, S_WIDTH, "CMD_LIST size %d bytes", sizeof(CMD_LIST));
            println(buffer);
            
            snprintf(buffer, S_WIDTH, "event_list size %d bytes", sizeof(event_list));
            println(buffer);
            
        }
            
        case CMD_DEBUG: 
        {
            debug = atoi(arg);
        }
        default:
            break;
            
    }
    return;
}