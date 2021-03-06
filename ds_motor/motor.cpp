//
//  motor.cpp
//  ds_motor
//
//  Created by David Cluytens on 31/12/15.
//
//
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"

#include "motor.hpp"
#include <println.h>

#define NO_PUMPS    4

extern unsigned long   curTime;

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myPump[NO_PUMPS];

void shield_pump_init(void) {
    AFMS.begin();
    for (int i = 0; i < NO_PUMPS; i++) {
        myPump[i] = AFMS.getMotor(i+1);
        myPump[i]->setSpeed(255);
        myPump[i]->run(RELEASE);
    }
    
}

void shield_drive_pump(unsigned long time, unsigned int motor, unsigned long rt_time) {
    //println("Time: %lu driving motor %u for %lu ms", time, motor, rt_time);
    Serial.print(F("Time: "));
    Serial.print(time);
    Serial.print(F(" driving motor "));
    Serial.print(motor+1);
    Serial.print(F(" for "));
    Serial.print(rt_time);
    Serial.print(F(" ms\n\r"));
    
    myPump[motor]->run(FORWARD);
    delay(rt_time);
    myPump[motor]->run(RELEASE);
    return;
}

void shield_start_pump(unsigned long time, unsigned int motor, unsigned long rt_time) {
    //println("Time: %lu motor started on pin %u for %lu ms", time, motor, rt_time);
    Serial.print(F("Time: "));
    Serial.print(time);
    Serial.print(F("motor "));
    Serial.print(motor+1);
    Serial.print(F(" started for "));
    Serial.print(rt_time);
    Serial.print(F(" ms\n\r"));
    myPump[motor]->run(FORWARD);
    return;
}

void shield_stop_pump(unsigned long time, unsigned int motor, unsigned long rt_time) {
    //println("Time: %lu motor stopped on pin %u for %lu ms", time, motor, rt_time);
    Serial.print(F("Time: "));
    Serial.print(time);
    Serial.print(F("motor "));
    Serial.print(motor+1);
    Serial.print(F(" stopped for "));
    Serial.print(rt_time);
    Serial.print(F(" ms\n\r"));
    myPump[motor]->run(RELEASE);
    return;
}
