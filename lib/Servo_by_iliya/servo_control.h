#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include "pico/stdlib.h"

// Constants for servo and joystick control
#define Servo_pin 14
#define Servo_freq 50
#define min_degree 0.0f 
#define max_degree 180.0f 
#define mid_degree 90.0f
#define control_pin 26

// Function declarations
void joystick_init(void);
void servo_init(void);
void set_servo_angle(float angle);
float degrees(int ADC_value);

#endif // SERVO_CONTROL_H
