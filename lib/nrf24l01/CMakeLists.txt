add_library(nrf24 
    nrf24_driver.c
    pin_manager.c
    spi_manager.c
)

target_include_directories(nrf24 PUBLIC .)
target_link_libraries(nrf24 PUBLIC 
    pico_stdlib
    hardware_spi 
    hardware_gpio
)