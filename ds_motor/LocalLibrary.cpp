//
// LocalLibrary.cpp 
// Library C++ code
// ----------------------------------
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
#include <avr/pgmspace.h>
#include "LocalLibrary.h"
#include "string.h"
#include "time.h"
#include "eeprom.h"
#include "scheduler.h"
#include "println.h"
#include "motor.hpp"

static void print_datetime(unsigned long time, unsigned int pin, unsigned long rt_time);

enum M_LIST {
    MOTOR_1 = 1,
    MOTOR_2,
    MOTOR_3,
    MOTOR_4,
};

const char cmd_string_1[] PROGMEM = "help";
const char cmd_string_2[] PROGMEM = "setUnixTime";
const char cmd_string_3[] PROGMEM = "getUnixTime";
const char cmd_string_4[] PROGMEM = "getDate";
const char cmd_string_5[] PROGMEM = "drive_motor_1";
const char cmd_string_6[] PROGMEM = "drive_motor_2";
const char cmd_string_7[] PROGMEM = "drive_motor_3";
const char cmd_string_8[] PROGMEM = "drive_motor_4";
const char cmd_string_9[] PROGMEM = "set_motor_1";
const char cmd_string_10[] PROGMEM = "set_motor_2";
const char cmd_string_11[] PROGMEM = "set_motor_3";
const char cmd_string_12[] PROGMEM = "set_motor_4";
const char cmd_string_13[] PROGMEM = "get_motor_1";
const char cmd_string_14[] PROGMEM = "get_motor_2";
const char cmd_string_15[] PROGMEM = "get_motor_3";
const char cmd_string_16[] PROGMEM = "get_motor_4";
const char cmd_string_17[] PROGMEM = "disable_motor_1";
const char cmd_string_18[] PROGMEM = "disable_motor_2";
const char cmd_string_19[] PROGMEM = "disable_motor_3";
const char cmd_string_20[] PROGMEM = "disable_motor_4";
const char cmd_string_21[] PROGMEM = "system_info";
const char cmd_string_22[] PROGMEM = "debug";
const char cmd_string_23[] PROGMEM = "save";

PGM_P const cmd_list[] PROGMEM = {
    cmd_string_1,
    cmd_string_2,
    cmd_string_3,
    cmd_string_4,
    cmd_string_5,
    cmd_string_6,
    cmd_string_7,
    cmd_string_8,
    cmd_string_9,
    cmd_string_10,
    cmd_string_11,
    cmd_string_12,
    cmd_string_13,
    cmd_string_14,
    cmd_string_15,
    cmd_string_16,
    cmd_string_17,
    cmd_string_18,
    cmd_string_19,
    cmd_string_20,
    cmd_string_21,
    cmd_string_22,
    cmd_string_23,
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
    CMD_MOTOR4,
    CMD_SET_MOTOR1,
    CMD_SET_MOTOR2,
    CMD_SET_MOTOR3,
    CMD_SET_MOTOR4,
    CMD_GET_MOTOR1,
    CMD_GET_MOTOR2,
    CMD_GET_MOTOR3,
    CMD_GET_MOTOR4,
    CMD_DISABLE_MOTOR1,
    CMD_DISABLE_MOTOR2,
    CMD_DISABLE_MOTOR3,
    CMD_DISABLE_MOTOR4,
    CMD_SYSTEM_INFO,
    CMD_DEBUG,
    CMD_SAVE,
    CMD_UNKNOWN 
};

#define T_DAY               86400
#define NOT_USED            0xffff

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
    {           0,    0,    M_PUMP_1, T_DAY, shield_start_pump     },
    {           0,    0,    M_PUMP_1, T_DAY, shield_stop_pump      },
    
    {           0,    0,    M_PUMP_2, T_DAY, shield_start_pump     },
    {           0,    0,    M_PUMP_2, T_DAY, shield_stop_pump      },
    
    {           0,    0,    M_PUMP_3, T_DAY, shield_start_pump     },
    {           0,    0,    M_PUMP_3, T_DAY, shield_stop_pump      },
    
    {           0,    0,    M_PUMP_4, T_DAY, shield_start_pump     },
    {           0,    0,    M_PUMP_4, T_DAY, shield_stop_pump      },
    
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
        Serial.println(F("Loading default config!"));
    } else {
        println("Magic: %d ok.", magic);
        Serial.println(F("Reading config from EEPROM."));
        EEPROM.read_block(event_list, (void *)EEPROM_START_ADDR_EVENTLIST, sizeof(event_list));
        EEPROM.read_block(&epoch, (void *)EEPROM_START_ADDR_LASTSAVE, sizeof(unsigned long));
    }
 
    /* Skip default events that are programmed at time 0 */ 
    Time.setUnixTime(epoch);
    Scheduler.set_event_list(event_list, sizeof(event_list));
}

void eeprom_write_event_list(unsigned long t) {
    Serial.println(F("Updating EEPROM."));
    EEPROM.write_block(&magic_num, EEPROM_START_ADDR_MAPINIT, sizeof(magic_num));
    EEPROM.write_block(event_list, (void *)EEPROM_START_ADDR_EVENTLIST, sizeof(event_list));
    EEPROM.write_block(&t, (void *)EEPROM_START_ADDR_LASTSAVE, sizeof(unsigned long));
}

