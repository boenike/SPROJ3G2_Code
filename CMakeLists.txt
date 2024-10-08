# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(SPROJ3G2_Controller C CXX ASM)
project(SPROJ3G2_Car C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(SPROJ3G2_Controller SPROJ3G2_Controller.c)
add_executable(SPROJ3G2_Car SPROJ3G2_Car.c)

add_subdirectory(lib)

#pico_set_program_name(SPROJ3G2_Code "SPROJ3G2_Code")
#pico_set_program_version(SPROJ3G2_Code "0.1")

# Generate PIO header
#pico_generate_pio_header(SPROJ3G2_Code ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(SPROJ3G2_Controller 1)
pico_enable_stdio_usb(SPROJ3G2_Controller 1)

pico_enable_stdio_uart(SPROJ3G2_Car 1)
pico_enable_stdio_usb(SPROJ3G2_Car 1)

# Add the standard include files to the build
target_include_directories(SPROJ3G2_Car PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

target_include_directories(SPROJ3G2_Controller PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(SPROJ3G2_Controller 
        pico_stdlib
        hardware_spi
        hardware_i2c
        hardware_adc
        hardware_pwm
        servo
        nrf24
        functions
        hardware_timer
)

target_link_libraries(SPROJ3G2_Car 
        pico_stdlib
        hardware_spi
        hardware_i2c
        hardware_adc
        hardware_pwm
        servo
        nrf24
        functions
        hardware_timer
)

pico_add_extra_outputs(SPROJ3G2_Controller)
pico_add_extra_outputs(SPROJ3G2_Car)