#include "servo_control.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"

void joystick_init(){
    // Enabling ADC for the joystick
    stdio_init_all(); 
    adc_init(); // Setup ADC
    adc_gpio_init(control_pin); // Set the pin for ADC (26-29)
    adc_select_input(control_pin-26); // Selecting the input (0-3)
}

void servo_init() {
    gpio_set_function(Servo_pin, GPIO_FUNC_PWM);   // Setting GPIO for PWM output

    uint slice_num = pwm_gpio_to_slice_num(Servo_pin); // Finding the PWM slice connected to the PWM pin

    // Setting PWM frequency
    uint32_t clock_freq = clock_get_hz(clk_sys);  // Clock frequency 
    uint32_t divider16 = clock_freq / (Servo_freq * 65536);  // Calculate divisor
    pwm_set_clkdiv(slice_num, divider16);  // Setting the prescaler

    // Initial position of 90 deg. (1.5 ms pulse width)
    pwm_set_wrap(slice_num, 65535); 
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 49152); 
    
    pwm_set_enabled(slice_num, true); // Enable PWM
}

void set_servo_angle(float angle) {
    float pulse_min = 1.0f;   // 1 ms for 0 deg
    float pulse_max = 2.0f;   // 2 ms for 180 deg
    float period_ms = 20.0f;  // Period of 20 ms (50 Hz)

    // Calculate the pulse width for the given angle
    float pulse_width = pulse_min + (angle / 180.0f) * (pulse_max - pulse_min);

    // Convert pulse width to duty cycle
    uint32_t duty_cycle = (uint32_t)((pulse_width / period_ms) * 65535);

    uint slice_num = pwm_gpio_to_slice_num(Servo_pin);  
    pwm_set_chan_level(slice_num, PWM_CHAN_A, duty_cycle);  // Set the duty cycle
}

float degrees(int ADC_value) {
    // ADC value to servo angle conversion
    float servo_angle = (max_degree / 4095) * ADC_value - mid_degree;
    return servo_angle;
}
