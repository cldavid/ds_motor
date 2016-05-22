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
#define YUN_USE_PROCESS_DATE 0

#include <stdio.h>
#include <avr/pgmspace.h>
#include <Console.h>
#include <Process.h>
#include <FileIO.h>
#include <DallasTemperature.h>
#include "LocalLibrary.h"
#include "string.h"
#include "time.h"
#include "eeprom.h"
#include "scheduler.h"
#include "motor.hpp"


// Data wire is plugged into pin 7 on the Arduino
#define ONE_WIRE_BUS    7
#define TEMP_INDEX      8
#define EPOCH_STR_LEN   12


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

static void print_datetime(unsigned long time, unsigned int pin, unsigned long rt_time);
static void print_temperature_info(unsigned long time, unsigned int pin, unsigned long rt_time);
static void yun_get_epoch(unsigned long time, unsigned int pin, unsigned long rt_time);

const char cmd_string_1[] PROGMEM = "help";
const char cmd_string_2[] PROGMEM = "getUnixTime";
const char cmd_string_3[] PROGMEM = "drive_motor";
const char cmd_string_4[] PROGMEM = "set_motor";
const char cmd_string_5[] PROGMEM = "get_motor";;
const char cmd_string_6[] PROGMEM = "disable_motor";
const char cmd_string_7[] PROGMEM = "set_temp_interval";
const char cmd_string_8[] PROGMEM = "get_temp_interval";
const char cmd_string_9[] PROGMEM = "get_temperatures";
const char cmd_string_10[] PROGMEM = "save";

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
    NULL
};

enum CMD_LIST {
    CMD_HELP         = 0,
    CMD_GET_UNIXTIME,
    CMD_MOTOR,
    CMD_SET_MOTOR,
    CMD_GET_MOTOR,
    CMD_DISABLE_MOTOR,
    CMD_SET_TEMP_INTERVAL,
    CMD_GET_TEMP_INTERVAL,
    CMD_GET_TEMPERATURES,
    CMD_SAVE,
    CMD_UNKNOWN 
};

#define T_DAY               86400
#define NOT_USED            0xffff

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
    {           0,    0,    M_PUMP_1, T_DAY, shield_start_pump          },
    {           0,    0,    M_PUMP_1, T_DAY, shield_stop_pump           },
    
    {           0,    0,    M_PUMP_2, T_DAY, shield_start_pump          },
    {           0,    0,    M_PUMP_2, T_DAY, shield_stop_pump           },
    
    {           0,    0,    M_PUMP_3, T_DAY, shield_start_pump          },
    {           0,    0,    M_PUMP_3, T_DAY, shield_stop_pump           },
    
    {           0,    0,    M_PUMP_4, T_DAY, shield_start_pump          },
    {           0,    0,    M_PUMP_4, T_DAY, shield_stop_pump           },
    
    {   BUILDTIME,    0,    NOT_USED,     60, print_temperature_info    },
    {   BUILDTIME,    0,    NOT_USED,     10, yun_get_epoch             },
    {   BUILDTIME,    0,    NOT_USED,     10, print_datetime            },
};

const uint8_t   magic_num = BUILDTIME & 0xFF;
uint8_t         magic;

/* EEPROM MAP
 * 0x0  - Event Scheduler - List
 */
#define EEPROM_START_ADDR_MAPINIT       0x0
#define EEPROM_START_ADDR_EVENTLIST     (EEPROM_START_ADDR_MAPINIT + sizeof(magic))
#define EEPROM_START_ADDR_LASTSAVE      (EEPROM_START_ADDR_EVENTLIST + sizeof(event_list))

void temperature_init(void) {
    sensors.begin();
}

void eeprom_read_config(void) {
    unsigned long epoch = BUILDTIME;

    EEPROM.read_block(&magic, EEPROM_START_ADDR_MAPINIT, sizeof(magic));
    if (magic != magic_num) {
        Console.println(F("Error invalid magic_num.\nLoading default config!"));
    } else {
        Console.println(F("Reading config from EEPROM."));
        EEPROM.read_block(event_list, (void *)EEPROM_START_ADDR_EVENTLIST, sizeof(event_list));
        EEPROM.read_block(&epoch, (void *)EEPROM_START_ADDR_LASTSAVE, sizeof(unsigned long));
    }
 
    /* Skip default events that are programmed at time 0 */ 
    Time.setUnixTime(epoch);
    Scheduler.set_event_list(event_list, sizeof(event_list));
}

void eeprom_write_event_list(unsigned long t) {
    Console.println(F("Updating EEPROM."));
    EEPROM.write_block(&magic_num, EEPROM_START_ADDR_MAPINIT, sizeof(magic_num));
    EEPROM.write_block(event_list, (void *)EEPROM_START_ADDR_EVENTLIST, sizeof(event_list));
    EEPROM.write_block(&t, (void *)EEPROM_START_ADDR_LASTSAVE, sizeof(unsigned long));
}

void get_motor_event_info(unsigned int motor) {
    size_t index;
    
    if (motor <= 0 || motor > 4 ) {
        Console.println(F("Invalid motor selected"));
        return;
    }

    /* 
     * Motor 1 => Index = (1 - 1) * 2 = 0;
     * Motor 2 => Index = (2 - 1) * 2 = 2;
     * Motor 3 => Index = (3 - 1) * 2 = 4;
     * Motor 4 => Index = (4 - 1) * 2 = 6;
     */
    index = (motor - 1) * 2;

    Console.print(F("Motor "));
    Console.println(motor);
    
    Console.print(F("Start: "));
    Console.println(event_list[index].time);
    
    Console.print(F("Stop: "));
    Console.println(event_list[index+1].time);
    
    Console.print(F("Rotate for: "));
    Console.print(event_list[index].rt_time);
    Console.println(F(" seconds"));
    
    Console.print(F("Every: "));
    Console.print(event_list[index].next_event);
    Console.println(F(" seconds"));
}

