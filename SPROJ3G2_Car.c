/**
 * Title: Semester Project 3 Group 2 - Car code
 * Institution: University of Southern Denmark (SDU)
 * Campus: Sonderborg
 * File: SPROJ3G2_Car.c
 * Authors: Bence Toth and Iliya Iliev
 * Date: 28/10/2024
 * Course: BEng in Electronics
 * Semester: 3rd
 * Platform: RP2040
 * RF module: nRF24L01+
 * OLED module: SSD1306 128x32 I2C
 * RF library:   https://github.com/andyrids/pico-nrf24
 * OLED library: https://github.com/daschr/pico-ssd1306
 */

// Include necessary libraries
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "nrf24_driver.h"
#include "functions.h"
#include "servo_control.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
//#include "hardware/timer.h"
//#include "hardware/uart.h"
//#include "hardware/irq.h"
//#include "pico/multicore.h"

payload_t Payload = { .direction = 1 , .servo_angle = INIT_ANGLE } ;
echo_t Echo = { .echo = 0 } ;

//ssd1306_t OLED ;
nrf_client_t RF24 ;
fn_status_t success ;    // Result of packet transmission

int main ( void ) {

    stdio_init_all ( ) ;    // Initialize all present standard stdio types

    gpio_init ( PICO_DEFAULT_LED_PIN ) ;
    gpio_set_dir ( PICO_DEFAULT_LED_PIN , GPIO_OUT ) ;

    servo_init ( SERVO_PIN ) ;

    //OLED_Setup ( &OLED_Pins , &OLED ) ;

    nRF24_Setup ( &RF24 , &RF_Pins , &RF_Config , SPI_BAUDRATE , DYNPD_DISABLE ) ;
    nRF24_Comm_Dir_Setup ( &RF24 , RF24_RX , sizeof ( payload_t ) , sizeof ( echo_t ) , ( data_pipe_t ) payload_pipe , ( data_pipe_t ) echo_pipe , PAYLOAD_ADDRESS , ECHO_ADDRESS ) ;

    while ( 1 ) {
        if ( RF24.is_packet ( &payload_pipe ) ) {
            RF24.read_packet ( &Payload , sizeof ( payload_t ) ) ;

            gpio_put ( PICO_DEFAULT_LED_PIN , Payload.direction ) ;
            set_servo_angle ( Payload.servo_angle , SERVO_PIN ) ;
        }
    }
    return 0 ;
}