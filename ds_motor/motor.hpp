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

#define M_PUMP_1  0
#define M_PUMP_2  1
#define M_PUMP_3  2
#define M_PUMP_4  3
#define M_PUMP_E  255

void shield_pump_init(void);
void shield_drive_pump(unsigned long time, unsigned int motor, unsigned long rt_time);
void shield_start_pump(unsigned long time, unsigned int motor, unsigned long rt_time);
void shield_stop_pump(unsigned long time, unsigned int motor, unsigned long rt_time);

#endif /* motor_hpp */
