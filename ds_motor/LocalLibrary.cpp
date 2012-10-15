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

#include <stdio.h>
#include "LocalLibrary.h"
#include "string.h"
#include "time.h"
#include "eeprom.h"
#include "scheduler.h"
#include "println.h"

static void print_datetime(unsigned long time, unsigned int pin, unsigned long rt_time);
static void drive_pump(unsigned long time, unsigned int pin, unsigned long rt_time);
static void start_pump(unsigned long time, unsigned int, unsigned long rt_time);
static void stop_pump(unsigned long time, unsigned int, unsigned long rt_time);

#define P_UNKNOWN   255
#define P_MOTOR_1   9
#define P_MOTOR_2   10
#define P_MOTOR_3   11

//extern unsigned long time;

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
    "disable_motor_1",
    "disable_motor_2",
    "disable_motor_3",
    "system_info",
    "debug",
    "save",
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
    CMD_DISABLE_MOTOR1,
    CMD_DISABLE_MOTOR2,
    CMD_DISABLE_MOTOR3,
    CMD_SYSTEM_INFO,
    CMD_DEBUG,
    CMD_SAVE,
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
 * For disabling a task set time to 0
 *
 */

event_t event_list[]  = { 
    {           0,    0,    P_MOTOR_1, T_DAY, start_pump     },
    {           0,    0,    P_MOTOR_1, T_DAY, stop_pump      },
    
    {           0,    0,    P_MOTOR_2, T_DAY, start_pump     },
    {           0,    0,    P_MOTOR_2, T_DAY, stop_pump      },
    
    {           0,    0,    P_MOTOR_3, T_DAY, start_pump     },
    {           0,    0,    P_MOTOR_3, T_DAY, stop_pump      },
    
    {   BUILDTIME,    0,    NOT_USED,     10, print_datetime }    
};

const uint8_t   magic_num = BUILDTIME & 0xFF;
uint8_t         magic;

/* EEPROM MAP
 * 0x0  - Event Scheduler - List
 */
#define EEPROM_START_ADDR_MAPINIT       0x0
#define EEPROM_START_ADDR_EVENTLIST     (EEPROM_START_ADDR_MAPINIT + sizeof(magic))
#define EEPROM_START_ADDR_LASTSAVE      (EEPROM_START_ADDR_EVENTLIST + sizeof(event_list))

void eeprom_read_config(void) {
    unsigned long epoch = BUILDTIME;

    EEPROM.read_block(&magic, EEPROM_START_ADDR_MAPINIT, sizeof(magic));
    if (magic != magic_num) {
        println("Incorrect magic number found %u != %u", magic, magic_num);
        println("Loading default config!");
    } else {
        println("Magic: %d ok.", magic);
        println("Reading config from EEPROM.");
        println("Magic address: %p", EEPROM_START_ADDR_MAPINIT);
        println("Event address: %p", EEPROM_START_ADDR_EVENTLIST);
        println("Epoch address: %p", EEPROM_START_ADDR_LASTSAVE);
        
        EEPROM.read_block(event_list, (void *)EEPROM_START_ADDR_EVENTLIST, sizeof(event_list));
        EEPROM.read_block(&epoch, (void *)EEPROM_START_ADDR_LASTSAVE, sizeof(unsigned long));
    }
    /* Skip default events that are programmed at time 0 */ 
    Time.setUnixTime(epoch);
    Scheduler.set_event_list(event_list, sizeof(event_list));
}

void eeprom_write_event_list(unsigned long t) {
    println("Updating EEPROM.");
    EEPROM.write_block(&magic_num, EEPROM_START_ADDR_MAPINIT, sizeof(magic_num));
    EEPROM.write_block(event_list, (void *)EEPROM_START_ADDR_EVENTLIST, sizeof(event_list));
    EEPROM.write_block(&t, (void *)EEPROM_START_ADDR_LASTSAVE, sizeof(unsigned long));
}

void get_motor_event_info(unsigned int motor) {
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

    println("Motor %u", motor);
    println("Start: %lu", event_list[index].time);
    println("Stop: %lu", event_list[index+1].time);
    println("Rotate for: %lu seconds", event_list[index].rt_time);
    println("Every: %lu seconds", event_list[index].next_event);    
}

