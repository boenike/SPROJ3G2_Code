add_library(functions 
    servo_control.c
)

target_include_directories(Pico Servo PRIVATE .)
target_link_libraries(Pico_servo PRIVATE
        pico_stdlib 
	hardware_pwm 
	hardware_gpio   
	hardware_adc 
	hardware_clocks
)