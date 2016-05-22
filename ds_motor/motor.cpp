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

#define NO_PUMPS    4

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
    //motor is unsigned no need to test if the content is smaller than 0
    if (motor > 4) {
        Serial1.println(F("Error invalid motor number [0 .. 3]"));
        return;
    }
    Serial1.print(F("T: "));
    Serial1.print(time);
    Serial1.print(F(" driving motor "));
    Serial1.print(motor);
    Serial1.print(F(" for "));
    Serial1.print(rt_time);
    Serial1.print(F(" ms\n\r"));
    
    myPump[motor]->run(FORWARD);
    delay(rt_time);
    myPump[motor]->run(RELEASE);
    return;
}

void shield_start_pump(unsigned long time, unsigned int motor, unsigned long rt_time) {
    Serial1.print(F("T: "));
    Serial1.print(time);
    Serial1.print(F("motor "));
    Serial1.print(motor);
    Serial1.print(F(" started for "));
    Serial1.print(rt_time);
    Serial1.print(F(" ms\n\r"));
    myPump[motor]->run(FORWARD);
    return;
}

void shield_stop_pump(unsigned long time, unsigned int motor, unsigned long rt_time) {
    Serial1.print(F("T: "));
    Serial1.print(time);
    Serial1.print(F("motor "));
    Serial1.print(motor);
    Serial1.print(F(" stopped for "));
    Serial1.print(rt_time);
    Serial1.print(F(" ms\n\r"));
    myPump[motor]->run(RELEASE);
    return;
}
