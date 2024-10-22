#ifdef __cplusplus
 extern "C" {
#endif

#include "pico/stdlib.h"

// Constants for servo and joystick control
#define Servo_freq 50    // in Hz
#define pulse_min 0.5f   // 0.5 ms for 0 deg
#define pulse_max 2.5f   // 2.5 ms for 180 deg
#define min_degree 0.0f
#define max_degree 180.0f
#define default_degree 90.0f    // default value
#define max_pwm_val 65535
#define max_adc_val 4095

// Function declarations
void servo_init ( uint8_t Servo_pin ) ;
void set_servo_angle ( uint8_t angle , uint8_t Servo_pin ) ;

#ifdef __cplusplus
}
#endif