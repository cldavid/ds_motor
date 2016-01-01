//
//  motor.hpp
//  ds_motor
//
//  Created by David Cluytens on 31/12/15.
//
//

#ifndef motor_hpp
#define motor_hpp

#include <stdio.h>

#define PUMP_1  0
#define PUMP_2  1
#define PUMP_3  2
#define PUMP_4  3

void pump_init(void);
void shield_drive_pump(unsigned long time, unsigned int motor, unsigned long rt_time);
void shield_start_pump(unsigned long time, unsigned int motor, unsigned long rt_time);
void shield_stop_pump(unsigned long time, unsigned int motor, unsigned long rt_time);

#endif /* motor_hpp */
