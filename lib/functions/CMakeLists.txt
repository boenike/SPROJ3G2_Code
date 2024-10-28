add_library(functions 
    functions.c
)

target_include_directories(functions PUBLIC .)
target_link_libraries(functions PUBLIC 
    pico_stdlib
    hardware_adc
    hardware_pwm
    hardware_spi
    hardware_i2c 
    hardware_gpio
    nrf24
    ssd1306
)