void get_motor_event_info(unsigned int motor) {
    size_t index;
    
    if (motor <= 0 || motor > 3 ) {
        Serial.println(F("Invalid motor selected"));
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
        Serial.println(F("Invalid motor selected"));
        return;
    }
    
    if (rt_time >= rp_time) {
        Serial.println(F("Rotate time cannot be bigger than repeat time"));
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
        println(Time.getHumanTime());
    }
    return;
}

void printSystemInfo(void) {
    println("Build     \t\ttime %lu s", BUILDTIME);
    println("M_LIST    \t\tsize %d bytes", sizeof(M_LIST));
    println("cmd_list  \t\tsize %d bytes", sizeof(cmd_list));
    println("CMD_LIST  \t\tsize %d bytes", sizeof(CMD_LIST));
    println("event_list\t\tsize %d bytes", sizeof(event_list));
    
    println("start_pump \t\taddr %p", shield_start_pump);
    println("stop_pump  \t\taddr %p", shield_stop_pump);
    println("print_datetime     \taddr %p", print_datetime);
    println("drive_pump  \t\taddr %p", shield_drive_pump);
    Scheduler.print_events();
    return;
}

void processCommand(const char *recvString) {
    char            buffer[21]    = "";
	char            cmd[21]       = "";
	char            arg[41]       = "";
    unsigned long   start_time      = 0;
    unsigned long   rt_time         = 0;
    unsigned long   rp_time         = 0;
    unsigned int    pin             = M_PUMP_E;
    unsigned long   t               = 0;
    size_t          i               = 0;
        
    /* S_WIDTH = 80. 80 / 2 = 40. 40 - 1 (\0) = 39 */
	sscanf(recvString, "%20s %40[^\n]", cmd, arg);
    
    for (i = 0; i < CMD_UNKNOWN; i++) {
        const char *p = (const char *)pgm_read_word(&cmd_list[i]);
        strcpy_P(buffer, p);
        if (!strcmp(buffer, cmd)) {
            break;
        }
    }
    
    switch(i) {
        case CMD_HELP:
            Serial.println(F("Serial Command List:"));
            for (i = 0; i < CMD_UNKNOWN; i++) {
                const char *p = (const char *)pgm_read_word(&cmd_list[i]);
                strcpy_P(buffer, p);
                println(buffer);
            }
            break;
            
        case CMD_SET_UNIXTIME:
            t = strtoul(arg, NULL, 10);
            Time.setUnixTime(t);
            Scheduler.update_events(t);
            break;
            
        case CMD_GET_UNIXTIME:
       		println("%lu", Time.getUnixTime());
            break;
            
        case CMD_GET_DATE:
            print_datetime(Time.getUnixTime(), NOT_USED, 0);
            break;

        case CMD_MOTOR1:
            pin = M_PUMP_1;
            t   = strtoul(arg, NULL, 10);
            
            shield_drive_pump(Time.getUnixTime(), pin, t);
            break;
            
        case CMD_MOTOR2:
            pin = M_PUMP_2;
            t   = strtoul(arg, NULL, 10);
            
            shield_drive_pump(Time.getUnixTime(), pin, t);
            break;
            
        case CMD_MOTOR3:
            pin = M_PUMP_3;
            t   = strtoul(arg, NULL, 10);

            shield_drive_pump(Time.getUnixTime(), pin, t);
            break;
        
        case CMD_MOTOR4:
            pin = M_PUMP_4;
            t   = strtoul(arg, NULL, 10);
            
            shield_drive_pump(Time.getUnixTime(), pin, t);
            break;
            
        case CMD_SET_MOTOR1:
            if (3 != sscanf(arg, "start\t%lu\tfor\t%lu\tevery\t%lu", &start_time, &rt_time, &rp_time)) {
                Serial.println(F("Error invalid input"));
                break;
            }           
            set_motor_event_info(MOTOR_1, start_time, rt_time, rp_time);
            break;
        
        case CMD_SET_MOTOR2:
            if (3 != sscanf(arg, "start\t%lu\tfor\t%lu\tevery\t%lu", &start_time, &rt_time, &rp_time)) {
                Serial.println(F("Error invalid input"));
                break;
            }            
            set_motor_event_info(MOTOR_2, start_time, rt_time, rp_time);
            break;
            
        case CMD_SET_MOTOR3:
            if (3 != sscanf(arg, "start\t%lu\tfor\t%lu\tevery\t%lu", &start_time, &rt_time, &rp_time)) {
                Serial.println(F("Error invalid input"));
                break;
            }
            set_motor_event_info(MOTOR_3, start_time, rt_time, rp_time);
            break;
        
        case CMD_SET_MOTOR4:
            if (3 != sscanf(arg, "start\t%lu\tfor\t%lu\tevery\t%lu", &start_time, &rt_time, &rp_time)) {
                Serial.println(F("Error invalid input"));
                break;
            }
            set_motor_event_info(MOTOR_4, start_time, rt_time, rp_time);
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
            
        case CMD_GET_MOTOR4:
            get_motor_event_info(MOTOR_4);
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
            
        case CMD_DISABLE_MOTOR4:
            set_motor_event_info(MOTOR_4, 0, 0, T_DAY);
            break;
            
        case CMD_SYSTEM_INFO:
            printSystemInfo();
            break;
            
        case CMD_DEBUG: 
            debug = atoi(arg);
            break;
        
        case CMD_SAVE:
            eeprom_write_event_list(Time.getUnixTime());
            break;
            
        default:
            break;
            
    }
    return;
}
