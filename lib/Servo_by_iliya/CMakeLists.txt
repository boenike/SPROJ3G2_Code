add_library(Servo_Iliya 
    servo_control.c
)

target_include_directories(Servo_Iliya PUBLIC .)
target_link_libraries(Servo_Iliya PUBLIC
        pico_stdlib 
	hardware_pwm 
	hardware_gpio   
	hardware_adc 
	hardware_clocks
)