void set_motor_event_info(unsigned int motor, unsigned long start_time, unsigned long rt_time, unsigned long rp_time) {
    size_t index;
    
    if (motor <= 0 || motor > 3 ) {
        println("Invalid motor selected");
        return;
    }
    
    if (rt_time >= rp_time) {
        println("Rotate time cannot be bigger than repeat time");
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
    event_list[index].rt_time       = rt_time;
    event_list[index + 1].rt_time   = 0;
    event_list[index].next_event    = rp_time;
    event_list[index+1].next_event  = rp_time;
    
    /* Write Settings To EEPROM */
    eeprom_write_event_list(Time.getUnixTime());
    return;
}

static void print_datetime(unsigned long time, unsigned int pin, unsigned long rt_time) {
    if (debug || !rt_time) {
        char humanTime[24] = "";
        memset(humanTime, 0, sizeof(humanTime));
        Time.ctime(humanTime, sizeof(humanTime));
        println(humanTime);
    }
    return;
}

static void drive_pump(unsigned long time, unsigned int pin, unsigned long rt_time) {
    println("[BLOCKING]Time: %lu Starting motor on pin %u for %lu ms", time, pin, rt_time);
    
    digitalWrite(pin, HIGH);
    delay(rt_time);
    digitalWrite(pin, LOW);
    return;
}

static void start_pump(unsigned long time, unsigned int pin, unsigned long rt_time) {
    println("Time: %lu Starting motor on pin %u for %lu ms", time, pin, rt_time);
    
    digitalWrite(pin, HIGH);
    return;
}

static void stop_pump(unsigned long time, unsigned int pin, unsigned long rt_time) {
    println("Time: %lu Stoping motor on pin %u for %lu ms", time, pin, rt_time);

    digitalWrite(pin, LOW);
    return;
}

void printSystemInfo(void) {
    println("Build         \ttime %lu s", BUILDTIME);
    println("M_LIST        \tsize %d bytes", sizeof(M_LIST));
    println("cmd_list      \tsize %d bytes", sizeof(cmd_list));
    println("CMD_LIST      \tsize %d bytes", sizeof(CMD_LIST));
    println("event_list    \tsize %d bytes", sizeof(event_list)); 
    
    println("start_pump    \taddr %p", start_pump);
    println("stop_pump     \taddr %p", stop_pump);
    println("print_datetime\taddr %p", print_datetime);
    println("drive_pump    \taddr %p", drive_pump);
    Scheduler.print_events();
    return;
}

void processCommand(unsigned long cur_time, const char *recvString) {
	char            cmd[S_WIDTH]    = "";
	char            arg[S_WIDTH]    = "";
    unsigned long   start_time      = 0;
    unsigned long   rt_time         = 0;
    unsigned long   rp_time         = 0;
    unsigned int    pin             = P_UNKNOWN;
    unsigned long   t               = 0;
    size_t          i               = 0;
    size_t          j               = 0;
        
    /* S_WIDTH = 80. 80 / 2 = 40. 40 - 1 (\0) = 39 */
	sscanf(recvString, "%s %39[^\n]", cmd, arg);
    
    for (i = 0; cmd_list[i] != NULL; i++) {
        if (!strcmp(cmd_list[i], cmd)) {
            break;
        }
    }
    
    switch(i) {
        case CMD_HELP:
            println("Serial Command List:");
            for (i = 0; cmd_list[i] != NULL; i++) {
                println(cmd_list[i]);
            }
            break;
            
        case CMD_SET_UNIXTIME:
            t = strtoul(arg, NULL, 10);
            Time.setUnixTime(t);
            Scheduler.update_events(t);
            break;
            
        case CMD_GET_UNIXTIME:
       		println("%lu", cur_time);     
            break;
            
        case CMD_GET_DATE:
            print_datetime(cur_time, NOT_USED, 0);
            break;

        case CMD_MOTOR1:
            pin = P_MOTOR_1;
            t   = strtoul(arg, NULL, 10);
            
            drive_pump(cur_time, pin, t);
            break;
            
        case CMD_MOTOR2:
            pin = P_MOTOR_2;
            t   = strtoul(arg, NULL, 10);
            
            drive_pump(cur_time, pin, t);
            break;
            
        case CMD_MOTOR3:
            pin = P_MOTOR_3;
            t   = strtoul(arg, NULL, 10);

            drive_pump(cur_time, pin, t);
            break;
        
        case CMD_SET_MOTOR1:
            if (3 != sscanf(arg, "start\t%lu\tfor\t%lu\tevery\t%lu", &start_time, &rt_time, &rp_time)) {
                println("Error invalid input");
                break;
            }           
            set_motor_event_info(MOTOR_1, start_time, rt_time, rp_time);
            break;
        
        case CMD_SET_MOTOR2:
            if (3 != sscanf(arg, "start\t%lu\tfor\t%lu\tevery\t%lu", &start_time, &rt_time, &rp_time)) {
                println("Error invalid input");
                break;
            }            
            set_motor_event_info(MOTOR_2, start_time, rt_time, rp_time);
            break;
            
        case CMD_SET_MOTOR3:
            if (3 != sscanf(arg, "start\t%lu\tfor\t%lu\tevery\t%lu", &start_time, &rt_time, &rp_time)) {
                println("Error invalid input");
                break;
            }
            set_motor_event_info(MOTOR_3, start_time, rt_time, rp_time);
            break;
            
        case CMD_GET_MOTOR1:
            get_motor_event_info(MOTOR_1);
            break;
        
        case CMD_GET_MOTOR2:
            get_motor_event_info(MOTOR_2);
            break;

        case CMD_GET_MOTOR3:
            get_motor_event_info(MOTOR_3);
            break;

        case CMD_DISABLE_MOTOR1:
            set_motor_event_info(MOTOR_1, 0, 0, T_DAY);
            break;
            
        case CMD_DISABLE_MOTOR2:
            set_motor_event_info(MOTOR_2, 0, 0, T_DAY);
            break;
            
        case CMD_DISABLE_MOTOR3:
            set_motor_event_info(MOTOR_3, 0, 0, T_DAY);
            break;
            
        case CMD_SYSTEM_INFO:
            printSystemInfo();
            break;
            
        case CMD_DEBUG: 
            debug = atoi(arg);
            break;
        
        case CMD_SAVE:
            eeprom_write_event_list(cur_time);
            break;
            
        default:
            break;
            
    }
    return;
}