void set_motor_event_info(unsigned int motor, unsigned long start_time, unsigned long rt_time, unsigned long rp_time) {
    size_t index;
    
    //motor is unsigned no need to test if the content is smaller than 0
    if (motor > 4) {
        Console.println(F("Error: unknown motor"));
        return;
    }
    
    if (rt_time >= rp_time) {
        Console.println(F("Error: rt_time >= rp_time"));
        return;
    }
        
    /* 
     * Motor 1 => Index = (1 - 1) * 2 = 0;
     * Motor 2 => Index = (2 - 1) * 2 = 2;
     * Motor 3 => Index = (3 - 1) * 2 = 4;
     * Motor 4 => Index = (4 - 1) * 2 = 6;
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

static void print_temperature_info(unsigned long time, unsigned int pin, unsigned long rt_time) {
    uint8_t no_sensors = sensors.getDeviceCount();
    
    File dataFile = FileSystem.open("/mnt/sd/datalog.txt", FILE_APPEND);
    if (!dataFile) {
        Console.println(F("Error opening datalog.txt"));
        return;
    }
    
    // call sensors.requestTemperatures() to issue a global temperature
    // request to all devices on the bus
    sensors.requestTemperatures(); // Send the command to get temperatures
    
    for (uint8_t i = 0; i < no_sensors; i++) {
        dataFile.print(F("{\n\"timestamp:\" "));
        dataFile.print(time);
        dataFile.print(F(",\n\"sensor:\" "));
        dataFile.print(i);
        dataFile.print(F(",\n\"temperature:\" "));
        dataFile.println(sensors.getTempCByIndex(i));
        dataFile.print(F("\n},\n"));

    }
    dataFile.close();
    return;
}

static void yun_get_epoch(unsigned long time, unsigned int pin, unsigned long rt_time) {
    long    t = 0;
    Process pTime;
    
    // Get UNIX timestamp
    pTime.begin("date");
    pTime.addParameter("+%s");
    pTime.run();
    
    while (pTime.available() > 0) {
        t = pTime.readString().toInt();
    }
    Time.setUnixTime(t);
    Scheduler.update_events(t);
    return;
}

static void print_datetime(unsigned long time, unsigned int pin, unsigned long rt_time) {
    if (!rt_time) {
#ifdef __DC_TIME_HAS_HUMAN_TIME__
        Console.println(Time.getHumanTime());
#else
        Console.println(Time.getUnixTime());
#endif //__DC_TIME_HAS_HUMAN_TIME__
    }
    return;
}

void processCommand(const char *recvString) {
    char            buffer[21]    = "";
	char            cmd[21]       = "";
	char            arg[41]       = "";
    unsigned long   start_time      = 0;
    unsigned long   rt_time         = 0;
    unsigned long   rp_time         = 0;
    unsigned int    motor           = M_PUMP_E;
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
            Console.println(F("Console Command List:"));
            for (i = 0; i < CMD_UNKNOWN; i++) {
                const char *p = (const char *)pgm_read_word(&cmd_list[i]);
                strcpy_P(buffer, p);
                Console.println(buffer);
            }
            break;
            
        case CMD_GET_UNIXTIME:
            Console.println(Time.getUnixTime());
            break;

        case CMD_MOTOR:
            if (2 != sscanf(arg, "%u\t%lu", &motor, &rt_time)) {
                Console.println(F("Error invalid input"));
                break;
            }
            
            shield_drive_pump(Time.getUnixTime(), motor, rt_time);
            break;
            
            
        case CMD_SET_MOTOR:
            if (4 != sscanf(arg, "%u\tstart\t%lu\tfor\t%lu\tevery\t%lu",
                            &motor, &start_time, &rt_time, &rp_time)) {
                Console.println(F("Error invalid input"));
                break;
            }
            
            set_motor_event_info(motor, start_time, rt_time, rp_time);
            break;
            
        case CMD_GET_MOTOR:
        case CMD_DISABLE_MOTOR:
        {
            if (1 != sscanf(arg, "%u", &motor)) {
                Console.println(F("Error invalid input"));
                break;
            }
            
            switch(i) {
                case CMD_GET_MOTOR:
                    get_motor_event_info(motor);
                    break;
                case CMD_DISABLE_MOTOR:
                    set_motor_event_info(motor, 0, 0, T_DAY);
                    break;
                default:
                    break;
            }
            break;
        }
            
        case CMD_SET_TEMP_INTERVAL:
            rt_time = strtoul(arg, NULL, 10);
            event_list[TEMP_INDEX].next_event = rt_time;
            break;

        case CMD_GET_TEMP_INTERVAL:
            Console.print(F("Interval = "));
            Console.println(event_list[TEMP_INDEX].next_event);
            break;
            
        case CMD_GET_TEMPERATURES:
            print_temperature_info(Time.getUnixTime(), 0, 0);
            break;
            
        case CMD_SAVE:
            eeprom_write_event_list(Time.getUnixTime());
            break;
            
        default:
            break;
            
    }
    return;
}
