#include <stdio.h>
#include "pico/stdlib.h"
#include "servo_control.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
//#include "hardware/adc.h"

uint8_t period_ms = 1000 / Servo_freq ;     // In milliseconds
uint16_t pwm_offset = max_pwm_val / 2 ;     // Offset for the PWM signal

void servo_init( uint8_t Servo_pin ) {
    gpio_set_function(Servo_pin, GPIO_FUNC_PWM);   // Setting GPIO for PWM output

    uint slice_num = pwm_gpio_to_slice_num(Servo_pin); // Finding the PWM slice connected to the PWM pin

    // Setting PWM frequency
    uint32_t clock_freq = clock_get_hz(clk_sys);  // Clock frequency 
    uint32_t divider16 = clock_freq / (Servo_freq * max_pwm_val);  // Calculate divisor
    uint16_t pwm_level = pwm_offset * ( 1 + (uint16_t)( default_degree/max_degree)) ;
    pwm_set_clkdiv(slice_num, divider16);  // Setting the prescaler

    // Set the initial position
    pwm_set_wrap(slice_num, max_pwm_val); 
    pwm_set_chan_level(slice_num, PWM_CHAN_A, pwm_level); 
    
    pwm_set_enabled(slice_num, true); // Enable PWM
}

void set_servo_angle(uint8_t angle , uint8_t Servo_pin ) {

    // Calculate the pulse width for the given angle
    float pulse_width = pulse_min + ((float)angle / max_degree) * (pulse_max - pulse_min);

    // Convert pulse width to duty cycle
    uint32_t duty_cycle = (uint32_t)((pulse_width / period_ms) * max_pwm_val);

    uint slice_num = pwm_gpio_to_slice_num(Servo_pin);  
    pwm_set_chan_level(slice_num, PWM_CHAN_A, duty_cycle);  // Set the duty cycle
}