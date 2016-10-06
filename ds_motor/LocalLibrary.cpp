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
#include <DallasTemperature.h>
#include "LocalLibrary.h"
#include "string.h"

#define __DC_TIME_HAS_HUMAN_TIME__ 1

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

static void print_temperature_info(unsigned long time, unsigned int pin, unsigned long rt_time);

const char cmd_string_1[] PROGMEM = "help";
const char cmd_string_2[] PROGMEM = "setUnixTime";
const char cmd_string_3[] PROGMEM = "getUnixTime";
const char cmd_string_4[] PROGMEM = "drive_motor";
const char cmd_string_5[] PROGMEM = "set_motor";
const char cmd_string_6[] PROGMEM = "get_motor";;
const char cmd_string_7[] PROGMEM = "disable_motor";
const char cmd_string_8[] PROGMEM = "set_temp_interval";
const char cmd_string_9[] PROGMEM = "get_temp_interval";
const char cmd_string_10[] PROGMEM = "get_temperatures";
const char cmd_string_11[] PROGMEM = "save";

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
	NULL
};

enum CMD_LIST {
	CMD_HELP         = 0,
	CMD_SET_UNIXTIME,
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
 * Settings for Motor 0 are located at index 0 and 1
 * Settings for Motor 1 are located at index 2 and 3
 * Settings for Motor 2 are located at index 4 and 5
 * Settings for Motor 3 are located at index 6 and 7 
 * Settings for Temperature interval is located at index 8
 *
 * Index [8-... Free to use
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
		Serial1.println(F("Error invalid magic_num.\nLoading default config!"));
	} else {
		Serial1.println(F("Reading config from EEPROM."));
		EEPROM.read_block(event_list, (void *)EEPROM_START_ADDR_EVENTLIST, sizeof(event_list));
		EEPROM.read_block(&epoch, (void *)EEPROM_START_ADDR_LASTSAVE, sizeof(unsigned long));
	}

	/* Skip default events that are programmed at time 0 */
	Time.setUnixTime(epoch);
	Scheduler.set_event_list(event_list, sizeof(event_list));
}

void eeprom_write_event_list(unsigned long t) {
	Serial1.println(F("Updating EEPROM."));
	EEPROM.write_block(&magic_num, EEPROM_START_ADDR_MAPINIT, sizeof(magic_num));
	EEPROM.write_block(event_list, (void *)EEPROM_START_ADDR_EVENTLIST, sizeof(event_list));
	EEPROM.write_block(&t, (void *)EEPROM_START_ADDR_LASTSAVE, sizeof(unsigned long));
}

void get_motor_event_info(unsigned int motor) {
	size_t index;

	if (motor > 3) {
		Serial1.println(F("Invalid motor selected [0 .. 3]"));
		return;
	}

	/*
	 * Motor 1 => Index = (1 - 1) * 2 = 0;
	 * Motor 2 => Index = (2 - 1) * 2 = 2;
	 * Motor 3 => Index = (3 - 1) * 2 = 4;
	 * Motor 4 => Index = (4 - 1) * 2 = 6;
	 */
	index = motor * 2;

	Serial1.print(F("Motor "));
	Serial1.println(motor);

	Serial1.print(F("Start: "));
	Serial1.println(event_list[index].time);

	Serial1.print(F("Stop: "));
	Serial1.println(event_list[index+1].time);

	Serial1.print(F("Rotate for: "));
	Serial1.print(event_list[index].rt_time);
	Serial1.println(F(" seconds"));

	Serial1.print(F("Every: "));
	Serial1.print(event_list[index].next_event);
	Serial1.println(F(" seconds"));
}

void set_motor_event_info(unsigned int motor, unsigned long start_time, unsigned long rt_time, unsigned long rp_time) {
	size_t index;

	//motor is unsigned no need to test if the content is smaller than 0
	if (motor > 3) {
		Serial1.println(F("Error: unknown motor [0 .. 3]"));
		return;
	}

	if (rt_time >= rp_time) {
		Serial1.println(F("Error: rt_time >= rp_time"));
		return;
	}

	/*
	 * Motor 1 => Index = (1 - 1) * 2 = 0;
	 * Motor 2 => Index = (2 - 1) * 2 = 2;
	 * Motor 3 => Index = (3 - 1) * 2 = 4;
	 * Motor 4 => Index = (4 - 1) * 2 = 6;
	 */
	index = motor * 2;
	event_list[index].time          = start_time;
	event_list[index+1].time        = start_time + rt_time;
	event_list[index].rt_time       = rt_time;
	event_list[index+1].rt_time     = 0;
	event_list[index].next_event    = rp_time;
	event_list[index+1].next_event  = rp_time;

	/* Write Settings To EEPROM */
	eeprom_write_event_list(Time.getUnixTime());
	return;
}

static void print_temperature_info(unsigned long time, unsigned int pin, unsigned long rt_time) {
	DeviceAddress deviceAddress;
	uint8_t no_sensors = sensors.getDeviceCount();

	// call sensors.requestTemperatures() to issue a global temperature
	// request to all devices on the bus
	sensors.requestTemperatures(); // Send the command to get temperatures
	Serial1.print(F("Epoch-Time: "));
	Serial1.print(time);

	if (no_sensors) {
		Serial1.print(F(" Sensor: "));
	}

	for (uint8_t i = 0; i < no_sensors; i++) {
		sensors.getAddress(deviceAddress, i);
		for (uint8_t j = 0; j < 8; j++) {
			Serial1.print(deviceAddress[j], HEX);
		}

		Serial1.print(F(","));
		Serial1.print(sensors.getTempCByIndex(i));

		if (i < (no_sensors - 1)) {
			Serial1.print(F(","));
		}
	}
	Serial1.println("");
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
			Serial1.println(F("Command List:"));
			for (i = 0; i < CMD_UNKNOWN; i++) {
				const char *p = (const char *)pgm_read_word(&cmd_list[i]);
				strcpy_P(buffer, p);
				Serial1.println(buffer);
			}
			break;

		case CMD_SET_UNIXTIME:
			rt_time = strtoul(arg, NULL, 10);
			Time.setUnixTime(rt_time);
			Scheduler.update_events(rt_time);
			break;

		case CMD_GET_UNIXTIME:
			Serial1.println(Time.getUnixTime());
			break;

		case CMD_MOTOR:
			if (2 != sscanf(arg, "%u\t%lu", &motor, &rt_time)) {
				Serial1.println(F("Error invalid input"));
				break;
			}

			shield_drive_pump(Time.getUnixTime(), motor, rt_time);
			break;


		case CMD_SET_MOTOR:
			if (4 != sscanf(arg, "%u\tstart\t%lu\tfor\t%lu\tevery\t%lu",
						&motor, &start_time, &rt_time, &rp_time)) {
				Serial1.println(F("Error invalid input"));
				break;
			}

			set_motor_event_info(motor, start_time, rt_time, rp_time);
			break;

		case CMD_GET_MOTOR:
		case CMD_DISABLE_MOTOR:
			{
				if (1 != sscanf(arg, "%u", &motor)) {
					Serial1.println(F("Error invalid input"));
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
			Serial1.print(F("Interval = "));
			Serial1.println(event_list[TEMP_INDEX].next_event);
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
