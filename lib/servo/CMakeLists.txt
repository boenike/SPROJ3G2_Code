add_library(servo 
    servo.c
)

target_include_directories(servo PUBLIC .)
target_link_libraries(servo PUBLIC 
    pico_stdlib
    hardware_gpio
    hardware_pwm
)